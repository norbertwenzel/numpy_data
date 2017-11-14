// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// Copyright (C) Norbert Wenzel 2016 - 2017.

#ifndef NUMPY_DATA_EXTENSION_BOOST_ENDIAN_HPP
#define NUMPY_DATA_EXTENSION_BOOST_ENDIAN_HPP

#include "numpy_data.hpp"

#include <boost/config.hpp>
#include <boost/endian/conversion.hpp>

namespace numpy
{
namespace data
{
namespace ext
{
namespace detail
{

template<boost::endian::order>
struct static_endianness
{
    BOOST_STATIC_CONSTEXPR auto value = numpy::data::byte_order::unknown;
};

template<>
struct static_endianness<boost::endian::order::little>
{
    BOOST_STATIC_CONSTEXPR auto value = numpy::data::byte_order::little_endian;
};

template<>
struct static_endianness<boost::endian::order::big>
{
    BOOST_STATIC_CONSTEXPR auto value = numpy::data::byte_order::big_endian;
};

} //namespace detail

struct boost_endian_conversion
{
    static BOOST_CONSTEXPR byte_order current_endianness() BOOST_NOEXCEPT
    {
        return detail::static_endianness<boost::endian::order::native>::value;
    }

    template<typename T>
    inline static T to_little_endian(T val) BOOST_NOEXCEPT
    {
        boost::endian::native_to_little_inplace(val);
        return val;
    }
};

} //namespace ext
} //namespace data
} //namespace numpy

#endif //NUMPY_DATA_EXTENSION_BOOST_ENDIAN_HPP
