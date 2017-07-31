//
// Created by fotyev on 2017-03-12.
//

#ifndef ZENGINE_PLAYERCONTROLLER_HPP
#define ZENGINE_PLAYERCONTROLLER_HPP

#include "components/netgame.hpp"

#include "util/optional.hpp"
#include "util/log.hpp"
#include "util/assert.hpp"

#include <SDL_timer.h>



template <unsigned TPS> // ticks per second
class tick_timer
{
private:
    uint32_t timerstart;

public:
    tick_timer(tick_t curtick = 0)
    {
        init(curtick);
    }

    void init(tick_t curtick = 0)
    {
        timerstart = SDL_GetTicks() - curtick * 1000 / TPS;
    }

    tick_t get_tick() const
    {
        return (SDL_GetTicks() - timerstart) * TPS / 1000;
    }
};

// GamestateSim reqs : update(tick), push(input), get_state, set_state
template <class GamestateSim>
class multiplayer_game
{
    netgame_i * netgame;

    bool host = true;
    bool connecting = false;
    std::vector<net_node_id> clients;

    std::list<std::pair<tick_t, net_node_id>> promised_clients; // [(starttick, id)]

    //optional<Controller> gamecontroller;



public:
    tick_timer<50> timer;

    GamestateSim sim;
public:

    multiplayer_game(netgame_i * netgame) : netgame(netgame), host(true)
    {
        // todo move to adapter
        netgame->set_event_handler([this](tick_input_t input) {
            boost::apply_visitor([this, &input](const auto& event) -> void { this->on_netevent(input, event);}, input.event);
        });
        if(host)
            clients.push_back(netgame->id()); // clients[0] is always host
        sim.push(tick_input_t{netgame->id(), 0, event::player_join{}});
    }

private:

    //=- register_event(name=>'join', params=>[]);
    void on_netevent(const tick_input_t& input, event::join)
    {

        send_peers(input.player);
    }

    void send_peers(net_node_id remote)
    {
        // ...
        //=- register_event(name=>'peers', params=>[['std::vector<net_node_id>','arr']]);
        LOGGER(info, "sent peers to", remote);
        send(remote, event::peers{clients});

    }

    void on_netevent(const tick_input_t& input, const event::peers& peers)
    {
        if(host)
        {
            // todo: check if clients changed?
            if(peers.arr == clients)
                peers_ok(input.player);
        }
        else
        {
            // todo: check if connected
            clients = peers.arr;
            send(input.player, peers);
        }
    }

    void peers_ok(net_node_id remote)
    {
        // start

        // add promise to send him state after (lag) ms
        tick_t start = timer.get_tick() + sim.lag;
        promised_clients.push_back({start, remote});
        // notify other clients ?
        //=- register_event(name=>'joined', params=>[['tick_t', 'tick'],['net_node_id','id']]);

        broadcast(event::joined{start, remote});
        //send_state(remote);
    }


    void send_state(net_node_id remote)
    {
        // send time & state ...
        //=- register_event(name=>'statesync', params=>[['std::string', 'state']]);
        send(remote, event::statesync{sim.get_state()});
    }


    void broadcast(event_t event)
    {
        netgame->send_input(clients, tick_input_t{netgame->id(), timer.get_tick(), std::move(event)});
    }

    void send(net_node_id remote, event_t event)
    {
        netgame->send_input(remote, tick_input_t{netgame->id(), timer.get_tick(), std::move(event)});
    }


public:
    void push_input(tick_input_t input)
    {
        netgame->send_input(clients, input);
        sim.push(std::move(input));
    }
    void push_event(event_t event)
    {
        push_input(tick_input_t{netgame->id(), timer.get_tick(), std::move(event)});
    }

    void update()
    {
        if(connecting)
            return;
        while(!promised_clients.empty() && timer.get_tick() >= promised_clients.front().first)
        {
            const net_node_id client = promised_clients.front().second;
            clients.push_back(client);
            if(host)
            {
                send_state(client);
                // pass join event
                //=- register_event(name=>'player_join');
                auto inp = tick_input_t{client, promised_clients.front().first, event::player_join{}};
                push_input(std::move(inp));
            }
            promised_clients.pop_front();
        }
        sim.update(timer.get_tick());
    }

    // client

    void join(net_node_id remote)
    {
        /*if(host || !clients.empty())
        {
            LOGGER(warn, __func__);
            return;
        }*/
        host = false; // todo
        connecting = true;
        // send join ev...
        send(remote, event::join{});
    }

private:


    void on_netevent(const tick_input_t& input, const event::statesync& ev)
    {
        if(clients.empty() || host || !connecting)
        {
            LOGGER(warn, __func__);
            return;
        }
        // set_state todo
        LOGGER(info, "setting state", timer.get_tick(), input.tick);
        sim.set_state(ev.state);
        timer.init(input.tick); // + rtt/2
        LOGGER(info, "new oldtick", sim.get_oldtick(), timer.get_tick());
        connecting = false;
    }
    //
    void on_netevent(const tick_input_t& input, const event::joined& ev)
    {
        if(clients.empty() || host)
        {
            LOGGER(warn, __func__);
            return;
        }
        if(input.player != clients[0])
            LOGGER(warn, "fake joined msg from", input.player);
        promised_clients.push_back({ev.tick, ev.id});
    }

    template <typename T>
    void on_netevent(const tick_input_t& input, const T&)
    {
        sim.push(input);
    }

};
#endif //ZENGINE_PLAYERCONTROLLER_HPP
