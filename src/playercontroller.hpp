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

#include <glfw/glfw3.h>

#ifndef TICKS_PER_SECOND
#define TICKS_PER_SECOND 60
#endif

template <unsigned TPS> // ticks per second
class tick_timer
{
private:
    double timerstart;

public:
    tick_timer()
    {
        init();
    }

    void init(tick_t curtick = 0, double rttfix = 0)
    {
        timerstart = glfwGetTime() - double(curtick) / TPS - rttfix / 2;
    }

    tick_t get_tick() const
    {
        return static_cast<tick_t>((glfwGetTime() - timerstart) * TPS);
    }
};

// GamestateSim reqs : update(tick), on_input(input), Serializable
template <class GamestateSim>
class multiplayer_game
{
    netgame_i * netgame;


    std::vector<net_node_id> clients; // list of peers

    // connecting client - only one at a time
    tick_t starttick = 0;
    net_node_id connecting_id = 0;
    enum {NONE,CONNECTING,WAITING_FOR_STATE} connecting_state = NONE;

    uint32_t rtt = 0; // round trip time to host, in ms

public:
    tick_timer<TICKS_PER_SECOND> timer;

    GamestateSim sim;
public:

    enum state_t
    {
        HOST,
        CLIENT,
        SYNC, // client is connecting
        STOPPED // game is stopped

    } state = STOPPED;
    /* multiplayer_game is a state machine
     * connecting process:
     *  1. the client sends event::join to host (state -> SYNC)
     *  2. host replies with a list of peers (event::peers)
     *  3. the client checks whether all peers are known to him and replies with event::peers
     *  4. host sends all connected clients that new player is connecting with his start time (starttick)
     *  5. clients save that information in starttick, connecting_id
     *  6. at starttick, clients and host add client to clients list (he will now receive inputs from them)
     *  7. when sim.oldtick == starttick, host sends state to the client
     *  8. the client receives state from host, initializes (state -> CLIENT)
     *
     * State:
     *  HOST - this game instance acts as a host
     *  - all new players should connect to it (by join())
     *  - it will handle disconnect of players
     *   connecting_state:
     *    NONE - normal operation
     *    CONNECTING - [connecting_id] is connecting to the game and he will join at tick [starttick]
     *    WAITING_FOR_STATE - tick [starttick] has passed and [connecting_id] has started receiving inputs, but not the state
     *    (he'll receive the state when sim.oldstate will be at [starttick])
     *  CLIENT - this game instance acts as a client to host [clients[0]]
     *   connecting_state:
     *    NONE - normal operation
     *    CONNECTING - [connecting_id] is connecting to the game and he will join at tick [starttick]
     *  SYNC - this game is stopped, but has started joining host [connecting_id]
     *   starttick: time when client has sent join msg, in ms
     *   connecting_state:
     *    CONNECTING - host has not responded to peers request yet
     *    WAITING_FOR_STATE - host has responded with peers and all peers are connected
     *  STOPPED - game is stopped
     **/

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
        //connect2
        CHECK_STATE(HOST);
        //=- register_event(name=>'peers', params=>[['std::vector<net_node_id>','arr']]);
        LOGGER(info, "sent peers to", input.player);
        send(input.player, event::peers{clients});
    }

    void on_netevent(const tick_input_t& input, const event::peers& peers)
    {
        if(state == HOST)
        {
            //connect4
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
            //connect3
            if(connecting_state == CONNECTING)
            {
                connecting_state = WAITING_FOR_STATE;
                rtt = glfwGetTime() / 1000 - starttick;
            } else LOGGER(warn, connecting_state, __func__);
            // check if connected
            const auto connected = netgame->nodes_list(); // assume list is sorted
            for(net_node_id id : peers.arr)
            {
                if(id != netgame->id() && !std::binary_search(connected.begin(), connected.end(), id))
                {
                    LOGGER(error, id, "is not connected, stopping");
                    stop();
                    return;
                }
            }
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
        LOGGER(debug4, "event push", input.player, "tick", input.tick, dump_event(input.event));
        netgame->send_input(clients, input);
        sim.on_input(std::move(input));
    }
public:
    void push_event(event_t event)
    {
        if(state == HOST || state == CLIENT)
            push_input(tick_input_t{netgame->id(), timer.get_tick(), std::move(event)});
    }

    bool playing() const { return state == HOST || state == CLIENT; }

    void update()
    {
        if(!playing())
            return;

        sim.update(timer.get_tick());

        // handle connecting client
        if(connecting_state != NONE)
        {
            if(timer.get_tick() > starttick)
            {
                if(connecting_state == CONNECTING)
                {
                    //connect6
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
                    //connect7
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
        rtt = 0;
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
        //connect1

        state = SYNC;
        connecting_state = CONNECTING;
        starttick = glfwGetTime() / 1000; // special meaning, join time in ms
        connecting_id = remote;

        // send join ev...
        send(remote, event::join{});
    }



private:


    void on_netevent(const tick_input_t& input, const event::statesync& ev)
    {
        CHECK_STATE(SYNC);
        if(connecting_state != WAITING_FOR_STATE) LOGGER(warn, __func__);

        // connect8
        LOGGER(info, "setting state", timer.get_tick(), input.tick);
        deserialize(ev.state, sim);
        timer.init(input.tick, rtt); // + rtt/2
        LOGGER(info, "new oldtick", sim.get_oldtick(), timer.get_tick());

        clients.push_back(netgame->id()); // add self to client list

        state = CLIENT;
        connecting_state = NONE;
    }
    //
    void on_netevent(const tick_input_t& input, const event::joined& ev)
    {
        CHECK_STATE(CLIENT);
        //connect5

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
        if(state != STOPPED) // ?
            sim.on_input(input);
    }

};

#undef CHECK_STATE

#endif //ZENGINE_PLAYERCONTROLLER_HPP
