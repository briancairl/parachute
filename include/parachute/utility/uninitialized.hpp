/**
 * @copyright 2023-present Brian Cairl
 *
 * @file countdown.hpp
 */
#pragma once

// C++ Standard Library
#include <cstddef>
#include <utility>

namespace para::utility
{

/**
 * @brief uninitialized block of data which can fit a value of type T
 */
template <typename T> class uninitialized
{
public:
  /**
   * @brief Construct value
   */
  template <typename... CTorArgTs> void emplace(CTorArgTs&&... args)
  {
    new (data()) T{ std::forward<CTorArgTs>(args)... };
  }

  /**
   * @brief Returns held value if valid, or null
   * @warning only valid if set() has been called
   */
  T get()
  {
    T rvo_data{ std::move(*data()) };
    data()->~T();
    return rvo_data;
  }

  /**
   * @brief Returns pointer to underlying value
   * @warning only valid if set() has been called
   */
  constexpr T* operator->() { return data(); }

  /**
   * @brief Returns pointer to underlying value
   * @warning only valid if set() has been called
   */
  constexpr const T* operator->() const { return data(); }

private:
  /// Returns pointer to held value
  constexpr T* data() { return reinterpret_cast<T*>(data_); }
  constexpr const T* data() const { return reinterpret_cast<const T*>(data_); }
  /// Held value
  alignas(T) std::byte data_[sizeof(T)];
};

}  // namespace para::utility