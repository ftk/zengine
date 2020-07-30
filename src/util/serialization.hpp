//
// Created by fotyev on 2017-07-26.
//

#ifndef ZENGINE_SERIALIZATION_HPP
#define ZENGINE_SERIALIZATION_HPP

#include <boost/vmd/is_empty.hpp>
#include <boost/preprocessor/control/iif.hpp>

// SERIALIZABLE(field1, field2, field3)
#define SERIALIZABLE(...) \
template<class Archive_> void serialize(Archive_& ar_) { BOOST_PP_IIF(BOOST_VMD_IS_EMPTY(__VA_ARGS__),,SERIALIZABLE_IMPL(__VA_ARGS__)) }


#ifndef SERIALIZE_NVP
#define SERIALIZABLE_IMPL(...) (void)ar_(__VA_ARGS__);

#else // SERIALIAZE_NVP

#include <boost/preprocessor/tuple/to_seq.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#define SERIALIZABLE_IMPL(...) SERIALIZABLE_IMPL_SEQ(BOOST_PP_TUPLE_TO_SEQ((__VA_ARGS__)))

// SERIALIZABLE_IMPL_SEQ((field1) (field2))
// (void)(ar & CEREAL_NVP(field1) & CEREAL_NVP(field2));
#define SERIALIZABLE_IMPL_SEQ(seq) (void)(ar_ BOOST_PP_SEQ_FOR_EACH(SERIALIZABLE_HELPER, ar_, seq) );

#define SERIALIZABLE_HELPER(r, ar, elem) & CEREAL_NVP(elem)
#endif // SERIALIAZE_NVP

#include <boost/preprocessor/tuple/rem.hpp>
// TODO: make empty pp tuples work
// serialization for class containing entt ECS registry
// example: struct component1 {} ; ...
// struct { entt::registry<> reg; int field1, ...; ENTT_SERIALIZABLE(reg, (component1, component2), (field1, field2)) };
#define ENTT_SERIALIZABLE(registry,components,other) \
template<class Archive_> \
void save(Archive_& ar_) const \
{ \
    entt::snapshot{registry}.entities(ar_).template component<BOOST_PP_REM components>(ar_); \
    BOOST_PP_IIF(BOOST_VMD_IS_EMPTY other,,SERIALIZABLE_IMPL other;) \
} \
template<class Archive_> \
void load(Archive_& ar_) \
{ \
    entt::snapshot_loader{registry }.entities(ar_).template component<BOOST_PP_REM components>(ar_); \
    BOOST_PP_IIF(BOOST_VMD_IS_EMPTY other,,SERIALIZABLE_IMPL other;) \
} \
/* */

#define ENTT_COPY_SERIALIZABLE(classname,registry,components,other) \
ENTT_COPYABLE(classname,registry,components,other) \
ENTT_SERIALIZABLE(registry,components,other) \
DEFAULT_MOVABLE(classname)


#define CEREAL_SIZE_TYPE uint32_t


#include <sstream>
#include <string_view>

using std::string_view;

template <typename Archive, typename... Args>
inline std::string cserialize(Args&&... args)
{
    std::ostringstream ss;
    Archive ar(ss);
    ar(std::forward<Args>(args)...);
    return ss.str();
}



class stream_view : public std::streambuf
{
public:
    stream_view(char *data, size_t size)
    {
        this->setg(data, data, data + size);
    }
    stream_view(string_view data)
    {
        this->setg(const_cast<char *>(data.data()),
                   const_cast<char *>(data.data()),
                   const_cast<char *>(data.data() + data.size()));
    }
};

template <class Archive, typename... Args>
void deserialize(string_view data, Args&... args)
{
    stream_view sv(data);
    std::istream s(&sv);

    Archive ar(s);
    ar(args...);
}


// simple pseudo random number generator with serialization support

template<typename UIntType, UIntType a, UIntType c, UIntType m>
class linear_congruential_engine
{
public:
    // types
    typedef UIntType result_type;

    // engine characteristics
    static constexpr result_type multiplier = a;
    static constexpr result_type increment = c;
    static constexpr result_type modulus = m;
    static constexpr result_type min() { return c == 0u ? 1u : 0u; }
    static constexpr result_type max() { return m - 1; }
    static constexpr result_type default_seed = 1u;

    result_type x;

    SERIALIZABLE(x)
      // constructors and seeding functions

    explicit linear_congruential_engine(result_type s = default_seed) : x(s) {}
    void seed(result_type s = default_seed) { x = s; }

    // generating functions

    result_type operator()() { return (x = (a * x + c) % m); }

    void discard(unsigned long long z) { while(z--) { (*this)(); } }
}; // end linear_congruential_engine

typedef linear_congruential_engine<std::uint_fast32_t, 16807, 0, 2147483647> minstd_rand0;
typedef linear_congruential_engine<std::uint_fast32_t, 48271, 0, 2147483647> minstd_rand;


#endif //ZENGINE_SERIALIZATION_HPP
