#include "numpy_data.hpp"

#include <array>
#include <fstream>


using mat4x4 = std::array<float, 16>;

int main()
{
    mat4x4 m = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    std::ofstream fout{"02_export_with_shape.npy", std::ios::out | std::ios::binary};
    //1. we know the one-dimensional array actually represents a 4x4 matrix
    const auto shape = {4, 4};
    //2. tell the exporter this array should be interpreted with our known shape (instead of (16,))
    numpy::data::write(fout, m.cbegin(), m.cend(), shape);
    fout.close();

    //shapes set manually are not checked. setting a wrong shape description will very likely create
    //invalid/unreadable NPY data.

    return 0;
}
