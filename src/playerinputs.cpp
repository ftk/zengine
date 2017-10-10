//
// Created by fotyev on 2017-02-03.
//

#include "events.hpp"
#include "playerinputs.hpp"


template<typename T>
inline std::ostream& operator << (std::ostream& o, const std::vector<T>& v)
{
    o << '(';
    for(const auto& obj : v)
        o << obj << ',';
    o << ')';
    return o;
}

/*<

   my $s = '';
   for my $event (dispatch('events')) {
   $s .= "\nstatic void dump(std::ostream& ss, const event::$event->{name} & e) {";
   $s .= qq[ss << "$event->{name}:"];
   $s .= qq[ << " $_->[1]: " << e. $_->[1]] for (@{$event->{params}});
   $s .= ";}";
   }
   $s;
   %*/
static void dump(std::ostream& ss, const event::click & e) {ss << "click:" << " x: " << e. x << " y: " << e. y;}
static void dump(std::ostream& ss, const event::join & e) {ss << "join:";}
static void dump(std::ostream& ss, const event::joined & e) {ss << "joined:" << " tick: " << e. tick << " id: " << e. id;}
static void dump(std::ostream& ss, const event::movement & e) {ss << "movement:" << " x: " << e. x << " y: " << e. y << " down: " << e. down;}
static void dump(std::ostream& ss, const event::node_connect & e) {ss << "node_connect:";}
static void dump(std::ostream& ss, const event::node_disconnect & e) {ss << "node_disconnect:";}
static void dump(std::ostream& ss, const event::null & e) {ss << "null:";}
static void dump(std::ostream& ss, const event::peers & e) {ss << "peers:" << " arr: " << e. arr;}
static void dump(std::ostream& ss, const event::player_join & e) {ss << "player_join:";}
static void dump(std::ostream& ss, const event::player_leave & e) {ss << "player_leave:";}
static void dump(std::ostream& ss, const event::statesync & e) {ss << "statesync:" << " state: " << e. state;}/*>*/



std::string dump_event(const event_t& event)
{
    std::ostringstream ss;
    boost::apply_visitor([&ss](auto e) { dump(ss, e); }, event);
    return ss.str();
}
