// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// Copyright (C) Norbert Wenzel 2017.

#define BOOST_TEST_MODULE non_arithmetic_type_export

#include <iterator>

#include <boost/test/included/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include "numpy_data.hpp"

using boost::test_tools::output_test_stream;


BOOST_AUTO_TEST_SUITE(numpy_test)

BOOST_AUTO_TEST_CASE(pointer_export)
{
    const int* const arr[3] = {nullptr, nullptr, nullptr};

    output_test_stream out;
    numpy::data::write(out, std::begin(arr), std::end(arr));
}

BOOST_AUTO_TEST_SUITE_END()
