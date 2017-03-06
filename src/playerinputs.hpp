//
// Created by fotyev on 2016-10-18.
//

#ifndef ZENGINE_PLAYERINPUTS_HPP
#define ZENGINE_PLAYERINPUTS_HPP

#include <boost/circular_buffer.hpp>

#include <algorithm>

#include "events.hpp"

#include "util/optional.hpp"
#include "util/assert.hpp"





struct tick_input_t
{
    net_node_id player;
    tick_t tick;
    event_t event;


    bool operator <(const tick_input_t& rhs) const { return tick < rhs.tick; }

    bool operator <(const tick_t tick) const { return this->tick < tick; }

    // serialize ...
    SERIALIZABLE((player)(tick)(event))

};

constexpr int max_queued_inputs = 64;

struct inputs_t
{
    boost::circular_buffer<tick_input_t> buf{max_queued_inputs}; // sorted by tick asc


    void push_back(tick_input_t inp)
    {
        // sorted
        assert(buf.empty() || inp.tick >= buf.back().tick);
        buf.push_back(std::move(inp));
    }

    void push(tick_input_t inp)
    {
        auto it = std::lower_bound(buf.begin(), buf.end(), inp);
        /*
        if(it != buf.end() && it->tick == inp.tick) // merge
        {
            while(it->inputs.size() < it->inputs.static_capacity && !inp.empty())
            {
                it->inputs.push_back(inp.inputs.back());
                inp.inputs.pop_back();
            }
        } else
        {
            // O(n) insert
            // however pushes to the end will be more frequent
            buf.insert(it, std::move(inp));
        }*/
        if(it == buf.end())
            push_back(std::move(inp));
        else
            buf.insert(it, std::move(inp));
    }

    bool empty() const { return buf.empty(); }

    void pop_old(tick_t until)
    {
        while(!empty() && buf.front().tick <= until)
            buf.pop_front();
    }

    auto lower_bound(tick_t tick)
    {
        auto it = std::lower_bound(buf.begin(), buf.end(), tick);
        return it;
    }


};

std::string serialize(const tick_input_t& event);

tick_input_t deserialize(string_view data);

std::string dump_event(const event_t& event);


#endif //ZENGINE_PLAYERINPUTS_HPP
