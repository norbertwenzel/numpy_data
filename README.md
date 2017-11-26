# Export STL containers to NumPy arrays

A self-contained, header-only C++11 export for STL-like containers to binary NumPy array files
(`*.npy`).

## Description

Dealing with large arrays in C++ might be efficient, but it is not fun to debug. Debugging,
prototyping and visualization are easier done in Python. This exporter is therefore mainly used as
a debugging aid. Thus the code is only a single header file with no dependencies besides the C++11
standard library to easily (temporarily) integrate the exporter into existing code bases.

## Limitations

This code only exports to NumPy and does not read any NumPy files. It only exports simple
(arithmetic in C++) datatypes and structs consisting of such types and does not support object
arrays. It does not support zipped `*.npz` containers of multiple arrays.

If you need more than a file exporter or are just curious take a look at the following libraries:

* [cnpy](https://github.com/rogersce/cnpy) by Carl Rogers
* [Boost.Python](http://www.boost.org/doc/libs/1_63_0/libs/python/doc/html/index.html) (since 1.63)

## Installing

To use the library simply copy the [`include/numpy_data.hpp`](include/numpy_data.hpp) file into
your source tree (or adapt your project's include path accordingly).

To export your data use the following example:

```cpp
#include "numpy_data.hpp"
#include <fstream>
#include <vector>

std::vector<int> mydata;
//fill mydata

std::ofstream fout("mydata.npy", std::ios::out | std::ios::binary);
numpy::data::write(fout, mydata.cbegin(), mydata.cend());
fout.close();
```

## Running the tests

Two Docker containers are built to develop and test the code with different compilers. The tests use
[Boost.Tests](http://www.boost.org/doc/libs/1_65_1/libs/test/doc/html/index.html) which is packaged
in the test containers.

To create the test containers:

1. Open terminal in project root

2. Build test environments using Docker:
   `[sudo] ./create_testenv.sh`

   This builds test environments for both Ubuntu LTS versions available at the moment:
     * Ubuntu 14.04 Trusty Tahr (Docker image: `numpy_data/trusty`)
     * Ubuntu 16.04 Xenial Xerus  (Docker image: `numpy_data/xenial`)

   Both images have GCC (default) and Clang installed in the default version provided by the
   system's package manager. Both use Boost 1.58, since this is the first version with
   Boost.Endian which is required to run the tests. Xenial has that version in the default
   package repositories and for Trusty Boost is downloaded and built on container creation.

   If you want to build only a single configuration run either
   `[sudo] docker build -f docker/Dockerfile.trusty -t numpy_data/trusty .` or
   `[sudo] docker build -f docker/Dockerfile.xenial -t numpy_data/xenial .`

3. Run the Docker image:
   `[sudo] docker run -it --rm -v "$(pwd)":/numpy_data --user "$(id -u)" numpy_data/trusty` and/or
   `[sudo] docker run -it --rm -v "$(pwd)":/numpy_data --user "$(id -u)" numpy_data/xenial`

   This maps the current working directory (project root) into the container, so all changes are
   immediately available in both containers. Also all binaries created will be available outside of
   the container.

4. Run tests in the container:
   `bjam debug release toolset=gcc toolset=clang`

   Default toolset is GCC and default mode is debug. Important arguments to `bjam` are
     * `-j X` run multiple jobs in parallel
     * `-a` (re)build all given targets, even if they are already available
     * `bjam clean` use the clean target to remove binaries

   `bjam` is in both containers also available using the newer `b2` name.

### TODO/Issues:

* Minimize the container size, ie. do not install `build-essential` and `libboost-all-dev` but only
  the compiler and the tools required.

## License

This project is licensed under [Mozilla Public License Version 2.0](LICENSE.md).
