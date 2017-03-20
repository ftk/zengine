//
// Created by fotyev on 2017-03-12.
//

#ifndef ZENGINE_PLAYERCONTROLLER_HPP
#define ZENGINE_PLAYERCONTROLLER_HPP

#include "components/netgame.hpp"

#include "util/optional.hpp"
#include "util/log.hpp"
#include "util/assert.hpp"


class stop_game {};

/* Controller requirements:
 * Controller(net_node_id local, net_node_id remote);
 * on_netevent(input_t, event_t);
 * //draw();
 *
 */

template <class Controller>
class two_player_game
{
protected:
    net_node_id local, remote = 0;

    netgame_i * netgame;

public:

    optional<Controller> gamecontroller;

public:
    two_player_game(netgame_i * netgame) : netgame(netgame)
    {
        local = netgame->id();
        assert(local);
        netgame->set_event_handler([this](const tick_input_t& input) {
            boost::apply_visitor([this, &input](const auto& event) -> void { this->on_netevent(input, event);}, input.event);
        });
    }

    void send_request(net_node_id remote)
    {
        if(gamecontroller)
            return;
        this->remote = remote;
        //=- register_event(name=>'game_start');
        LOGGER(info, "request sent to", remote);
        netgame->send_event(remote, event::game_start{});
    }

private:


    void on_netevent(const tick_input_t& input, event::game_start)
    {
        if(gamecontroller)
            return;
        //=- register_event(name=>'game_start_ack');

        if(1)
        {
            remote = input.player;
            LOGGER(info, "game start received from", remote);
            netgame->send_event(remote, event::game_start_ack{});
            //gamecontroller = make_optional<Controller>(local, remote);
            gamecontroller.emplace(local, remote);
        }

    }
    void on_netevent(const tick_input_t& input, event::game_start_ack)
    {
        if(input.player != remote)
            return;
        LOGGER(info, "game start acknowledged");
        gamecontroller.emplace(local, remote);
    }

    void on_netevent(const tick_input_t& input, event::disconnect)
    {
        if(input.player != remote)
            return;
        LOGGER(info, "player disconnected");
        // shut down
        gamecontroller = nullopt;
    }

    template<typename T>
    void on_netevent(const tick_input_t& input, const T& event)
    {
        if(gamecontroller)
        {
            gamecontroller->on_netevent(input, event);
        }
    }

};


#endif //ZENGINE_PLAYERCONTROLLER_HPP
