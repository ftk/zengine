//
// Created by fotyev on 2017-02-03.
//

#ifndef ZENGINE_NETGAME_HPP
#define ZENGINE_NETGAME_HPP


#include "playerinputs.hpp"

#include <vector>



struct netgame_i
{
    netgame_i() = default;
    virtual ~netgame_i() = default;

    virtual net_node_id id() const { return 1; } // return local player id

    // send input to everybody connected
    virtual void broadcast_input(const tick_input_t& event) {}

    virtual void send_event(net_node_id id, event_t event) {};

    // handler should be thread safe
    virtual void set_event_handler(std::function<void(tick_input_t)> handler) {}

    virtual std::vector<net_node_id> nodes_list() const { return std::vector<net_node_id> {};}
};


#endif //ZENGINE_NETGAME_HPP
