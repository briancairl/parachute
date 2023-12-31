/**
 * @copyright 2023-present Brian Cairl
 *
 * @file pool.cpp
 */

// C++ Standard Library
#include <chrono>
#include <future>
#include <thread>

// GTest
#include <gtest/gtest.h>

// Parachute
#include <parachute/non_blocking_future.hpp>
#include <parachute/pool.hpp>
#include <parachute/post.hpp>

using namespace para;


template <typename T> class PoolTestSuite : public ::testing::Test
{
public:
};

using PoolTestSuiteTypes =
  ::testing::Types<worker, worker_strict, static_pool<4>, static_pool_strict<4>, pool, pool_strict>;

TYPED_TEST_SUITE(PoolTestSuite, PoolTestSuiteTypes);

TYPED_TEST(PoolTestSuite, EmplaceAndDtor)
{
  using pool_type = TypeParam;

  pool_type wp;

  wp.emplace([] { ::std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
}

TYPED_TEST(PoolTestSuite, Post)
{
  using pool_type = TypeParam;

  pool_type wp;

  auto tracker = post<strategy::non_blocking>(wp, [] { return 1; });

  while (true)
  {
    if (tracker.valid())
    {
      ASSERT_EQ(tracker.get(), 1);
      break;
    }
    else
    {
      ::std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}

TYPED_TEST(PoolTestSuite, PostVoid)
{
  using pool_type = TypeParam;

  pool_type wp;

  auto tracker = post<strategy::non_blocking>(wp, [] {});

  while (true)
  {
    if (tracker.valid())
    {
      tracker.get();
      break;
    }
    else
    {
      ::std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
}

TYPED_TEST(PoolTestSuite, PostBlocking)
{
  using pool_type = TypeParam;

  pool_type wp;

  auto tracker = post(wp, [] { return 1; });

  ASSERT_EQ(tracker.get(), 1);
}
