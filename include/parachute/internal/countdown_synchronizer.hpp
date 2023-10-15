/**
 * @copyright 2023-present Brian Cairl
 *
 * @file countdown_synchronizer.hpp
 */
#pragma once

// C++ Standard Library
#include <condition_variable>
#include <mutex>
#include <thread>

namespace para::internal
{

/**
 * @brief Waits until a count reaches zero
 */
class countdown_synchronizer
{
public:
  explicit countdown_synchronizer(std::size_t n) : count_{ n } {}

  countdown_synchronizer& operator--()
  {
    return decrement([] {});
  }

  template <typename FnT> countdown_synchronizer& decrement(FnT f)
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

private:
  std::size_t count_;
  std::mutex count_mutex_;
  std::condition_variable count_cv_;
};

}  // namespace para::internal