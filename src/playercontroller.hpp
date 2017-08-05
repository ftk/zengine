//
// Created by fotyev on 2017-03-12.
//

#ifndef ZENGINE_PLAYERCONTROLLER_HPP
#define ZENGINE_PLAYERCONTROLLER_HPP

#include "components/netgame.hpp"

#include "util/optional.hpp"
#include "util/log.hpp"
#include "util/assert.hpp"
#include "util/serialization.hpp"

#include <SDL_timer.h>


template <unsigned TPS> // ticks per second
class tick_timer
{
private:
    uint32_t timerstart;

public:
    tick_timer()
    {
        init();
    }

    void init(tick_t curtick = 0, unsigned rttfix = 0)
    {
        timerstart = SDL_GetTicks() - curtick * 1000 / TPS - rttfix / 2;
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


    std::vector<net_node_id> clients;

    // connecting client - only one at a time
    tick_t starttick = 0;
    net_node_id connecting_id = 0;
    enum {NONE,CONNECTING,WAITING_FOR_STATE} connecting_state = NONE;

public:
    tick_timer<50> timer;

    GamestateSim sim;
public:

    enum state_t
    {
        HOST,
        CLIENT,
        SYNC, // client is waiting for state
        STOPPED // game is stopped

    } state = STOPPED;

public:

    multiplayer_game(netgame_i * netgame) : netgame(netgame)
    {
        // todo move to adapter
        netgame->set_event_handler([this](tick_input_t input) {
            boost::apply_visitor([this, &input](const auto& event) -> void { this->on_netevent(input, event);}, input.event);
        });
    }



private:

#define CHECK_STATE(st) if(state != st) { LOGGER(warn, "state", state, "expected", st, __PRETTY_FUNCTION__); return; }

    //=- register_event(name=>'join', params=>[]);
    void on_netevent(const tick_input_t& input, event::join)
    {
        CHECK_STATE(HOST);
        //=- register_event(name=>'peers', params=>[['std::vector<net_node_id>','arr']]);
        LOGGER(info, "sent peers to", input.player);
        send(input.player, event::peers{clients});
    }

    void on_netevent(const tick_input_t& input, const event::peers& peers)
    {
        if(state == HOST)
        {
            if(connecting_state != NONE)
            {
                LOGGER(info, "rejected", input.player, "as", connecting_id, "is already connecting");
                return;
            }
            // todo: check if clients changed?
            if(peers.arr == clients)
            {
                // add promise to send him state of (now+lag)
                starttick = timer.get_tick() + sim.lag;
                connecting_id = input.player;
                // notify other clients ?
                //=- register_event(name=>'joined', params=>[['tick_t', 'tick'],['net_node_id','id']]);

                broadcast(event::joined{starttick, input.player});
                //send_state(remote);
                connecting_state = CONNECTING;
            }
        }
        else if(state == SYNC)
        {
            if(connecting_state == CONNECTING)
            {
                connecting_state = WAITING_FOR_STATE;
                starttick = SDL_GetTicks() - starttick; // now it holds RTT
            } else LOGGER(warn, connecting_state, __func__);
            // todo: check if connected
            clients = peers.arr;
            send(input.player, peers);
        }
        else LOGGER(warn, __func__);
    }

    void broadcast(event_t event)
    {
        netgame->send_input(clients, tick_input_t{netgame->id(), timer.get_tick(), std::move(event)});
    }

    void send(net_node_id remote, event_t event)
    {
        netgame->send_input(remote, tick_input_t{netgame->id(), timer.get_tick(), std::move(event)});
    }

    void push_input(tick_input_t input)
    {
        netgame->send_input(clients, input);
        sim.on_input(std::move(input));
    }
public:
    void push_event(event_t event)
    {
        if(state == HOST || state == CLIENT)
            push_input(tick_input_t{netgame->id(), timer.get_tick(), std::move(event)});
    }

    void update()
    {
        if(state != HOST && state != CLIENT)
            return;

        sim.update(timer.get_tick());

        // handle connecting client
        if(connecting_state != NONE)
        {
            if(timer.get_tick() > starttick)
            {
                if(connecting_state == CONNECTING)
                {
                    //=- register_event(name=>'player_join', params=>[]);
                    LOGGER(info, "client connected", connecting_id);
                    sim.on_input(tick_input_t{connecting_id, starttick, event::player_join{}});
                    clients.push_back(connecting_id);
                    if(state == HOST)
                        connecting_state = WAITING_FOR_STATE;
                    else
                        connecting_state = NONE;
                }
                if(connecting_state == WAITING_FOR_STATE)
                {
                    if(sim.get_oldtick() > starttick)
                    {
                        LOGGER(info, "sending state to", connecting_id);
                        // send time & state ...
                        //=- register_event(name=>'statesync', params=>[['std::string', 'state']]);
                        send(connecting_id, event::statesync{cserialize(sim)});
                        connecting_state = NONE;
                    }
                }

            }
        }
    }

    // state transitions

    void stop()
    {
        sim.clear();
        clients.clear();
        state = STOPPED;
        connecting_state = NONE;
    }
    void host()
    {
        CHECK_STATE(STOPPED);
        assume(clients.empty());
        clients.push_back(netgame->id()); // clients[0] is always host
        sim.on_input(tick_input_t{netgame->id(), 0, event::player_join{}});
        timer.init(0);
        state = HOST;
    }

    void join(net_node_id remote)
    {
        CHECK_STATE(STOPPED);

        state = SYNC;
        connecting_state = CONNECTING;
        starttick = SDL_GetTicks(); // special meaning, in ms
        connecting_id = remote;

        // send join ev...
        send(remote, event::join{});
    }



private:


    void on_netevent(const tick_input_t& input, const event::statesync& ev)
    {
        CHECK_STATE(SYNC);
        if(connecting_state != WAITING_FOR_STATE) LOGGER(warn, __func__);

        // set_state todo
        LOGGER(info, "setting state", timer.get_tick(), input.tick);
        deserialize(ev.state, sim);
        timer.init(input.tick, starttick); // + rtt/2
        LOGGER(info, "new oldtick", sim.get_oldtick(), timer.get_tick());

        state = CLIENT;
        connecting_state = NONE;
    }
    //
    void on_netevent(const tick_input_t& input, const event::joined& ev)
    {
        CHECK_STATE(CLIENT);

        if(input.player != clients[0])
            LOGGER(warn, "fake joined msg from", input.player);

        connecting_state = CONNECTING;
        connecting_id = ev.id;
        starttick = ev.tick;
    }

    void on_netevent(const tick_input_t& input, const event::node_connect&)
    {
    }

    void on_netevent(const tick_input_t& input, const event::node_disconnect&)
    {
        auto rm_player = [this](net_node_id player) -> bool {
            auto it = std::find(clients.begin(), clients.end(), player);
            if(it == clients.end()) return false;
            *it = clients.back();
            clients.pop_back();
            return true;
        };
        switch(state)
        {
            case CLIENT:
                if(input.player == clients[0]) // host has disconnected :(
                {
                    // todo: host migration
                    stop();
                }
                rm_player(input.player);
                break; // wait about disconnect msg from host
            case HOST:
                if(rm_player(input.player))
                {
                    //=- register_event(name=>'player_leave');
                    push_input(tick_input_t{input.player, timer.get_tick(), event::player_leave{}});
                }
                break;
            case SYNC:
                if(input.player == connecting_id) stop();
                // todo: check if connected players DC'ed
                rm_player(input.player);
                break;
            case STOPPED:
                break;
        }
    }

    template <typename T>
    void on_netevent(const tick_input_t& input, const T&)
    {
        if(state != STOPPED)
            sim.on_input(input);
    }

};

#undef CHECK_STATE

#endif //ZENGINE_PLAYERCONTROLLER_HPP
