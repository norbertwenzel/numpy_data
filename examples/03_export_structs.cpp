#include "numpy_data.hpp"

#include <cassert>
#include <fstream>
#include <vector>


struct point3d { float x, y, z; };

namespace numpy
{
namespace data
{

//1. specialize trait for point3d
template<>
struct array_data_traits<point3d>
{
    using value_type = point3d; //2. the value_type of the iterator written to numpy
    using scalar_type = float; //3. the scalar type written as numpy array
    using pointer_type = typename std::add_pointer<typename std::add_const<scalar_type>::type>::type;
    static constexpr std::size_t dimensions = 3; //4. the number of dimensions of value_type

    static pointer_type access(const point3d &p, const std::size_t idx)
    {
        //5. return const pointer to desired element p[idx]
        assert(idx < dimensions);
        switch(idx)
        {
        case 0: return std::addressof(p.x);
        case 1: return std::addressof(p.y);
        case 2: return std::addressof(p.z);
        default: return nullptr;
        }
        //as long as three const float* are returned the actual point3d struct may contain any
        //members of any type, like point3d{ float x, y, z; bool some_flag; }.
    }
};

} //namespace data
} //namespace numpy

int main()
{
    std::vector<point3d> plist = {{ 0,  1,  2},
                                  { 3,  4,  5},
                                  { 6,  7,  8},
                                  { 9, 10, 11},
                                  {12, 13, 14}};

    std::ofstream fout{"03_export_structs.npy", std::ios::out | std::ios::binary};
    numpy::data::write(fout, plist.cbegin(), plist.cend());
    fout.close();

    return 0;
}
