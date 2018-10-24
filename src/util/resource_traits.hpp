//
// Created by fotyev on 2018-10-24.
//

#ifndef ZENGINE_RESOURCE_TRAITS_HPP
#define ZENGINE_RESOURCE_TRAITS_HPP

#include <utility>

template <class Resource>
struct resource_traits
{
    static unsigned int get_size(const Resource& rsc) { return 1; }
    template <typename T>
    static T from_id(T id) { return std::forward<T>(id); }
};

#endif //ZENGINE_RESOURCE_TRAITS_HPP
