/**
 * @copyright 2023-present Brian Cairl
 *
 * @file fifo.hpp
 */
#pragma once

// C++ Standard Library
#include <functional>
#include <utility>
#include <vector>

namespace para
{

/**
 * @brief Represents a default, first-in-first-out (FIFO) work queue
 */
template <typename WorkStorageT = std::function<void()>, typename WorkStorageAllocatorT = std::allocator<WorkStorageT>>
class work_queue_fifo
{
public:
  /**
   * @brief Returns next job to run
   * @warning behavior is undefined if <code>empty() == true</code>
   */
  [[nodiscard]] WorkStorageT pop()
  {
    WorkStorageT next_job{std::move(c_.back())};
    c_.pop_back();
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
  std::vector<WorkStorageT, WorkStorageAllocatorT> c_;
};

}  // namespace para
