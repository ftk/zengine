#ifndef CONSTSTRID_HPP
#define CONSTSTRID_HPP

#include <type_traits>
#include <utility>

// https://github.com/hogliux/semimap/blob/master/semimap.h

namespace detail
{
    // evaluates to the type returned by a constexpr lambda
    template <typename Identifier>
    using identifier_type = decltype(std::declval<Identifier>()());

    //==============================================================================
    constexpr std::size_t constexpr_strlen(const char * str)
    { return str[0] == 0 ? 0 : constexpr_strlen(str + 1) + 1; }

    //==============================================================================
    template <auto...>
    struct dummy_t
    {
    };

    template <typename Identifier, std::enable_if_t<std::is_integral_v<identifier_type<Identifier>>, int> = 0>
    constexpr auto idval2type(Identifier id)
    {
        return dummy_t<id()>{};
    }

    template <typename Identifier, std::size_t... I>
    constexpr auto array_id2type(Identifier id, std::index_sequence<I...>)
    {
        return dummy_t<id()[I]...>{};
    }

    template <typename Identifier, std::enable_if_t<std::is_same_v<identifier_type<Identifier>, const char *>, int> = 0>
    constexpr auto idval2type(Identifier id)
    {
        return array_id2type(id, std::make_index_sequence < constexpr_strlen(id()) > {});
    }
}

#define ID(x) []() constexpr { return x; }

/* String literal to type
 * example:

auto f1()
{
    return detail::idval2type(ID("test"));
}
auto f2()
{
    return detail::idval2type(ID("test"));
}
auto f3()
{
    return ID("test");
}
auto f4()
{
    return ID("test");
}


static_assert(std::is_same_v<decltype(f1()), decltype(f2())>);
static_assert(!std::is_same_v<decltype(f3()), decltype(f4())>);

 */
#endif //CONSTSTRID_HPP
