/**
 * @copyright 2023-present Brian Cairl
 *
 * @file static.hpp
 */
#pragma once

// C++ Standard Library
#include <algorithm>
#include <array>
#include <thread>

namespace para
{

/**
 * @brief Managers N threads of execution, decided at compile-time, each of which run an identical work-loop
 *
 * Joins threads on destruction
 *
 * @tparam N  thread count
 */
template <std::size_t N> class work_group_static
{
public:
  static_assert(N > 0, "work_group_static<N> must have (N > 0) threads");

  /**
   * @brief Starts all workers running work callback \c f
   */
  template <typename WorkLoopFnT> explicit work_group_static(WorkLoopFnT f)
  {
    work_group_static::each([f](auto& t) { t = std::thread{f}; });
  }

  /**
   * @brief Waits for all work threads to join
   */
  ~work_group_static()
  {
    work_group_static::each([](auto& t) { t.join(); });
  }

private:
  /// Executes a callback on each worker thread
  template <typename UnaryFnT> void each(UnaryFnT&& fn)
  {
    std::for_each(workers_.begin(), workers_.end(), std::forward<UnaryFnT>(fn));
  }

  /// Worker threads
  std::array<std::thread, N> workers_;
};

/**
 * @brief Managers 1 thread of execution which runs a work-loop
 *
 * Joins threads on destruction
 */
template <> class work_group_static<1>
{
public:
  /**
   * @brief Starts worker running work callback \c f
   */
  template <typename WorkLoopFnT> explicit work_group_static(WorkLoopFnT&& f) : worker_{std::forward<WorkLoopFnT>(f)} {}

  /**
   * @brief Waits for work thread to join
   */
  ~work_group_static() { worker_.join(); }

private:
  /// Worker thread
  std::thread worker_;
};

}  // namespace para