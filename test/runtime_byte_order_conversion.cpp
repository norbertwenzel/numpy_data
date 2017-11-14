// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// Copyright (C) Norbert Wenzel 2017.

#define BOOST_TEST_MODULE runtime_byte_order_conversion

#include <cassert>
#include <climits>
#include <cmath>
#include <cstdint>
#include <type_traits>

#include <boost/endian/conversion.hpp>
#include <boost/mpl/list.hpp>
#include <boost/test/included/unit_test.hpp>

#include "numpy_data.hpp"

//left shift/output operator used in Boost.Test
namespace numpy
{
namespace data
{

template<typename OStream>
OStream& operator<<(OStream &out, const byte_order bo)
{
    switch(bo)
    {
    case byte_order::little_endian:
        return out << "byte_order::little_endian";
    case byte_order::big_endian:
        return out << "byte_order::big_endian";
    case byte_order::unknown:
        return out << "byte_order::unknown";
    }
    assert(false && "Unknown value for byte_order enum");
    return out;
}

} //namespace data
} //namespace numpy

//typedefs for runtime endianness, static little and big endian
using rte = numpy::data::detail::runtime_byte_order_conversion;
using sle = numpy::data::detail::little_endian_byte_order;
using sbe = numpy::data::detail::big_endian_byte_order;


BOOST_AUTO_TEST_SUITE(numpy_test)

BOOST_AUTO_TEST_CASE(current_endianness)
{
    const auto bo = rte::current_endianness();
    if(boost::endian::order::native == boost::endian::order::little)
    {
        BOOST_REQUIRE_EQUAL(bo, numpy::data::byte_order::little_endian);
    }
    else if(boost::endian::order::native == boost::endian::order::big)
    {
        BOOST_REQUIRE_EQUAL(bo, numpy::data::byte_order::big_endian);
    }
    else
    {
        BOOST_REQUIRE_EQUAL(bo, numpy::data::byte_order::unknown);
        BOOST_TEST_MESSAGE("The system endianness is unknown. This has been detected correctly, "
                           "but the actual NumPy export will only work on either little or big "
                           "endian systems.");
    }
}

BOOST_AUTO_TEST_CASE(fixed_endianness)
{
    //check static variants current_endianness() function
    BOOST_CHECK_EQUAL(sle::current_endianness(), numpy::data::byte_order::little_endian);
    BOOST_CHECK_EQUAL(sbe::current_endianness(), numpy::data::byte_order::big_endian);
}

//C++11 std::min is not constexpr
template<typename T>
constexpr const T& cmin(const T &a, const T &b)
{
    return b > a ? a : b;
}

//list of endian types, ie. types Boost.Endian supports
typedef boost::mpl::list<std::int8_t,  std::int16_t,  std::int32_t,  std::int64_t,
                         std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t> endian_types;
BOOST_AUTO_TEST_CASE_TEMPLATE(to_little_endian, T, endian_types)
{
    namespace be = boost::endian;
    using limits = std::numeric_limits<T>;
    static constexpr T step_size = 1;
    static constexpr T num_steps = cmin<T>(step_size * 100, limits::max());

    //check around the range minimum
    for(T i = limits::lowest();
        i <= limits::lowest() + step_size * num_steps;
        i += step_size)
    {
        const auto expected = be::native_to_little(i);

        BOOST_CHECK_BITWISE_EQUAL(rte::to_little_endian(i), expected);
        BOOST_CHECK_BITWISE_EQUAL(sle::to_little_endian(be::native_to_little(i)), expected);
        BOOST_CHECK_BITWISE_EQUAL(sbe::to_little_endian(be::native_to_big(i)), expected);
    }

    if(std::is_signed<T>::value)
    {
        //check around zero for signed types only
        for(T i = -(step_size * num_steps);
            i <= step_size * num_steps;
            i += step_size)
        {
            const auto expected = be::native_to_little(i);

            BOOST_CHECK_BITWISE_EQUAL(rte::to_little_endian(i), expected);
            BOOST_CHECK_BITWISE_EQUAL(sle::to_little_endian(be::native_to_little(i)), expected);
            BOOST_CHECK_BITWISE_EQUAL(sbe::to_little_endian(be::native_to_big(i)), expected);
        }
    }

    //check around the range maximum
    for(T i = limits::max();
        i >= limits::max() - step_size * num_steps;
        i -= step_size)
    {
        const auto expected = be::native_to_little(i);

        BOOST_CHECK_BITWISE_EQUAL(rte::to_little_endian(i), expected);
        BOOST_CHECK_BITWISE_EQUAL(sle::to_little_endian(be::native_to_little(i)), expected);
        BOOST_CHECK_BITWISE_EQUAL(sbe::to_little_endian(be::native_to_big(i)), expected);
    }
}

BOOST_AUTO_TEST_SUITE_END()
