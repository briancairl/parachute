/**
 * @copyright 2023-present Brian Cairl
 *
 * @file transform.cpp
 */

// C++ Standard Library
#include <algorithm>
#include <vector>

// GTest
#include <gtest/gtest.h>

// Parachute
#include <parachute/algorithm/transform.hpp>
#include <parachute/pool.hpp>

using namespace para;


TEST(TransformUnordered, EmptySequence)
{
  using pool_type = worker;

  pool_type wp;

  const std::vector<double> original_sequence = {};

  std::vector<double> expected_sequence = original_sequence;
  std::for_each(expected_sequence.begin(), expected_sequence.end(), [](double& v) { v *= 2; });

  std::vector<double> transformed_sequence;
  algorithm::transform(
    wp, original_sequence.begin(), original_sequence.end(), std::back_inserter(transformed_sequence), [](double v) {
      return v * 2;
    });

  std::sort(transformed_sequence.begin(), transformed_sequence.end());
  EXPECT_EQ(transformed_sequence, expected_sequence);
}


TEST(TransformUnordered, FullSequence)
{
  using pool_type = static_pool<4>;

  pool_type wp;

  const std::vector<double> original_sequence = { 1, 2, 3, 4, 5, 6 };

  std::vector<double> expected_sequence = original_sequence;
  std::for_each(expected_sequence.begin(), expected_sequence.end(), [](double& v) { v *= 2; });

  std::vector<double> transformed_sequence;
  algorithm::transform(
    wp, original_sequence.begin(), original_sequence.end(), std::back_inserter(transformed_sequence), [](double v) {
      return v * 2;
    });

  std::sort(transformed_sequence.begin(), transformed_sequence.end());
  EXPECT_EQ(transformed_sequence, expected_sequence);
}


TEST(TransformOrdered, EmptySequence)
{
  using pool_type = static_pool<4>;

  pool_type wp;

  const std::vector<double> original_sequence = {};

  std::vector<double> expected_sequence = original_sequence;
  std::for_each(expected_sequence.begin(), expected_sequence.end(), [](double& v) { v *= 2; });

  std::vector<double> transformed_sequence;
  transformed_sequence.resize(original_sequence.size());
  algorithm::transform(
    wp,
    original_sequence.begin(),
    original_sequence.end(),
    transformed_sequence.begin(),
    transformed_sequence.end(),
    [](double v) { return v * 2; });

  EXPECT_EQ(transformed_sequence, expected_sequence);
}


TEST(TransformOrdered, FullSequence)
{
  using pool_type = static_pool<4>;

  pool_type wp;

  const std::vector<double> original_sequence = { 1, 2, 3, 4, 5, 6 };

  std::vector<double> expected_sequence = original_sequence;
  std::for_each(expected_sequence.begin(), expected_sequence.end(), [](double& v) { v *= 2; });

  std::vector<double> transformed_sequence;
  transformed_sequence.resize(original_sequence.size());
  algorithm::transform(
    wp,
    original_sequence.begin(),
    original_sequence.end(),
    transformed_sequence.begin(),
    transformed_sequence.end(),
    [](double v) { return v * 2; });

  EXPECT_EQ(transformed_sequence, expected_sequence);
}


TEST(TransformOrdered, UndersizedOupoutSequence)
{
  using pool_type = static_pool<4>;

  pool_type wp;

  const std::vector<double> original_sequence = { 1, 2, 3, 4, 5, 6 };

  std::vector<double> expected_sequence = original_sequence;
  std::for_each(expected_sequence.begin(), expected_sequence.end(), [](double& v) { v *= 2; });

  std::vector<double> transformed_sequence;
  transformed_sequence.resize(original_sequence.size() / 2 + 1);
  algorithm::transform(
    wp,
    original_sequence.begin(),
    original_sequence.end(),
    transformed_sequence.begin(),
    transformed_sequence.end(),
    [](double v) { return v * 2; });

  EXPECT_NE(transformed_sequence, expected_sequence);
}
