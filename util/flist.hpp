//
// Created by fotyev on 2017-02-14.
//

#ifndef ZENGINE_LIST_HPP
#define ZENGINE_LIST_HPP

#include <forward_list>

#include "util/movable.hpp"
#include "util/assert.hpp"

/*
 * A very fast single linked list with strange usage
 *
 * Usage:
 *
 * auto l1 = flist<int>{1,2,3,4,5};
 * auto l2 = 1 >>= 2 >>= 3 >>= 4 >>= 5 >>= flist<int>{};
 * auto one = l1.pop(); // first = 1, l1 = {2,3,4,5}
 * auto l3 = +l1; // O(n), l3 now has a copy of l1
 * l1 = -l1 + -l2; // O(1), l1 now contains {2,3,4,5,1,2,3,4,5}
 *
 *
    auto l4 = (+l1).map([](int v)->float{return v / 2.f;});
    l4.for_each([](auto& b) { std::cout << b << ' ';});
    auto l5 = (-l1).map([](int v)->int{return v * 2;});
    l5.for_each([](auto& b) { std::cout << b << ' ';});

 *
 * T : movable
 */
template <typename T, typename Allocator = std::allocator<T> >
class flist : public std::forward_list<T, Allocator>
{
    NONCOPYABLE(flist)
    typedef std::forward_list<T, Allocator> base_class;
public:

    flist() = default;
    explicit flist(const Allocator& alloc) : base_class(alloc) {};
    flist(std::initializer_list<T> init,
         const Allocator& alloc = Allocator()) : base_class(std::move(init), alloc) {};


    //~flist() = default;

    flist(flist&&) = default;
    flist& operator=(flist&&) = default;

    // append to beginning
    // >>= was selected because it is right associative
    // flist<int> newlist = 1 >>= 2 >>= 3 >>= flist<int>{};
    friend inline flist operator >>= (T value, flist l)
    {
        l.push_front(std::move(value));
        return l;
    }

    // concatenation (a strange one: [1,2] += [3,4] -> [3,4,1,2])
    /*flist& operator += (flist rhs)
    {
        this->splice_after(this->before_begin(), std::move(rhs));
        return *this;
    }*/

    // concatenation
    friend inline flist operator + (flist lhs, flist rhs)
    {
        rhs.splice_after(rhs.before_begin(), std::move(lhs));
        return rhs;
    }

    // move
    // usage: -l is equivalent to std::move(l)
    flist&& operator - ()
    {
        return static_cast<flist&&>(*this); // std::move
    }

    // copy O(n)
    // usage: auto newlist = +l; // copies
    flist operator + ()
    {
        flist newlist;
        newlist.assign(this->begin(), this->end());
        return newlist;
    }
    //
    T pop()
    {
        assert(!this->empty());
        T first = std::move(this->front());
        this->pop_front();
        return first;
    }

    // map, rvalue version
    template <typename Callable>
    auto map(Callable f) && -> std::enable_if_t<!std::is_same<decltype(f(T{})), T>::value, flist<decltype(f(T{}))> >
    {
        if(!this->empty())
        {
            auto v = f(pop());
            return std::move(v) >>= std::move(*this).map(f);
        }
        else
            return flist<decltype(f(T{}))>{};
    }

    // map, transform version
    template <typename Callable>
    auto map(Callable f) && -> std::enable_if_t<std::is_same<decltype(f(T{})), T>::value, flist>
    {
        for(auto it = this->begin(); it != this->end(); ++it)
        {
            *it = f(std::move(*it));
        }
        return std::move(*this);
    }


    // mutable map  : list.for_each([](T& v){v+=2;});
    template <typename Callable>
    void for_each(Callable f)
    {
        for(auto it = this->begin(); it != this->end(); ++it)
        {
            f(*it);
        }
    }
};

#endif //ZENGINE_LIST_HPP
