#include "numpy_data.hpp"

#include <fstream>

int main()
{
    auto data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};

    //1. make sure to always open your stream in binary mode
    std::ofstream fout{"01_simple_array_export.npy", std::ios::out | std::ios::binary};
    //2. write all data defined by the iterator pair as binary NPY to the given ostream
    numpy::data::write(fout, std::begin(data), std::end(data));
    fout.close();

    return 0;
}
