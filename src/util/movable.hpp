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


#define DEFAULT_MOVABLE(classname) classname(classname&&) = default; classname& operator = (classname&& rhs) = default;

#include <boost/preprocessor/tuple/rem.hpp>
#include <boost/preprocessor/tuple/to_seq.hpp>

#define COPY_ASSIGN_HELPER(r,other,elem) this-> elem = other . elem;
//#define COPY_CONSTRUCT_HELPER(r,other,elem) , elem { other . elem }

#define ENTT_COPYABLE(classname,registry,copy_components,fields) \
classname(const classname& other) noexcept { \
this-> registry .clear<BOOST_PP_REM copy_components>(); \
entt::clone<BOOST_PP_REM copy_components>(other.registry, this->registry); \
BOOST_PP_SEQ_FOR_EACH(COPY_ASSIGN_HELPER, other, BOOST_PP_TUPLE_TO_SEQ(fields)) \
}

#include "entt/entity/registry.hpp"
namespace entt {
  template <typename... Type>
  void clone(const entt::registry& from, entt::registry& to)
  {
    ([&]() {
      const auto *data = from.data<Type>();
      const auto size = from.size<Type>();

      if constexpr(ENTT_IS_EMPTY(Type))
      {
        to.insert<Type>(data, data + size);
      } else
      {
        const auto *raw = from.raw<Type>();
        to.insert<Type>(data, data + size, raw, raw + size);
      }
    }(), ...);

  }
}


#endif //ZENGINE_MOVABLE_HPP
