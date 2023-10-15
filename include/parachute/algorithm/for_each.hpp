/**
 * @copyright 2023-present Brian Cairl
 *
 * @file for_each.hpp
 */
#pragma once

// C++ Standard Library
#include <algorithm>

// Parachute
#include <parachute/internal/countdown_synchronizer.hpp>
#include <parachute/pool_base.hpp>

namespace para::algorithm
{

/**
 * @brief Parallel version of std::for_each which invokes a unary callback on each element of a sequence [first, last)
 *
 * @param pool  thread pool
 * @param first  iterator to first element in sequence
 * @param last  iterator to one past last element in sequence
 * @param f  callback to run on each element of sequence
 *
 * @return f
 */
template <typename WorkGroupT, typename WorkQueueT, typename WorkControlT, typename InputIt, typename UnaryFunction>
UnaryFunction
for_each(pool_base<WorkGroupT, WorkQueueT, WorkControlT>& pool, InputIt first, InputIt last, UnaryFunction f)
{
  internal::countdown_synchronizer countdown{ static_cast<std::size_t>(std::distance(first, last)) };
  std::for_each(first, last, [&pool, &countdown, &f](auto&& value) mutable {
    pool.emplace([&countdown, &f, &value]() {
      f(value);
      --countdown;
    });
  });
  countdown.wait();
  return f;
}

}  // namespace para::algorithm