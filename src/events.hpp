//
// Created by fotyev on 2017-02-02.
//

#ifndef ZENGINE_EVENTS_HPP
#define ZENGINE_EVENTS_HPP

#include <cstdint>

#include <boost/variant/variant.hpp>
#include <boost/variant.hpp>

//#include <type_traits>

#include "util/hash.hpp"

#include <vector>
#include <boost/serialization/vector.hpp>


#include <boost/preprocessor/seq/for_each.hpp>


// SERIALIZABLE((field1) (field2))
// template<class Archive> void serialize(Archive& ar, const unsigned) { ar&field1&field2; }
#define SERIALIZABLE(seq) \
template<class Archive> void serialize(Archive& ar, const unsigned) { (void)(ar \
BOOST_PP_SEQ_FOR_EACH(SERIALIZABLE_HELPER, ar, seq) \
); }

//#define DEBUG_EVENTS

#ifdef DEBUG_EVENTS
#include <boost/serialization/nvp.hpp>
#define SERIALIZABLE_HELPER(r, ar, elem) & BOOST_SERIALIZATION_NVP(elem)
#else
#define SERIALIZABLE_HELPER(r, ar, elem) & elem
#endif


using tick_t = uint32_t;
using net_node_id = uint64_t;

namespace event {


/*<
   sub register_event {
    collect('events', {@_});
   }
   register_event(name=>'null');

   my $s = '';
   my $seq = '';
   my $index = 0;
   for my $event (dispatch('events')) {
   $seq .= '(' . $event->{name} . ')';
   $s .= "\nstruct $event->{name} {";
   $s .= "static constexpr unsigned index = $index; ";
   $index++;
   $s .= "$_->[0] $_->[1]; " for (@{$event->{params}});
   $s .= 'SERIALIZABLE(';
   $s .= "($_->[1])" for (@{$event->{params}});
   $s .= ')};';

   #$s .= 'template <class Archive> void serialize(Archive& ar, const unsigned int) { ';
   #$s .= "ar & $_->[1];" for (@{$event->{params}});
   #$s .= "};";
   }
   $s .= "\n\n#define EVENTS_SEQ $seq";
   $s;
   %*/
struct join {static constexpr unsigned index = 0; SERIALIZABLE()};
struct joined {static constexpr unsigned index = 1; tick_t tick; net_node_id id; SERIALIZABLE((tick)(id))};
struct movement {static constexpr unsigned index = 2; int32_t x; int32_t y; SERIALIZABLE((x)(y))};
struct null {static constexpr unsigned index = 3; SERIALIZABLE()};
struct peers {static constexpr unsigned index = 4; std::vector<net_node_id> arr; SERIALIZABLE((arr))};
struct player_join {static constexpr unsigned index = 5; SERIALIZABLE()};
struct statesync {static constexpr unsigned index = 6; std::string state; SERIALIZABLE((state))};

#define EVENTS_SEQ (join)(joined)(movement)(null)(peers)(player_join)(statesync)/*>*/

//static_assert(std::is_pod<null>::value == true);

} // namespace event

using event_t = boost::variant<
    /*< join ', ', map { "event::$_->{name}" } dispatch('events');
     %*/event::join, event::joined, event::movement, event::null, event::peers, event::player_join, event::statesync/*>*/
>;
//static_assert(std::is_pod<event_t>::value == true);

#define EVENT_VISITOR_HELPER(r,f1,ev_t) case event::ev_t::index: return f1 (boost::get<event::ev_t>(ev));

#define EVENT_VISITOR(e,f1,f2,seq) \
[](const event_t& ev) { \
switch(ev.which()) { \
BOOST_PP_SEQ_FOR_EACH(EVENT_VISITOR_HELPER, f1, seq) \
default: return f2 (ev); \
}}(e);

#define EVENT_VISITOR_ALL(e,f) \
[](const event_t& ev, auto f1) { \
switch(ev.which()) { \
BOOST_PP_SEQ_FOR_EACH(EVENT_VISITOR_HELPER, f1, EVENTS_SEQ) \
}}(e,f);




#endif //ZENGINE_EVENTS_HPP
