// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// Copyright (C) Norbert Wenzel 2017.

#define BOOST_TEST_MODULE array_export

#include <algorithm>
#include <array>
#include <initializer_list>
#include <vector>

#include <boost/config.hpp>
#include <boost/mpl/list.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include "numpy_data.hpp"

using boost::test_tools::output_test_stream;


BOOST_AUTO_TEST_SUITE(numpy_test)

//list of possible one dimensional array types expected as shape descriptors
typedef boost::mpl::list<std::size_t[1], std::array<std::size_t, 1>, std::vector<std::size_t>,
                         std::initializer_list<std::size_t>> one_dim_array_1val;
BOOST_AUTO_TEST_CASE_TEMPLATE(shape_descriptor_1, T, one_dim_array_1val)
{
    const T s = {42};

    output_test_stream out{"", true, false};
    numpy::data::detail::write_shape_description(out, s);
    BOOST_CHECK(out.is_equal("(42,)"));
}

//list of possible one dimensional array types expected as shape descriptors
typedef boost::mpl::list<std::size_t[2], std::array<std::size_t, 2>, std::vector<std::size_t>,
                         std::initializer_list<std::size_t>> one_dim_array_2vals;
BOOST_AUTO_TEST_CASE_TEMPLATE(shape_descriptor_2, T, one_dim_array_2vals)
{
    const T s = {3, 4};

    output_test_stream out{"", true, false};
    numpy::data::detail::write_shape_description(out, s);
    BOOST_CHECK(out.is_equal("(3, 4)"));
}

//list of possible one dimensional array types expected as shape descriptors
typedef boost::mpl::list<std::size_t[5], std::array<std::size_t, 5>, std::vector<std::size_t>,
                         std::initializer_list<std::size_t>> one_dim_array_5vals;
BOOST_AUTO_TEST_CASE_TEMPLATE(shape_descriptor_5, T, one_dim_array_5vals)
{
    const T s = {1, 2, 3, 4, 5};

    output_test_stream out{"", true, false};
    numpy::data::detail::write_shape_description(out, s);
    BOOST_CHECK(out.is_equal("(1, 2, 3, 4, 5)"));
}

//list of arithmetic types, that are supported by numpy export
typedef boost::mpl::list<std::int8_t,  std::int16_t,  std::int32_t,  std::int64_t,
                         std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t,
                         bool, float, double> arithmetic_types;
BOOST_AUTO_TEST_CASE_TEMPLATE(one_dim_array_export, T, arithmetic_types)
{
    BOOST_STATIC_CONSTEXPR std::size_t num_elements = 6;
    using array_t = std::array<T, num_elements>;

    array_t arr;
    std::generate_n(arr.begin(), arr.size(), []{ static T val = 0; val += 1; return val; });

    output_test_stream out;
    numpy::data::write(out, arr.cbegin(), arr.cend());
    BOOST_CHECK(!out.is_empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(two_dim_array_export, T, arithmetic_types)
{
    BOOST_STATIC_CONSTEXPR std::size_t num_elements = 6;
    using array_t = std::array<T, num_elements>;

    array_t arr;
    std::generate_n(arr.begin(), arr.size(), []{ static T val = 0; val += 1; return val; });

    output_test_stream out;
    const auto shape = {2, 3};
    numpy::data::write(out, arr.cbegin(), arr.cend(), shape);
    BOOST_CHECK(!out.is_empty(false));
}

BOOST_AUTO_TEST_SUITE_END()
