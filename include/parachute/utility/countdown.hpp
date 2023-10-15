/**
 * @copyright 2023-present Brian Cairl
 *
 * @file countdown.hpp
 */
#pragma once

// C++ Standard Library
#include <condition_variable>
#include <mutex>
#include <thread>

namespace para::utility
{

/**
 * @brief Waits until a count reaches zero
 */
class countdown
{
public:
  explicit countdown(std::size_t n) : count_{ n } {}

  countdown& operator--()
  {
    return decrement([] {});
  }

  template <typename FnT> countdown& decrement(FnT f)
  {
    std::lock_guard lock{ count_mutex_ };
    f();
    --count_;
    count_cv_.notify_one();
    return *this;
  }

  void wait()
  {
    std::unique_lock lock{ count_mutex_ };
    if (count_)
    {
      count_cv_.wait(lock, [this] { return count_ == 0; });
    }
  }

  constexpr bool valid() const { return count_ > 0; }

private:
  std::size_t count_;
  std::mutex count_mutex_;
  std::condition_variable count_cv_;
};

}  // namespace para::utility