//
// Created by fotyev on 2017-02-03.
//

#include "events.hpp"
#include "playerinputs.hpp"

#ifdef DEBUG_EVENTS
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#define ARCHIVE text
#else
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>
#define ARCHIVE binary
#endif

//#include <boost/serialization/export.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/array.hpp>

#include <boost/preprocessor/cat.hpp>

#include <sstream>

std::string serialize(const tick_input_t& event)
{
    std::ostringstream ss;
    using namespace boost::archive;
    BOOST_PP_CAT(ARCHIVE, _oarchive) oa(ss, no_header | no_codecvt | no_tracking);
    oa & event;
    return ss.str();
}



class stream_view : public std::streambuf {
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

tick_input_t deserialize(string_view data)
{
    tick_input_t event;
    stream_view sv(data);
    std::istream s(&sv);
    using namespace boost::archive;

    BOOST_PP_CAT(ARCHIVE, _iarchive) ia(s, no_header | no_codecvt | no_tracking);
    ia & event;
    return event;
}

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
static void dump(std::ostream& ss, const event::join & e) {ss << "join:";}
static void dump(std::ostream& ss, const event::joined & e) {ss << "joined:" << " tick: " << e. tick << " id: " << e. id;}
static void dump(std::ostream& ss, const event::movement & e) {ss << "movement:" << " x: " << e. x << " y: " << e. y;}
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
