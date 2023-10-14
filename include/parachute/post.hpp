/**
 * @copyright 2023-present Brian Cairl
 *
 * @file post.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>
#include <exception>

namespace std
{
template<typename T> class promise;
}  // namespace std

namespace para
{

template <typename WorkGroupT, typename WorkQueueT, typename WorkPoolOptionsT> class pool_base;
template <typename T> class non_blocking_promise;

namespace strategy
{
  template <typename T> using blocking = ::std::promise<T>;
  template <typename T> using non_blocking = non_blocking_promise<T>;
}  // namespace strategy

/**
 * @brief Enqueues work to a work pool and returns a tracker for that work
 */
template <
  template<typename> class PromiseTmpl,
  typename WorkGroupT,
  typename WorkQueueT,
  typename WorkPoolOptionsT,
  typename WorkT,
  typename ResultT = std::invoke_result_t<std::remove_reference_t<WorkT>>>
[[nodiscard]] auto post(pool_base<WorkGroupT, WorkQueueT, WorkPoolOptionsT>& wp, WorkT&& work)
{
  auto p = new PromiseTmpl<ResultT>{};
  auto f = p->get_future();
  wp.emplace(
    [p, w = std::forward<WorkT>(work)]() mutable {
      try
      {
        if constexpr (std::is_same_v<ResultT, void>)
        {
          w();
          p->set_value();
        }
        else
        {
          p->set_value(w());
        }
      }
      catch (...)
      {
        p->set_exception(std::current_exception());
      }
      delete p;
    }
  );
  return f;
}

}  // namespace para
