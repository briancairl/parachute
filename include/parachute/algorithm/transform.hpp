/**
 * @copyright 2023-present Brian Cairl
 *
 * @file transform.hpp
 */
#pragma once

// C++ Standard Library
#include <algorithm>
#include <mutex>
#include <thread>

// Parachute
#include <parachute/internal/countdown_synchronizer.hpp>
#include <parachute/pool_base.hpp>

namespace para::algorithm
{

/**
 * @brief Parallel version of std::transform which invokes a unary callback on each element of a sequence [first, last)
 *
 * @param pool  thread pool
 * @param first  iterator to first element in sequence
 * @param last  iterator to one past last element in sequence
 * @param out  output iterator; dereferenced value is assigned to return value of <code>f(*first)</code>
 * @param f  callback to run on each element of sequence which returns an output value to assign to <code>out</code>
 *
 * @return f
 *
 * @warning order of output values is not gauranteed to match input sequence
 */
template <
  typename WorkGroupT,
  typename WorkQueueT,
  typename WorkControlT,
  typename InputIt,
  typename OutputIt,
  typename UnaryFunction>
OutputIt transform(
  pool_base<WorkGroupT, WorkQueueT, WorkControlT>& pool,
  InputIt first,
  InputIt last,
  OutputIt out,
  UnaryFunction f)
{
  internal::countdown_synchronizer countdown{ static_cast<std::size_t>(std::distance(first, last)) };
  std::for_each(first, last, [&pool, &countdown, &out, &f](auto&& value) mutable {
    pool.emplace([&countdown, &out, &f, &value]() mutable {
      auto t_value = f(value);
      countdown.decrement([&out, &f, t_value = std::move(t_value)]() mutable { (*out++) = std::move(t_value); });
    });
  });
  countdown.wait();
  return out;
}

}  // namespace para::algorithm