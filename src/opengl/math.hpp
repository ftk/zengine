//
// Created by fotyev on 2016-10-31.
//

#ifndef ZENGINE_MATH_HPP
#define ZENGINE_MATH_HPP

#include <boost/qvm/all.hpp>
#include <type_traits>

#include <ostream>


namespace qvm {

using namespace boost::qvm;
/*<
 my $x;
  $x .= "using vec$_ = vec<float, $_>;\n" for (1 .. 4);
  $x .= "using ivec${_} = vec<int, $_>;\n" for (1 .. 4);
  $x .= "using uvec${_} = vec<unsigned, $_>;\n" for (1 .. 4);

  $x .= "using mat$_ = mat<float, $_, $_>;\n" for (1 .. 4);
  #for my $i (1..4) {for my $j (1..4) { $x .= $i!=$j?"using mat${i}x${j} = mat<float, $i, $j>;\n":'';}}
  $x .= "constexpr auto identity_mat$_ = identity_mat<float, $_>;\n" for (1 .. 4);
  return $x;
 %*/using vec1 = vec<float, 1>;
using vec2 = vec<float, 2>;
using vec3 = vec<float, 3>;
using vec4 = vec<float, 4>;
using ivec1 = vec<int, 1>;
using ivec2 = vec<int, 2>;
using ivec3 = vec<int, 3>;
using ivec4 = vec<int, 4>;
using uvec1 = vec<unsigned, 1>;
using uvec2 = vec<unsigned, 2>;
using uvec3 = vec<unsigned, 3>;
using uvec4 = vec<unsigned, 4>;
using mat1 = mat<float, 1, 1>;
using mat2 = mat<float, 2, 2>;
using mat3 = mat<float, 3, 3>;
using mat4 = mat<float, 4, 4>;
constexpr auto identity_mat1 = identity_mat<float, 1>;
constexpr auto identity_mat2 = identity_mat<float, 2>;
constexpr auto identity_mat3 = identity_mat<float, 3>;
constexpr auto identity_mat4 = identity_mat<float, 4>;
/*>*/
using quat = quat<float>;



inline mat4 scale_mat4(float scale)
{
    return diag_mat(XXX1(vec1{scale}));
}

template <typename T>
T clamp(T val, T min, T max)
{
    return (val < min) ? min : ((val > max) ? max : val);
}

} // namespace qvm

// define input/output for vec*
namespace boost { namespace qvm {
    template <typename CharT, typename Scalar, int Dim>
    std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& out, boost::qvm::vec<Scalar, Dim> const& v)
    {
        out << '(';
        for(int i = 0; i < Dim; i++)
        {
            if(i)
                out << ',';
            out << v.a[i];
        }
        out << ')';
        return out;
    }

    template <typename CharT, typename Scalar, int Dim>
    std::basic_istream<CharT>& operator>>(std::basic_istream<CharT>& in, boost::qvm::vec<Scalar, Dim>& v)
    {
        CharT c = in.peek();
        bool brackets = false;

        // expect "3,4,5"

        if(!(c >= '0' && c <= '9') && c != '-')
        {
            // expect "(1,2,3)"
            in >> c;
            brackets = true;
        }


        for(int i = 0; i < Dim; i++)
        {
            if(i)
            {
                // separator (, ;)
                in >> c;
                if(c == '-' || (c >= '0' && c <= '9'))
                    throw std::runtime_error{"vec istream: Expected separator"};
            }
            in >> v.a[i];
        }
        if(brackets) // closing bracket
            in >> c;
        return in;
    }
}}

namespace cereal {
// todo: optimize?
template <class Archive, class T, int D>
void serialize(Archive& ar, boost::qvm::vec<T, D>& vec)
{
    ar & vec.a;
};

} // namespace cereal

#endif //ZENGINE_MATH_HPP
