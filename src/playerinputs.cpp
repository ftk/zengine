#define SERIALIZE_NVP

#include "events.hpp"
#include "playerinputs.hpp"
#include <cereal/archives/json.hpp>


std::string dump_event(const event_t& event)
{
    return cserialize<cereal::JSONOutputArchive>(event);
}
