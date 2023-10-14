/**
 * @copyright 2023-present Brian Cairl
 *
 * @file pool_base.hpp
 */
#pragma once

// C++ Standard Library
#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>

namespace para
{

/**
 * @brief Represents a pool of 1 of workers (typically threads) which participate in executing enqueued work
 */
template <typename WorkGroupT, typename WorkQueueT, typename WorkControlT> class pool_base
{
public:
  /**
   * @brief Initializes workers
   *
   * @param work_control  controls how thread pool behavior around work completion
   * @param work_group_args...  arguments, other than work loop callback, used to setup work group, i.e.
   * <code>WorkGroupT{loop_fn, work_group_args...}</code>
   */
  template <typename... WorkGroupArgTs>
  explicit pool_base(WorkControlT&& work_control, WorkGroupArgTs&&... work_group_args) :
      worker_control_{std::move(work_control)},
      workers_{
        [this]() {
          std::unique_lock lock{work_queue_mutex_};
          // Keep doing work until stopped
          while (worker_control_.check(work_queue_))
          {
            if (work_queue_.empty())
            {
              // If no work is available, wait for emplace
              work_queue_cv_.wait(lock);
            }
            else
            {
              // Get next work to do
              auto next_to_run = work_queue_.pop();

              // Unlock queue lock
              lock.unlock();

              // Do the work
              next_to_run();

              // Lock queue lock
              lock.lock();
            }
          }
        },
        std::forward<WorkGroupArgTs>(work_group_args)...}
  {}

  /**
   * @brief Initializes workers
   *
   * @param work_group_args...  arguments, other than work loop callback, used to setup work group, i.e.
   * <code>WorkGroupT{loop_fn, work_group_args...}</code>
   */
  template <typename... WorkGroupArgTs>
  explicit pool_base(WorkGroupArgTs&&... work_group_args) :
    pool_base{WorkControlT{}, std::forward<WorkGroupArgTs>(work_group_args)...}
  {}

  /**
   * @brief Enqueues new work
   */
  template <typename WorkT> void emplace(WorkT&& work)
  {
    // Adds work under lock
    {
      std::lock_guard lock{work_queue_mutex_};
      work_queue_.enqueue(std::forward<WorkT>(work));
    }
    // Signal that work is available
    work_queue_cv_.notify_one();
  }

  ~pool_base()
  {
    // Stop work loop under look
    {
      std::lock_guard lock{work_queue_mutex_};
      worker_control_.stop();
    }
    // Unblock any active waits
    work_queue_cv_.notify_all();
  }

private:
  /// Protects concurrent access of work_queue_cv_
  std::mutex work_queue_mutex_;

  /// Condition variable to signal new work
  std::condition_variable work_queue_cv_;

  /// Constrains work queue behavior
  WorkControlT worker_control_;

  /// Queue of jobs to run
  WorkQueueT work_queue_;

  /// Workers which run jobs
  WorkGroupT workers_;
};

}  // namespace para