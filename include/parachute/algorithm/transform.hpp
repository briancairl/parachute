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
#include <parachute/pool_base.hpp>
#include <parachute/utility/countdown.hpp>

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
  const InputIt last,
  OutputIt out,
  UnaryFunction f)
{
  utility::countdown barrier{ static_cast<std::size_t>(std::distance(first, last)) };
  std::for_each(first, last, [&pool, &barrier, &out, &f](auto&& value) mutable {
    pool.emplace([&barrier, &out, &f, &value]() mutable {
      auto t_value = f(value);
      barrier.decrement([&out, &f, t_value = std::move(t_value)]() mutable { (*out++) = std::move(t_value); });
    });
  });
  barrier.wait();
  return out;
}

/**
 * @brief Parallel version of std::transform which invokes a unary callback on each element of a sequence [first, last)
 *
 * @param pool  thread pool
 * @param in_first  iterator to first element in sequence
 * @param in_last  iterator to one past last element in sequence
 * @param out  output iterator; dereferenced value is assigned to return value of <code>f(*first)</code>
 * @param f  callback to run on each element of sequence which returns an output value to assign to <code>out</code>
 *
 * @return f
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
  InputIt in_first,
  const InputIt in_last,
  OutputIt out_first,
  const OutputIt out_last,
  UnaryFunction f)
{
  utility::countdown barrier{ std::min(
    static_cast<std::size_t>(std::distance(in_first, in_last)),
    static_cast<std::size_t>(std::distance(out_first, out_last))) };
  std::for_each(in_first, in_last, [&pool, &barrier, &out_first, &f](auto&& value) mutable {
    if (barrier.valid())
    {
      auto& out = *out_first;
      pool.emplace([&barrier, &out, &f, &value]() mutable {
        out = f(value);
        --barrier;
      });
      ++out_first;
    }
  });
  barrier.wait();
  return out_first;
}

}  // namespace para::algorithm