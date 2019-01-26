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
    template <typename IdVal>
    static Rsc& get(IdVal id)
    {
        using Id = decltype(detail::idval2type(id));
        if(LIKELY(resource<Id>))
            return *resource<Id>;
        return resource<Id>.emplace(resource_traits<Rsc>::from_id(id()));
    }

    template <typename IdVal>
    static Rsc& at(IdVal id)
    {
        using Id = decltype(detail::idval2type(id));

        assume(resource<Id>);
        return *resource<Id>;
    }

    template <typename IdVal, typename F>
    static Rsc& get(IdVal id, F callback)
    {
        using Id = decltype(detail::idval2type(id));
        if(LIKELY(resource<Id>))
            return *resource<Id>;
        return resource<Id>.emplace(std::move(callback)());
    }

    template <typename IdVal>
    static std::optional<Rsc>& get_optional(IdVal id)
    {
        using Id = decltype(detail::idval2type(id));
        return resource<Id>;
    }


    template <typename IdVal>
    static Rsc& reset(IdVal id)
    {
        using Id = decltype(detail::idval2type(id));
        resource<Id>.reset();
    }

};

template <class Rsc>
template <typename>
std::optional<Rsc> static_resource<Rsc>::resource;


#endif //ZENGINE_STATIC_RESOURCE_HPP
