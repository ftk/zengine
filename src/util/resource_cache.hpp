//
// Created by fotyev on 2018-07-08.
//

#ifndef ZENGINE_RESOURCE_CACHE_HPP
#define ZENGINE_RESOURCE_CACHE_HPP

#include <utility>
#include "util/hash.hpp"
#include "util/lru_cache.hpp"
#include "util/resource_traits.hpp"

template <class Resource>
class resource_cache
{
    using key_type = uint64_t; // hash
    using size_type = decltype(resource_traits<Resource>::get_size(std::declval<Resource&>()));

    lru_cache_t<key_type, Resource> cache;

    size_type capacity, size = 0;

public:
    resource_cache(size_type capacity) : capacity(std::move(capacity)) {}


    /** Insert without checking.
     */
    Resource& insert(const key_type& key, Resource rsc)
    {
        clean();
        Resource& r = cache.insert(key, std::move(rsc))->second;
        size += resource_traits<Resource>::get_size(r);
        return r;
    }


    /** Create resource from identifier (e.g. const string) and put it into cache.
     */
     //template <typename = std::void_t<decltype(resource_traits<Resource>::from_id)>>

    Resource& get(string_view id)
    {
        //static_assert(fnv1a::hash(filename) != 0);
        return get(std::move(id), &resource_traits<Resource>::from_id);
    }

    /**
     * Lazy get, callback() will be called if no resource present
     */
    template <typename Callable>
    Resource& get(const key_type& key, const Callable& callback)
    {
        if(cache.count(key))
            return cache.get(key);
        // callback will create new Resource
        return insert(key, callback());
    }
    /**
     * Lazy get, callback(id) will be called if no resource present
     */
    template <typename Callable>
    Resource& get(string_view id, const Callable& callback)
    {
        auto key = fnv1a::hash(id);
        if(cache.count(key))
            return cache.get(key);
        // callback will create new Resource
        return insert(key, callback(std::move(id)));
    }


    bool remove(string_view id) { return remove(fnv1a::hash(id)); }

    bool remove(const key_type& key)
    {
        auto it = cache.find(key);
        if(it == cache.end())
            return false;
        cache.erase(std::move(it));
        return true;
    }

    /** Remove least recently used elements if size > capacity.
     */
    void clean()
    {
        while(size > capacity && !cache.empty())
        {
            size -=  resource_traits<Resource>::get_size(cache.lru().second);
            cache.lru_remove();
        }
    }

    /** Clear cache.
     */
    void clear()
    {
        cache.clear();
        size = size_type{};
    }

};
#endif //ZENGINE_RESOURCE_CACHE_HPP
