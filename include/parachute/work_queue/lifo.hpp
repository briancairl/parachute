/**
 * @copyright 2023-present Brian Cairl
 *
 * @file lifo.hpp
 */
#pragma once

// C++ Standard Library
#include <deque>
#include <functional>
#include <utility>

namespace para
{

/**
 * @brief Represents a default, last-in-first-out (LIFO) work queue
 */
template <typename WorkStorageT = std::function<void()>, typename WorkStorageAllocatorT = std::allocator<WorkStorageT>>
class work_queue_lifo
{
public:
  /**
   * @brief Returns next job to run
   * @warning behavior is undefined if <code>empty() == true</code>
   */
  [[nodiscard]] WorkStorageT pop()
  {
    WorkStorageT next_job{ std::move(c_.front()) };
    c_.pop_front();
    return next_job;
  }

  /**
   * @brief Adds new \c work to the queue
   */
  template <typename WorkT> void enqueue(WorkT&& work) { c_.emplace_back(std::forward<WorkT>(work)); }

  /**
   * @brief Returns true if queue contains no work
   */
  constexpr bool empty() const { return c_.empty(); }

private:
  /// Underlying queue storage
  std::deque<WorkStorageT, WorkStorageAllocatorT> c_;
};

}  // namespace para