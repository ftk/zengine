//
// Created by fotyev on 2017-02-02.
//

#ifndef ZENGINE_EVENTS_HPP
#define ZENGINE_EVENTS_HPP

#include <cstdint>

#include "util/serialization.hpp"

#include <variant>
#include <cereal/types/variant.hpp>

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
   #register_event(name=>'null');

   my $s = '';
   my $seq = '';
   my $index = 0;
   for my $event (dispatch('events')) {
   $seq .= '(' . $event->{name} . ')';
   $s .= "\nstruct $event->{name} {";
   $s .= "static constexpr unsigned index = $index; ";
   $index++;
   $s .= "$_->[0] $_->[1]; " for (@{$event->{params}});
   $s .= 'SERIALIZABLE(' . join(',', map { $_->[1] } @{$event->{params}}) . ')';


   #$s .= 'bool check() const {return true';
   #$s .= join " ", map { "&&" . ((scalar @$_ >= 4) ? "$_->[1] >= $_->[3] && $_->[1] <= $_->[4]" : "true") } @{$event->{params}};
   #$s .= ';}';
   $s .= '};';

   #$s .= 'template <class Archive> void serialize(Archive& ar, const unsigned int) { ';
   #$s .= "ar & $_->[1];" for (@{$event->{params}});
   #$s .= "};";
   }
   $s .= "\n\nconstexpr unsigned events_num = $index;\n#define EVENTS_SEQ $seq";
   $s;
   %*/
struct host_adv {static constexpr unsigned index = 0; SERIALIZABLE()};
struct join {static constexpr unsigned index = 1; SERIALIZABLE()};
struct joined {static constexpr unsigned index = 2; tick_t tick; net_node_id id; SERIALIZABLE(tick,id)};
struct make_new_snake {static constexpr unsigned index = 3; SERIALIZABLE()};
struct new_food {static constexpr unsigned index = 4; float x; float y; SERIALIZABLE(x,y)};
struct node_connect {static constexpr unsigned index = 5; SERIALIZABLE()};
struct node_disconnect {static constexpr unsigned index = 6; SERIALIZABLE()};
struct peers {static constexpr unsigned index = 7; std::vector<net_node_id> arr; SERIALIZABLE(arr)};
struct player_join {static constexpr unsigned index = 8; SERIALIZABLE()};
struct player_leave {static constexpr unsigned index = 9; SERIALIZABLE()};
struct statesync {static constexpr unsigned index = 10; std::string state; SERIALIZABLE(state)};

constexpr unsigned events_num = 11;
#define EVENTS_SEQ (host_adv)(join)(joined)(make_new_snake)(new_food)(node_connect)(node_disconnect)(peers)(player_join)(player_leave)(statesync)/*>*/

} // namespace event

using event_t = std::variant<
    /*< join ', ', map { "event::$_->{name}" } dispatch('events');
     %*/event::host_adv, event::join, event::joined, event::make_new_snake, event::new_food, event::node_connect, event::node_disconnect, event::peers, event::player_join, event::player_leave, event::statesync/*>*/
>;

// IF_EVENT(input.event, my_event, ev) { int x = ev->param; ... }
#define IF_EVENT(variant, type, name) if(const event:: type * const name = std::get_if<event:: type>(&variant))
// IF_EVENT_(input.event, player_join) { ... }
#define IF_EVENT_(variant, type) if(std::get_if<event:: type>(&variant))

#endif //ZENGINE_EVENTS_HPP
