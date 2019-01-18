//
// Created by fotyev on 2018-07-24.
//

#ifndef ZENGINE_STRING_HPP
#define ZENGINE_STRING_HPP

#include <array>
#include <vector>
#include <string_view>

using std::string_view;

// example: auto a = string_split<2>("x  y  z", ' '); a[0] == "x"; a[1] == "y  z";
template <unsigned N, typename T>
inline std::array<string_view, N> string_split(string_view str, T delimiters) noexcept
{
    string_view::size_type pos = 0;
    std::array<string_view, N> arr;
    for(unsigned i = 0; i < N && pos != string_view::npos; i++)
    {
        auto np = str.find_first_of(delimiters, pos);
        if(np == string_view::npos || i == N - 1)
        {
            arr[i] = str.substr(pos);
            return arr;
        }
        arr[i] = str.substr(pos, np - pos);
        pos = str.find_first_not_of(delimiters, np);
    }
    return arr;
}

// vector
template <typename T>
inline std::vector<string_view> string_split_v(string_view str, T delimiters) noexcept
{
    string_view::size_type pos = 0;
    std::vector<string_view> arr;
    do
    {
        auto np = str.find_first_of(delimiters, pos);
        if(np == string_view::npos)
        {
            arr.push_back(str.substr(pos));
            return arr;
        }
        arr.push_back(str.substr(pos, np - pos));
        pos = str.find_first_not_of(delimiters, np);
    }
    while(pos != string_view::npos);
    return arr;
}

inline constexpr bool begins_with(string_view str, string_view beg) noexcept
{
    return str.size() >= beg.size() && str.substr(0, beg.size()) == beg; // && strncmp(str.data(), beg.data(), beg.size()) == 0
}




#endif //ZENGINE_STRING_HPP
