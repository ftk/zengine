//
// Created by fotyev on 2016-10-15.
//

#ifndef ZENGINE_LRU_CACHE_HPP
#define ZENGINE_LRU_CACHE_HPP

#include <unordered_map>
#include <list>
#include "assert.hpp"

// TODO: allocator
template <typename Key, typename Value>
class lru_cache_t
{
public:
    typedef std::pair<Key, Value> value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef typename std::list<value_type>::iterator iterator;
    typedef typename std::list<value_type>::const_iterator const_iterator;
private:
    std::list<value_type> items;
    std::unordered_map<Key, iterator> dict;

public:

#if 0/*% # export methods
     my @funs = qw(begin begin_c end end_c cbegin_c cend_c
     rbegin rbegin_c rend rend_c crbegin_c crend_c
     size_c empty_c front front_c back back_c);
     "\t" . join "\n\t", map { my $c=''; $c = "const" if s/_c$//; "auto $_() $c -> decltype(items.$_()) { return items.$_(); }" } @funs;
     #*/
#else
	auto begin()  -> decltype(items.begin()) { return items.begin(); }
	auto begin() const -> decltype(items.begin()) { return items.begin(); }
	auto end()  -> decltype(items.end()) { return items.end(); }
	auto end() const -> decltype(items.end()) { return items.end(); }
	auto cbegin() const -> decltype(items.cbegin()) { return items.cbegin(); }
	auto cend() const -> decltype(items.cend()) { return items.cend(); }
	auto rbegin()  -> decltype(items.rbegin()) { return items.rbegin(); }
	auto rbegin() const -> decltype(items.rbegin()) { return items.rbegin(); }
	auto rend()  -> decltype(items.rend()) { return items.rend(); }
	auto rend() const -> decltype(items.rend()) { return items.rend(); }
	auto crbegin() const -> decltype(items.crbegin()) { return items.crbegin(); }
	auto crend() const -> decltype(items.crend()) { return items.crend(); }
	auto size() const -> decltype(items.size()) { return items.size(); }
	auto empty() const -> decltype(items.empty()) { return items.empty(); }
	auto front()  -> decltype(items.front()) { return items.front(); }
	auto front() const -> decltype(items.front()) { return items.front(); }
	auto back()  -> decltype(items.back()) { return items.back(); }
	auto back() const -> decltype(items.back()) { return items.back(); }
#endif
    //iterator begin() { return items.begin(); }
    //iterator end() { return items.end(); }

    //std::size_t size() const { return dict.size(); }
    //bool empty() const { return dict.empty(); }

    iterator find(const Key& k) // without updating lru state
    {
        auto it = dict.find(k);
        return (it == dict.end()) ? end() : it->second;
    }

    void erase(iterator it)
    {
        dict.erase(it->first);
        items.erase(it);
    }

    iterator insert(Key k, Value v)
    {
        auto it = dict.find(k);
        if(it != dict.end())
        {
            items.erase(it->second);
            dict.erase(it);
        }
        items.emplace_front(k, std::move(v));
        dict.emplace(std::move(k), items.begin());
        return begin();
    }

    std::size_t count(const Key& k) const
    {
        return dict.count(k);
    }

    Value& get(const Key& k) // "use"
    {
        assert(count(k));
        iterator pos = find(k);
        items.splice(items.begin(), items, pos); // move to front
        return pos->second;
    }

    void lru_remove() // removes least recently used element
    {
        assert(!empty());
        dict.erase(items.back().first);
        items.pop_back();
    }
    value_type& lru()
    {
        assert(!empty());
        return items.back();
    }

    void shrink(std::size_t newsize)
    {
        while(size() > newsize)
            lru_remove();
    }


    void clear()
    {
        dict.clear();
        items.clear();
    }

};

#endif //ZENGINE_LRU_CACHE_HPP
