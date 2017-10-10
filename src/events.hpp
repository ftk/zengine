//
// Created by fotyev on 2017-02-02.
//

#ifndef ZENGINE_EVENTS_HPP
#define ZENGINE_EVENTS_HPP

#include <cstdint>

#include "util/serialization.hpp"

#include <boost/variant/variant.hpp>
#include <cereal/types/boost_variant.hpp>

#include "util/hash.hpp"

#include <vector>
#include <cereal/types/vector.hpp>

#include <string>
#include <cereal/types/string.hpp>


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
struct click {static constexpr unsigned index = 0; float x; float y; SERIALIZABLE((x)(y))};
struct join {static constexpr unsigned index = 1; SERIALIZABLE()};
struct joined {static constexpr unsigned index = 2; tick_t tick; net_node_id id; SERIALIZABLE((tick)(id))};
struct movement {static constexpr unsigned index = 3; int32_t x; int32_t y; uint8_t down; SERIALIZABLE((x)(y)(down))};
struct node_connect {static constexpr unsigned index = 4; SERIALIZABLE()};
struct node_disconnect {static constexpr unsigned index = 5; SERIALIZABLE()};
struct null {static constexpr unsigned index = 6; SERIALIZABLE()};
struct peers {static constexpr unsigned index = 7; std::vector<net_node_id> arr; SERIALIZABLE((arr))};
struct player_join {static constexpr unsigned index = 8; SERIALIZABLE()};
struct player_leave {static constexpr unsigned index = 9; SERIALIZABLE()};
struct statesync {static constexpr unsigned index = 10; std::string state; SERIALIZABLE((state))};

#define EVENTS_SEQ (click)(join)(joined)(movement)(node_connect)(node_disconnect)(null)(peers)(player_join)(player_leave)(statesync)/*>*/

//static_assert(std::is_pod<null>::value == true);

} // namespace event

using event_t = boost::variant<
    /*< join ', ', map { "event::$_->{name}" } dispatch('events');
     %*/event::click, event::join, event::joined, event::movement, event::node_connect, event::node_disconnect, event::null, event::peers, event::player_join, event::player_leave, event::statesync/*>*/
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
