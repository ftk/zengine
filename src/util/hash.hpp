//
// Created by fotyev on 2016-10-18.
//

#ifndef ZENGINE_HASH_HPP
#define ZENGINE_HASH_HPP

#include <cstdint>
#include <utility>

#include <experimental/string_view>

using std::experimental::string_view;



template <unsigned N>
constexpr inline string_view const_string(const char (&s)[N])
{
    return string_view{s, N - 1}; // exclude \0
}



struct fnv1a
{
    constexpr static uint64_t offset_basis = 14695981039346656037ULL;
    constexpr static uint64_t prime = 1099511628211ULL;


// c string (recursive)
    constexpr static inline uint64_t hash(const char * const str, const uint64_t val = offset_basis)
    {
        return (str[0] == '\0') ? val : hash(&str[1], (val ^ uint64_t(str[0])) * prime);
    }

// string_view (recursive)
    constexpr static inline uint64_t hash(const string_view str, const uint64_t val = offset_basis)
    {
        return (str.length() == 0) ? val : hash(str.substr(1), (val ^ uint64_t(str[0])) * prime);
    }

// uint64_t
    constexpr static inline uint64_t hash(const uint64_t v, const uint64_t val = offset_basis)
    {
        return (val ^ v) * prime;
    }

    // T
    template <typename T>
    constexpr static inline uint64_t stdhash(const T& v, const uint64_t val = offset_basis)
    {
        return hash(std::hash<T>{}(v), val);
    }


    // right fold
    template <typename Arg1, typename... Args>
    constexpr static inline uint64_t foldr(Arg1&& arg1, Args&& ... args)
    {
        return hash(std::forward<Arg1>(arg1), foldr(std::forward<Args>(args)...));
    }

    template <typename Arg>
    constexpr static inline uint64_t foldr(Arg&& arg)
    {
        return hash(std::forward<Arg>(arg));
    }

};


// "string"_fnv compile time hashing
inline constexpr uint64_t operator "" _fnv (const char * str, const size_t len)
{
    return fnv1a::hash(string_view(str, len));
}

#ifdef TEST

static_assert(fnv1a::hash(" hash test") == 0xacf86bd432fd145d, "bad hash");

static_assert(fnv1a::hash("") == fnv1a::offset_basis, "empty string hash failed");
static_assert(fnv1a::hash(" hash test") == fnv1a::hash(string_view(" hash test", 10)), "hash test failed");
static_assert(fnv1a::hash(" hash test") == " hash test"_fnv, "hash test failed");

static_assert(fnv1a::hash(" hash test") == fnv1a::foldr("test", const_string(" "), ("hash"), " "), "fold test failed");

#endif

#endif //ZENGINE_HASH_HPP
