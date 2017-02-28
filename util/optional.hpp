//
// Created by fotyev on 2016-10-14.
//

#ifndef ZENGINE_OPTIONAL_HPP
#define ZENGINE_OPTIONAL_HPP

#include <experimental/optional>

#include "util/assert.hpp"

using std::experimental::optional;
using std::experimental::bad_optional_access;
using std::experimental::nullopt;
using std::experimental::nullopt_t;

template <typename T>
class optional_ref
{
    T * ptr;
public:
    constexpr optional_ref() : ptr(nullptr) {}
    constexpr optional_ref(T& ref) : ptr(&ref) {}
    constexpr optional_ref(const optional_ref<T>& opt) : ptr(opt.ptr) {}

    constexpr optional_ref<T>& operator = (nullopt_t) { ptr = nullptr; return (*this); }
    constexpr optional_ref<T>& operator = (T& ref) { ptr = &ref; return (*this); }
    constexpr optional_ref<T>& operator = (const optional_ref<T>& opt) { ptr = opt.ptr; return (*this); }

    constexpr explicit operator bool() const { return ptr != nullptr; }
    constexpr bool has_value() const { return ptr != nullptr; }

    constexpr void reset() { ptr = nullptr; }

    constexpr bool operator == (const optional_ref<T>& rhs) const { return ptr == rhs.ptr; }
    constexpr bool operator != (const optional_ref<T>& rhs) const { return ptr != rhs.ptr; }


#define CHECK_DEREF() if(!ptr) {throw bad_optional_access{};}
    constexpr T& value() { CHECK_DEREF() return *ptr; }
    constexpr const T& value() const { CHECK_DEREF() return *ptr; }

#ifdef NDEBUG
#undef CHECK_DEREF
#define CHECK_DEREF() assume(ptr);
#endif
    constexpr T& operator * () { CHECK_DEREF() return *ptr; }
    constexpr const T& operator * () const { CHECK_DEREF() return *ptr; }

    constexpr T* operator -> () { CHECK_DEREF() return ptr; }
    constexpr const T* operator -> () const { CHECK_DEREF() return ptr; }

#undef CHECK_DEREF

};

#endif //ZENGINE_OPTIONAL_HPP
