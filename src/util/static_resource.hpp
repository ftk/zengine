//
// Created by fotyev on 2018-10-24.
//

#ifndef ZENGINE_STATIC_RESOURCE_HPP
#define ZENGINE_STATIC_RESOURCE_HPP

#define ID(x) []() constexpr { return x; }

#include <optional>
#include <type_traits>

#include "util/resource_traits.hpp"
#include "util/assert.hpp"

template <class Rsc>
class static_resource
{
    template <typename Id>
    static/* inline*/ std::optional<Rsc> resource;

public:
    template <typename Id>
    static Rsc& get(Id id)
    {
        static_assert(std::is_invocable_v<Id>);
        if(LIKELY(resource<Id>))
            return *resource<Id>;
        return resource<Id>.emplace(resource_traits<Rsc>::from_id(id()));
    }

    template <typename Id>
    static Rsc& at(Id id)
    {
        assume(resource<Id>);
        return *resource<Id>;
    }

    template <typename Id, typename F>
    static Rsc& get(Id id, F callback)
    {
        if(LIKELY(resource<Id>))
            return *resource<Id>;
        return resource<Id>.emplace(callback());
    }

    template <typename Id>
    static std::optional<Rsc>& get_optional(Id id)
    {
        return resource<Id>;
    }


    template <typename Id>
    static Rsc& reset(Id id)
    {
        resource<Id>.reset();
    }

};

template <class Rsc>
template <typename>
std::optional<Rsc> static_resource<Rsc>::resource;


#endif //ZENGINE_STATIC_RESOURCE_HPP
