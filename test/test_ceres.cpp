// smooth: Lie Theory for Robotics
// https://github.com/pettni/smooth
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
//
// Copyright (c) 2021 Petter Nilsson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <gtest/gtest.h>

#include "smooth/compat/ceres.hpp"

#include "smooth/bundle.hpp"
#include "smooth/se2.hpp"
#include "smooth/se3.hpp"
#include "smooth/so2.hpp"
#include "smooth/so3.hpp"
#include "smooth/tn.hpp"


template<smooth::LieGroup G>
class CeresLocalParam : public ::testing::Test
{};

using GroupsToTest = ::testing::Types<
  smooth::SO2d, smooth::SO3d, smooth::SE2d, smooth::SE3d,
  smooth::Bundle<smooth::SO3d, smooth::T4d, smooth::SE2d>
>;

TYPED_TEST_SUITE(CeresLocalParam, GroupsToTest);

TYPED_TEST(CeresLocalParam, ComputeRandom)
{
  smooth::CeresLocalParameterization<TypeParam> param;

  static constexpr uint32_t p = TypeParam::RepSize;
  static constexpr uint32_t n = TypeParam::Dof;

  using ParamsT = Eigen::Matrix<double, p, 1>;

  // check that local parameterization gives expected sizes
  ASSERT_EQ(param.LocalSize(), n);
  ASSERT_EQ(param.GlobalSize(), p);


  for (std::size_t i = 0; i != 10; ++i) {
    // random group element and tangent vector
    TypeParam g = TypeParam::Random();
    Eigen::Matrix<double, n, 1> b = 1e-4 * Eigen::Matrix<double, n, 1>::Random();

    TypeParam gp = g + b;
    TypeParam gp_ceres;

    // compute plus
    param.Plus(g.data(), b.data(), gp_ceres.data());
    ASSERT_TRUE(gp.isApprox(gp_ceres));

    // compute jacobian from local parameterization
    Eigen::Matrix<double, p, n, (n > 1) ? Eigen::RowMajor : Eigen::ColMajor> jac;
    param.ComputeJacobian(g.data(), jac.data());

    // expect vee(hat(g) + b) \approx g + jac * b
    // where  hat(g) maps from parameters to the group
    // and    vee does the opposite
    ParamsT param = Eigen::Map<const ParamsT>(g.data());
    ParamsT param_plus = Eigen::Map<const ParamsT>(gp.data());

    ASSERT_TRUE(param_plus.isApprox(param + jac * b, 1e-6));
  }
}

