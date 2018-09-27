//
// Created by fotyev on 2016-11-16.
//

#ifndef ZENGINE_MOVABLE_HPP
#define ZENGINE_MOVABLE_HPP

#include <utility>
#include <boost/preprocessor/seq/for_each.hpp>


// GENERATE_SWAP(test, (field1) (field2))
// expands to:
// void swap(test& other) noexcept { using std::swap; swap(field1, rhs.field1); swap(field1, rhs.field2); }
#define GENERATE_SWAP(classname, seq) \
void swap(classname& other) noexcept { \
using std::swap; \
BOOST_PP_SEQ_FOR_EACH(GENERATE_SWAP_HELPER, other, seq) \
}

#define GENERATE_SWAP_HELPER(r, other, elem) swap(elem, other . elem);


#define NONCOPYABLE(classname) \
classname(const classname&) = delete; \
classname& operator =(const classname&) = delete;


// generates swap , move-assign and move-contruct functions and deletes copy-assign and copy-construct
#define NONCOPYABLE_BUT_SWAPPABLE(classname, fields) \
public: NONCOPYABLE(classname) \
GENERATE_SWAP(classname, fields) \
classname(classname&& rhs) noexcept { this->swap(rhs); } \
classname& operator = (classname&& rhs) noexcept { this->swap(rhs); return(*this); } \
private: friend void swap(classname& lhs, classname& rhs) noexcept { lhs.swap(rhs); }
#endif //ZENGINE_MOVABLE_HPP
