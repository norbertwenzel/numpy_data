// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// Copyright (C) Norbert Wenzel 2016 - 2017.

#ifndef NUMPY_DATA_HPP
#define NUMPY_DATA_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <limits>
#include <ostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace numpy
{
namespace data
{

template<typename Number>
struct array_data_traits
{
    static_assert(std::is_arithmetic<typename std::decay<Number>::type>::value,
                  "Number type needed.");

    using value_type = Number;
    using scalar_type = value_type;
    using pointer_type = typename std::add_pointer<typename std::add_const<scalar_type>::type>::type;
    static constexpr std::size_t dimensions = 1;

    static pointer_type access(const Number &i, const std::size_t)
    {
        return std::addressof(i);
    }
};

enum class byte_order { unknown, big_endian, little_endian };

namespace detail
{

template<typename T>
void swap_byte_order(T* const t)
{
    //do not accept floating point numbers, structs or other complicated stuff
    static_assert(std::is_integral<T>::value, "only integral type byte swap supported");

    using byte = std::uint8_t;
    byte* const p = reinterpret_cast<byte*>(t);
    std::reverse(p, std::next(p, sizeof(T)));
}

template<typename T>
void swap_byte_order(T &t)
{
    swap_byte_order(std::addressof(t));
}

struct runtime_byte_order_conversion
{
    static byte_order current_endianness()
    {
        static const auto bo = []
        {
            constexpr std::uint32_t i = 0x00010203;
            const std::uint8_t* const p = reinterpret_cast<const std::uint8_t*>(std::addressof(i));
            if(p[0] == 3 && p[3] == 0) { return byte_order::little_endian; }
            else if(p[0] == 0 && p[3] == 3) { return byte_order::big_endian; }
            else { return byte_order::unknown; }
        }(); //call temporary lambda to initialize constant once
        return bo; //simply return precomputed value
    }

    template<typename T>
    static T to_little_endian(T val)
    {
        const auto endianness = current_endianness();
        if(endianness == byte_order::unknown) { throw std::runtime_error("Unknown endianness"); }

        assert(endianness == byte_order::big_endian || endianness == byte_order::little_endian);
        //swap bytes to get little endian data
        if(endianness == byte_order::big_endian) { swap_byte_order(val); }
        //else: already little_endian so just return val

        return val;
    }
};

struct little_endian_byte_order
{
    static constexpr byte_order current_endianness() { return byte_order::little_endian; }

    template<typename T> inline
    static constexpr const T& to_little_endian(const T &val) { return val; }
};

struct big_endian_byte_order
{
    static constexpr byte_order current_endianness() { return byte_order::big_endian; }

    template<typename T> inline
    static T to_little_endian(T val) { swap_byte_order(val); return val; }
};

struct default_storage_tag {};
struct contiguous_storage_tag {};

template<typename V, typename S, std::size_t D>
struct storage_layout
{
    //if the size of the whole type is just D repetitions of scalar types storage is contiguous
    using type = typename std::conditional<sizeof(V) == sizeof(S) * D,
                                           contiguous_storage_tag,
                                           default_storage_tag>::type;
};

template<typename T>
struct storage_layout<T, T, 1>
{
    //if there is a one-dimensional point (ie. a scalar) that is directly stored in the
    //container (ie. iterator value_type and scalar_type are equal) we have contiguous storage
    using type = contiguous_storage_tag;
};

constexpr char dtype_byte_order(byte_order bo)
{
#if defined(__GNUC__) && __GNUC__ < 5
    //GCC <5 does not compile the comma operator assertion in constexpr, so remove the assertion
    //for all GCC major versions below 5
    return
#else
    //C++11 does not allow assertions in constexpr functions, therefore use comma operator to make
    //this a function containing *only* a single return statement
    return assert((bo == byte_order::big_endian || bo == byte_order::little_endian) &&
                  "Specify a valid byte order for NumPy export."),
#endif
    //actual return
        (bo == byte_order::unknown ? '?' : (bo == byte_order::little_endian ? '<' : '>'));
}

template<typename T, bool IsSigned, bool IsIntegral>
struct dtype_description
{
    static constexpr char code = 0;
};

template<typename T>
struct dtype_description<T, true, true>
{
    static_assert(std::is_integral<T>::value && std::is_signed<T>::value,
                  "Expect signed integral type.");
    static constexpr char code = 'i';
};

template<typename T>
struct dtype_description<T, false, true>
{
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value,
                  "Expect unsigned integral type.");
    static constexpr char code = 'u';
};

template<>
struct dtype_description<bool, false, true>
{
    static constexpr char code = 'b';
};

template<typename T, bool IsSigned>
struct dtype_description<T, IsSigned, false>
{
    static_assert(std::is_floating_point<T>::value,
                  "Expect floating point type.");
    static constexpr char code = 'f';
};

template<typename T>
constexpr char dtype_type_code()
{
    static_assert(std::is_arithmetic<T>::value, "Only arrays of arithmetic types supported.");
    return dtype_description<T, std::is_signed<T>::value, std::is_integral<T>::value>::code;
}

template<typename T, typename EndianConv, typename OStream>
void write_dtype_description(OStream &out)
{
    out << dtype_byte_order(EndianConv::current_endianness());
    out << dtype_type_code<T>();
    out << sizeof(T);
}

template<typename ShapeDesc, typename OStream>
void write_shape_description(OStream &out, const ShapeDesc &shape)
{
    using shape_value = typename std::decay<decltype(*std::begin(shape))>::type;
    static_assert(std::is_integral<shape_value>::value, "Shape description requires integer types");
    assert(std::distance(std::begin(shape), std::end(shape)) > 0);
#ifndef NDEBUG
    for(const auto s : shape) { assert(s >= 0 && "Shape dimension needs to be positive"); }
#endif //NDEBUG

    auto cur = std::begin(shape);
    out << "(";
    //write the first element and *always* a comma (due to Python tuple syntax)
    out << *cur << ",";
    ++cur;
    if(cur != std::end(shape))
    {
        out << " ";
        //iterate over shape *without* the last element
        std::copy(cur, std::end(shape) - 1,
                std::ostream_iterator<shape_value>(out, ", "));
        //print the last element and close parenthesis
        out << *(std::end(shape) - 1);
    }
    out << ")";
}

template<typename T, typename EndianConv, typename ShapeDesc>
std::string create_header_dictionary(bool fortran_order, const ShapeDesc &shape)
{
    //{'descr': '<f8', 'fortran_order': False, 'shape': (40256, 3), }
    std::ostringstream dict;
    dict << "{'descr': '";
    write_dtype_description<T, EndianConv>(dict);
    dict << "', "
        "'fortran_order': " << (fortran_order ? "True" : "False") << ", "
        "'shape': ";
    write_shape_description(dict, shape);
    dict << "}";
    return dict.str();
}

// define magic header for NumPy files. use non-uniform initialization for 0x93 to force constant
// to signed int8 without narrowing conversion from intermediate integer even on older compilers
static constexpr std::int8_t magic_header[] = {std::int8_t(0x93), 'N', 'U', 'M', 'P', 'Y'};

template<typename EndianConv, typename Iter, typename OStream, typename ShapeDesc>
void write_header(OStream &out, const ShapeDesc &shape, bool fortran_order)
{
    for(auto c : magic_header) { out.put(c); }

    //fail_print_type<typename array_data_traits<typename std::iterator_traits<Iter>::value_type>::scalar_type>{};

    //create the header dictionary
    std::string header_dict = create_header_dictionary<
                                                typename array_data_traits<
                                                    typename std::iterator_traits<Iter>::value_type
                                                >::scalar_type,
                                                EndianConv
                                            >(fortran_order, shape);

    //determine the numpy data format version (depending on length of header)
    static constexpr std::uint16_t v1_max_length = std::numeric_limits<std::uint16_t>::max() -
                                                   std::uint16_t{1};
    const std::uint8_t format_version = (header_dict.size() >= v1_max_length ? 2 : 1);
    assert(format_version == 1 || format_version == 2);

    //write data format version
    out.put(format_version); out.put(std::uint8_t{0});

    //compute necessary padding for the header
    const std::size_t total_header_length = sizeof(magic_header) +
                                            2 + //the format version major and minor number
                                            (format_version == 1 ? sizeof(std::uint16_t) :
                                                                   sizeof(std::uint32_t)) +
                                            header_dict.size() + 1; //count newline character
    const std::uint8_t padding_length = static_cast<std::uint8_t>(
        (16 - (total_header_length % 16)) % 16);
    assert(padding_length < 16);

    //compute the length of the header (including padding and the newline character)
    const auto header_len = header_dict.size() + padding_length + 1;
    assert((format_version == 1 && header_len <= std::numeric_limits<std::uint16_t>::max()) ||
           (format_version == 2 && header_len <= std::numeric_limits<std::uint32_t>::max()));

    //write the number of bytes that form the rest of the header
    if(format_version == 1)
    {
        assert(header_len <= std::numeric_limits<std::uint16_t>::max());
        std::uint16_t len = EndianConv::to_little_endian(static_cast<std::uint16_t>(header_len));
        out.write(reinterpret_cast<const char*>(std::addressof(len)), sizeof(len));
    }
    else if(format_version == 2)
    {
        assert(header_len <= std::numeric_limits<std::uint32_t>::max());
        std::uint32_t len = EndianConv::to_little_endian(static_cast<std::uint32_t>(header_len));
        out.write(reinterpret_cast<const char*>(std::addressof(len)), sizeof(len));
    }

    //write the header
    out << header_dict;

    //write spaces as header padding
    for(std::uint8_t i = 0; i < padding_length; ++i) { out.put(' '); }

    //write final newline
    out.put('\n');

    //assume we have now written a multiple of 16 characters for the whole header
    assert(out.tellp() % 16 == 0);
}

template<typename Iter, typename IterCat>
constexpr bool is_contiguous_impl(Iter, Iter, IterCat)
{
    //only random access iterators will be checked
    return false;
}

template<typename RandIter>
bool is_contiguous_impl(RandIter begin, RandIter end, std::random_access_iterator_tag)
{
    using std::addressof;
    using std::distance;
    return distance(begin, end) > 0 &&
           distance(addressof(*begin), addressof(*(end - 1))) == distance(begin, (end - 1));
}

template<typename Iter>
bool is_contiguous(Iter begin, Iter end)
{
    return is_contiguous_impl(begin, end, typename std::iterator_traits<Iter>::iterator_category{});
}

template<typename OStream, typename Iter>
void write_data_impl(OStream &out, Iter begin, Iter end, contiguous_storage_tag)
{
    using array_data = array_data_traits<typename std::iterator_traits<Iter>::value_type>;
    using data_type = typename array_data::scalar_type;

    //this is a runtime check for random access iterators only. everything else is discarded as
    //non-contiguous at compile-time
    if(is_contiguous(begin, end))
    {
        out.write(reinterpret_cast<const char *>(array_data::access(*begin, 0)),
                  sizeof(data_type) * array_data::dimensions * std::distance(begin, end));
    }
    else
    {
        for( ; begin != end; ++begin)
        {
            out.write(reinterpret_cast<const char *>(array_data::access(*begin, 0)),
                      sizeof(data_type) * array_data::dimensions);
        }
    }
}

template<typename OStream, typename Iter>
void write_data_impl(OStream &out, Iter begin, Iter end, default_storage_tag)
{
    using array_data = array_data_traits<typename std::iterator_traits<Iter>::value_type>;
    using data_type = typename array_data::scalar_type;

    for( ; begin != end; ++begin)
    {
        //write each coefficient on its own
        for(std::size_t d = 0; d < array_data::dimensions; ++d)
        {
            out.write(reinterpret_cast<const char *>(array_data::access(*begin, d)),
                      sizeof(data_type));
        }
    }
}

template<typename OStream, typename Iter>
void write_data(OStream &out, Iter begin, Iter end)
{
    using array_data = array_data_traits<typename std::iterator_traits<Iter>::value_type>;
    using storage_tag = typename storage_layout<typename array_data::value_type,
                                                typename array_data::scalar_type,
                                                array_data::dimensions>::type;

    write_data_impl(out, begin, end, storage_tag{});
}

template<typename Iter>
constexpr std::size_t dims()
{
    return array_data_traits<typename std::iterator_traits<Iter>::value_type>::dimensions;
}

} //namespace detail


template<typename EndianConv = detail::runtime_byte_order_conversion,
         typename OStream, typename Iter, typename ShapeDesc>
void write(OStream &out, Iter begin, Iter end, const ShapeDesc &shape, bool fortran_order = false)
{
    assert(out.good());
    detail::write_header<EndianConv, Iter>(out, shape, fortran_order);
    detail::write_data(out, begin, end);
}

template<typename EndianConv = detail::runtime_byte_order_conversion,
         typename OStream, typename Iter>
void write(OStream &out, Iter begin, Iter end)
{
    assert(out.good());
    assert(std::distance(begin, end) >= 0);
    static constexpr bool fortran_order = false;

    if(detail::dims<Iter>() == 1)
    {
        using shape = std::array<std::size_t, 1>;
        detail::write_header<EndianConv, Iter>(
                                        out,
                                        shape{{static_cast<std::size_t>(std::distance(begin, end))}},
                                        fortran_order);
    }
    else
    {
        using shape = std::array<std::size_t, 2>;
        detail::write_header<EndianConv, Iter>(
                                        out,
                                        shape{{static_cast<std::size_t>(std::distance(begin, end)),
                                              detail::dims<Iter>()}},
                                        fortran_order);
    }
    detail::write_data(out, begin, end);
}

} //namespace data
} //namespace numpy

#endif //NUMPY_DATA_HPP
