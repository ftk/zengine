//
// Created by fotyev on 2018-10-24.
//

#ifndef ZENGINE_STATIC_RESOURCE_HPP
#define ZENGINE_STATIC_RESOURCE_HPP


#include <optional>
#include <type_traits>

#include "util/resource_traits.hpp"
#include "util/const_strid.hpp"
#include "util/assert.hpp"

template <class Rsc>
class static_resource
{
    template <typename Id>
    static/* inline*/ std::optional<Rsc> resource;

public:
    static Rsc& get(auto id)
    {
        using Id = decltype(detail::idval2type(id));
        if(LIKELY(resource<Id>))
            return *resource<Id>;
        return resource<Id>.emplace(resource_traits<Rsc>::from_id(id()));
    }

    static Rsc& at(auto id)
    {
        using Id = decltype(detail::idval2type(id));

        assume(resource<Id>);
        return *resource<Id>;
    }

    static Rsc& get(auto id, auto callback)
    {
        using Id = decltype(detail::idval2type(id));
        if(LIKELY(resource<Id>))
            return *resource<Id>;
        return resource<Id>.emplace(std::move(callback)());
    }

    static std::optional<Rsc>& get_optional(auto id)
    {
        using Id = decltype(detail::idval2type(id));
        return resource<Id>;
    }


    static Rsc& reset(auto id)
    {
        using Id = decltype(detail::idval2type(id));
        resource<Id>.reset();
    }

};

template <class Rsc>
template <typename>
std::optional<Rsc> static_resource<Rsc>::resource;


#endif //ZENGINE_STATIC_RESOURCE_HPP
