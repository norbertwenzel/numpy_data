# Examples

1. [Simple array export](01_simple_array_export.cpp): export a one-dimensional array to a binary NPY
   file.
2. [Export with user-defined shape](02_export_with_shape.cpp): export a one-dimensional array but
   set a user-defined shape description, which tells NumPy to interpret the array with a certain
   shape.
3. [Export user-defined structs](03_export_structs.cpp): specialize the internal `array_data_traits`
   for a user defined type (eg. a point or vector class) and export an array of user-defined structs.

## Building

To build these examples start any of the supplied Docker images and call `bjam` in this or the
projects's root directory (where the examples are built together with tests).
If you do not want to run Docker these examples can be trivially compiled using your compilers
command line interface in C++11 mode as long as the `numpy_data.hpp` file is made availabe. This can
be done by setting the proper include path or by copying the file into this folder.
