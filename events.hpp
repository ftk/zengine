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
struct connect_ack {static constexpr unsigned index = 0; uint32_t tick; SERIALIZABLE((tick))};
struct connect_req {static constexpr unsigned index = 1; SERIALIZABLE()};
struct game_start {static constexpr unsigned index = 2; SERIALIZABLE()};
struct game_start_ack {static constexpr unsigned index = 3; SERIALIZABLE()};
struct movement {static constexpr unsigned index = 4; int32_t x; int32_t y; SERIALIZABLE((x)(y))};
struct null {static constexpr unsigned index = 5; SERIALIZABLE()};

#define EVENTS_SEQ (connect_ack)(connect_req)(game_start)(game_start_ack)(movement)(null)/*>*/

//static_assert(std::is_pod<null>::value == true);

} // namespace event

using event_t = boost::variant<
    /*< join ', ', map { "event::$_->{name}" } dispatch('events');
     %*/event::connect_ack, event::connect_req, event::game_start, event::game_start_ack, event::movement, event::null/*>*/
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


#include <SDL_timer.h>

typedef uint32_t tick_t;

constexpr unsigned TICKS_PER_SECOND = 50;

inline tick_t get_tick() { return SDL_GetTicks() / (1000 / TICKS_PER_SECOND); }


using net_node_id = uint64_t;





#endif //ZENGINE_EVENTS_HPP
