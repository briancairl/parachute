/**
 * @copyright 2023-present Brian Cairl
 *
 * @file non_blocking_future.hpp
 */
#pragma once

// C++ Standard Library
#include <atomic>
#include <cstddef>
#include <exception>
#include <mutex>
#include <type_traits>

// Parachute
#include <parachute/utility/uninitialized.hpp>

namespace para
{
template <typename T> class non_blocking_promise;
template <typename T> class non_blocking_future;

/**
 * @brief Error codes used when throwing <code>non_blocking_future_error</code>
 */
enum class non_blocking_future_errc
{
  no_state,  ///< state was already retrieved from future; or future already retrieved from promise
  promise_already_satisfied  ///< value already set to promise
};

/**
 * @brief Exception type thrown with <code>non_blocking_future</code> and <code>non_blocking_promise</code>
 */
struct non_blocking_future_error
{
  non_blocking_future_errc error;
};

namespace detail
{

/**
 * @brief Holds result state shared between threads
 *
 * @tparam T  held value type
 */
template <typename T> class non_blocking_shared_state;

/**
 * @copydoc non_blocking_shared_state
 * @note void specialization; meant for flagging only
 */
template <> class non_blocking_shared_state<void>
{
public:
  /**
   * @brief Returns true if result is ready such that <code>non_blocking_promise::get()</code> is valid
   */
  bool valid() const { return result_ready_flag_; }

  /**
   * @brief Sets current exception
   */
  void set(std::exception_ptr&& ex) { current_exception_ = std::move(ex); }

  /**
   * @brief Sets ready state
   */
  void set()
  {
    if (result_ready_flag_)
    {
      throw non_blocking_future_error{ non_blocking_future_errc::promise_already_satisfied };
    }
    result_ready_flag_ = true;
  }

  /**
   * @brief Clears ready state
   * @throws if there is an active exception
   */
  void get()
  {
    if (current_exception_ != nullptr)
    {
      std::rethrow_exception(current_exception_);
    }

    if (!result_ready_flag_)
    {
      throw non_blocking_future_error{ non_blocking_future_errc::no_state };
    }
    result_ready_flag_ = false;
  }

private:
  /// Indicates if result is ready or not
  std::atomic<bool> result_ready_flag_ = false;
  /// Current exception
  std::exception_ptr current_exception_ = nullptr;
};

/**
 * @copydoc non_blocking_shared_state
 */
template <typename T> class non_blocking_shared_state
{
public:
  /**
   * @brief Returns true if result is ready such that <code>non_blocking_promise::get()</code> is valid
   */
  bool valid() { return result_signal_.valid(); }

  /**
   * @brief Sets current exception
   */
  void set(std::exception_ptr&& ex) { result_signal_.set(std::move(ex)); }

  /**
   * @brief Sets value
   */
  void set(T&& result) noexcept(false)
  {
    std::lock_guard lock{ result_mutex_ };
    result_value_.emplace(std::move(result));
    result_signal_.set();
  }

  /**
   * @brief Returns held value if valid, or null
   */
  T get()
  {
    std::lock_guard lock{ result_mutex_ };
    result_signal_.get();
    return result_value_.get();
  }

private:
  /// Indicates if result is ready or not
  non_blocking_shared_state<void> result_signal_;
  /// Protects shared result state between threads of execution
  std::mutex result_mutex_;
  /// Held result value
  utility::uninitialized<T> result_value_;
};

/**
 * @brief Common base of non_blocking_promise types
 *
 * @tparam T  held value type
 */
template <typename T> class non_blocking_promise_common
{
public:
  /**
   * @brief Creates a non_blocking_promise_common with unfulfilled result value
   */
  non_blocking_promise_common() : state_{ new detail::non_blocking_shared_state<T>{} }, owning_{ true } {}

  /**
   * @brief Returns handle to shared work state
   */
  non_blocking_future<T> get_future() noexcept(false);

  /**
   * @brief Sets active exception
   */
  void set_exception(std::exception_ptr ex) { state_->set(std::move(ex)); }

  non_blocking_promise_common(const non_blocking_promise_common&) = delete;

  non_blocking_promise_common(non_blocking_promise_common&& other) : state_{ other.state_ }, owning_{ other.owning_ }
  {
    other.state_ = nullptr;
    other.owning_ = false;
  }

  ~non_blocking_promise_common()
  {
    if (owning_)
    {
      delete state_;
    }
  }

protected:
  /// Shared result state
  detail::non_blocking_shared_state<T>* state_ = nullptr;
  /// Shared state is owned by promise
  bool owning_ = false;
};

}  // namespace detail

/**
 * @brief Represents a future value to be computed, generally in an asynchronous manner
 *
 * @tparam T  held value type
 */
template <typename T> class non_blocking_future
{
  friend class detail::non_blocking_promise_common<T>;

public:
  /**
   * @brief Returns true if value held by future is valid
   */
  bool valid() { return state_->valid(); }

  /**
   * @brief Returns held value if valid, or error
   *
   * @see non_blocking_future_error
   */
  decltype(auto) get() { return state_->get(); }

  non_blocking_future(const non_blocking_future&) = delete;

  non_blocking_future(non_blocking_future&& other) : state_{ other.state_ } { other.state_ = nullptr; }

  ~non_blocking_future()
  {
    if (state_ != nullptr)
    {
      delete state_;
    }
  }

private:
  /**
   * @brief Constructs non_blocking_future from non_blocking_promise
   *
   * @note only accessible by non_blocking_promise<T>
   */
  explicit non_blocking_future(detail::non_blocking_shared_state<T>* shared_state) : state_{ shared_state } {};

  /// Shared result state
  detail::non_blocking_shared_state<T>* state_;
};

/**
 * @brief Represents the state of an eventual value, shared between execution contexts
 *
 * @tparam T  held value type
 */
template <typename T> struct non_blocking_promise : public detail::non_blocking_promise_common<T>
{
  /**
   * @brief Sets value payload shared with future
   */
  void set_value(T&& result) { this->state_->set(std::move(result)); }
};

/**
 * @copydock non_blocking_promise
 * @note
 */
template <> struct non_blocking_promise<void> : public detail::non_blocking_promise_common<void>
{
  /**
   * @brief Sets ready state
   */
  void set_value() { this->state_->set(); }
};

template <typename T> non_blocking_future<T> detail::non_blocking_promise_common<T>::get_future() noexcept(false)
{
  if (!owning_)
  {
    throw non_blocking_future_error{ non_blocking_future_errc::no_state };
  }
  auto* const transferred_state = state_;
  owning_ = false;
  return non_blocking_future<T>{ transferred_state };
}

}  // namespace para