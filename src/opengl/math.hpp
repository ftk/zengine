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



/*<
 my $s = '';
 for my $d (1..4) {
 my $out;
 $out .= " << ' ' << A<$_>(rhs)" for (0..$d-1);

 $s .= qq%
 // output vec$d
 template <typename T>
 std::enable_if_t<is_vec<T>::value && vec_traits<T>::dim == $d, std::ostream&>
 inline operator << (std::ostream& lhs, const T& rhs) {
    return(lhs << '(' $out << ' ' << ')');
 }
%;
}
for my $d (1..4) {
 my $out;
 $out .= " << row<$_>(rhs) << '\\n'" for (0..$d-1);

 $s .= qq%
 // output mat$d
 template <typename T>
 std::enable_if_t<is_mat<T>::value && mat_traits<T>::rows == $d, std::ostream&>
 inline operator << (std::ostream& lhs, const T& rhs) {
    return(lhs $out << '\\n');
 }
%;
}

return $s;
%*/
 // output vec1
 template <typename T>
 std::enable_if_t<is_vec<T>::value && vec_traits<T>::dim == 1, std::ostream&>
 inline operator << (std::ostream& lhs, const T& rhs) {
    return(lhs << '('  << ' ' << A<0>(rhs) << ' ' << ')');
 }

 // output vec2
 template <typename T>
 std::enable_if_t<is_vec<T>::value && vec_traits<T>::dim == 2, std::ostream&>
 inline operator << (std::ostream& lhs, const T& rhs) {
    return(lhs << '('  << ' ' << A<0>(rhs) << ' ' << A<1>(rhs) << ' ' << ')');
 }

 // output vec3
 template <typename T>
 std::enable_if_t<is_vec<T>::value && vec_traits<T>::dim == 3, std::ostream&>
 inline operator << (std::ostream& lhs, const T& rhs) {
    return(lhs << '('  << ' ' << A<0>(rhs) << ' ' << A<1>(rhs) << ' ' << A<2>(rhs) << ' ' << ')');
 }

 // output vec4
 template <typename T>
 std::enable_if_t<is_vec<T>::value && vec_traits<T>::dim == 4, std::ostream&>
 inline operator << (std::ostream& lhs, const T& rhs) {
    return(lhs << '('  << ' ' << A<0>(rhs) << ' ' << A<1>(rhs) << ' ' << A<2>(rhs) << ' ' << A<3>(rhs) << ' ' << ')');
 }

 // output mat1
 template <typename T>
 std::enable_if_t<is_mat<T>::value && mat_traits<T>::rows == 1, std::ostream&>
 inline operator << (std::ostream& lhs, const T& rhs) {
    return(lhs  << row<0>(rhs) << '\n' << '\n');
 }

 // output mat2
 template <typename T>
 std::enable_if_t<is_mat<T>::value && mat_traits<T>::rows == 2, std::ostream&>
 inline operator << (std::ostream& lhs, const T& rhs) {
    return(lhs  << row<0>(rhs) << '\n' << row<1>(rhs) << '\n' << '\n');
 }

 // output mat3
 template <typename T>
 std::enable_if_t<is_mat<T>::value && mat_traits<T>::rows == 3, std::ostream&>
 inline operator << (std::ostream& lhs, const T& rhs) {
    return(lhs  << row<0>(rhs) << '\n' << row<1>(rhs) << '\n' << row<2>(rhs) << '\n' << '\n');
 }

 // output mat4
 template <typename T>
 std::enable_if_t<is_mat<T>::value && mat_traits<T>::rows == 4, std::ostream&>
 inline operator << (std::ostream& lhs, const T& rhs) {
    return(lhs  << row<0>(rhs) << '\n' << row<1>(rhs) << '\n' << row<2>(rhs) << '\n' << row<3>(rhs) << '\n' << '\n');
 }
/*>*/

} // namespace qvm

namespace boost { namespace serialization {
// todo: optimize?
template <class Archive, class T, int D>
void serialize(Archive & ar, boost::qvm::vec<T, D> & vec, unsigned int)
{
    ar & vec.a;
};

}} // namespace boost

#endif //ZENGINE_MATH_HPP
