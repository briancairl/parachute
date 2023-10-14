/**
 * @copyright 2023-present Brian Cairl
 *
 * @file dynamic.hpp
 */
#pragma once

// C++ Standard Library
#include <algorithm>
#include <memory>
#include <thread>

namespace para
{

/**
 * @brief Managers N threads of execution, decided at runtime, each of which run an identical work-loop
 *
 * Joins threads on destruction
 */
class work_group_dynamic
{
public:
  /**
   * @brief Starts all workers running work callback \c f
   */
  template <typename WorkLoopFnT>
  explicit work_group_dynamic(WorkLoopFnT f, const std::size_t n_workers = std::thread::hardware_concurrency()) :
      workers_{std::make_unique<std::thread[]>(n_workers)}, n_workers_{n_workers}
  {
    work_group_dynamic::each([f](auto& t) { t = std::thread{f}; });
  }

  /**
   * @brief Waits for all work threads to join
   */
  ~work_group_dynamic()
  {
    work_group_dynamic::each([](auto& t) { t.join(); });
  }

private:
  /// Executes a callback on each worker thread
  template <typename UnaryFnT> void each(UnaryFnT&& fn)
  {
    std::for_each(workers_.get(), workers_.get() + n_workers_, std::forward<UnaryFnT>(fn));
  }

  /// Worker threads
  std::unique_ptr<std::thread[]> workers_;
  /// Worker thread count
  std::size_t n_workers_;
};

}  // namespace para