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
#include "util/log.hpp"





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


struct inputs_t
{
    boost::circular_buffer<tick_input_t> buf{32}; // sorted by tick asc


    void push_back(tick_input_t inp)
    {
        check_capacity();
        // sorted
        assert(buf.empty() || inp.tick >= buf.back().tick);
        buf.push_back(std::move(inp));
    }

    void push(tick_input_t inp)
    {
        check_capacity();

        auto it = std::lower_bound(buf.begin(), buf.end(), inp);
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

private:
    void check_capacity()
    {
        if(buf.full())
        {
            const auto ncap = buf.capacity() * 2;
            LOGGER(warn, "Input buffer is overflowing, increasing to", ncap);
            // TODO: since this incalidates iterators, fix in case of multithreading
            buf.set_capacity(ncap);
        }
    }
};

std::string dump_event(const event_t& event);


#endif //ZENGINE_PLAYERINPUTS_HPP
