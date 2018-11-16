//
// Created by fotyev on 2017-02-03.
//

#ifndef ZENGINE_NETGAME_HPP
#define ZENGINE_NETGAME_HPP


#include "playerinputs.hpp"

#include <vector>
#include "util/signals.hpp"


class netgame_i
{
public:
    netgame_i() = default;
    virtual ~netgame_i() = default;

    virtual net_node_id id() const { return 1; } // return local player id

    virtual void send_input(net_node_id id, const tick_input_t& input) = 0;
    virtual void send_input(const std::vector<net_node_id>& ids, const tick_input_t& input) = 0;

    virtual std::vector<net_node_id> nodes_list() const { return std::vector<net_node_id> {};}

public:
    sig::signal<void (tick_input_t)> on_event;
};


#endif //ZENGINE_NETGAME_HPP
