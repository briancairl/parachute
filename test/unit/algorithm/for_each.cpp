/**
 * @copyright 2023-present Brian Cairl
 *
 * @file for_each.cpp
 */

// C++ Standard Library
#include <algorithm>
#include <vector>

// GTest
#include <gtest/gtest.h>

// Parachute
#include <parachute/algorithm/for_each.hpp>
#include <parachute/pool.hpp>

using namespace para;


TEST(ForEach, EmptySequence)
{
  using pool_type = worker;

  pool_type wp;

  std::vector<double> mutated_seqeunce = {};

  std::vector<double> expected_seqeunce = mutated_seqeunce;
  std::for_each(expected_seqeunce.begin(), expected_seqeunce.end(), [](double& v) { v *= 2; });

  algorithm::for_each(wp, mutated_seqeunce.begin(), mutated_seqeunce.end(), [](double& v) { v *= 2; });

  EXPECT_EQ(mutated_seqeunce, expected_seqeunce);
}


TEST(ForEach, FullSequence)
{
  using pool_type = static_pool<4>;

  pool_type wp;

  std::vector<double> mutated_seqeunce = { 1, 2, 3, 4, 5, 6 };

  std::vector<double> expected_seqeunce = mutated_seqeunce;
  std::for_each(expected_seqeunce.begin(), expected_seqeunce.end(), [](double& v) { v *= 2; });

  algorithm::for_each(wp, mutated_seqeunce.begin(), mutated_seqeunce.end(), [](double& v) { v *= 2; });

  EXPECT_EQ(mutated_seqeunce, expected_seqeunce);
}
