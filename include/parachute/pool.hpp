/**
 * @copyright 2023-present Brian Cairl
 *
 * @file pool.hpp
 */
#pragma once

// Parachute
#include <parachute/pool_base.hpp>
#include <parachute/work_group/dynamic.hpp>
#include <parachute/work_group/static.hpp>
#include <parachute/work_queue/fifo.hpp>
#include <parachute/work_queue/lifo.hpp>

namespace para
{

/**
 * @brief Default work pool execution options
 */
struct work_control_default
{
public:
  template <typename WorkQueueT> constexpr bool check([[maybe_unused]] WorkQueueT&& ignored) { return working_; }
  constexpr void stop() { working_ = false; }

private:
  bool working_ = true;
};

/**
 * @brief Default work pool execution options
 */
struct work_control_strict
{
public:
  template <typename WorkQueueT> constexpr bool check(WorkQueueT&& queue) { return working_ or !queue.empty(); }
  constexpr void stop() { working_ = false; }

private:
  bool working_ = true;
};

/**
 * @brief A single-threaded work
 */
using worker = pool_base<work_group_static<1>, work_queue_lifo<>, work_control_default>;

/**
 * @copydoc worker
 * @note always finishes all work
 */
using worker_strict = pool_base<work_group_static<1>, work_queue_lifo<>, work_control_strict>;

/**
 * @brief A multi-threaded worker; thread count decided at compile-time
 */
template <std::size_t N> using static_pool = pool_base<work_group_static<N>, work_queue_lifo<>, work_control_default>;

/**
 * @copydoc static_pool
 * @note always finishes all work
 */
template <std::size_t N>
using static_pool_strict = pool_base<work_group_static<N>, work_queue_lifo<>, work_control_strict>;

/**
 * @brief A multi-threaded worker; thread count decided at runtime
 */
using pool = pool_base<work_group_dynamic, work_queue_lifo<>, work_control_default>;

/**
 * @copydoc pool
 * @note always finishes all work
 */
using pool_strict = pool_base<work_group_dynamic, work_queue_lifo<>, work_control_strict>;

#ifdef PARACHUTE_COMPILED
extern template class pool_base<work_group_static<1>, work_queue_lifo<>, work_control_default>;
extern template class pool_base<work_group_static<1>, work_queue_lifo<>, work_control_strict>;
extern template class pool_base<work_group_dynamic, work_queue_lifo<>, work_control_default>;
extern template class pool_base<work_group_dynamic, work_queue_lifo<>, work_control_strict>;
#endif  // PARACHUTE_COMPILED

}  // namespace para