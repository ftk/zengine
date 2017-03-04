//
// Created by fotyev on 2017-02-27.
//

#ifndef ZENGINE_NETGAME_IMPL_HPP
#define ZENGINE_NETGAME_IMPL_HPP

#include "components/netgame.hpp"
#include "main.hpp"
#include "components/network.hpp"
#include "components/config.hpp"

#include <thread>

#include <boost/container/flat_map.hpp>

#include "util/log.hpp"
#include "util/assert.hpp"

//=- register_component(class=>'netgame_c', interface=>'netgame_i', name=>'netgame', priority=>91, scriptexport=>[qw(id nodes_list)]);
class netgame_c : public netgame_i
{
    std::thread netthread;
public:
    network_c& network = *(g_app->network);
private:
    using header_t = uint8_t;

    boost::container::flat_map<net_node_id, int> playeroffsets; // remote + offset = local

    std::function<void (tick_input_t)> event_callback;
public:

    netgame_c()
    {
        assert(g_app->network);

        //=- collect('config', {name=>'mastersrv_ip', type=>'string', def=>'"127.0.0.1"', save=>1});
        //=- collect('config', {name=>'mastersrv_port', type=>'uint16_t', def=>'9999', save=>1});

        network.id = (net_node_id) rand();
        NETLOG(info, "my id", network.id);

        network.add_new_node(0, g_app->config->mastersrv_ip.c_str(), (uint16_t)g_app->config->mastersrv_port);

        network.receive_callback = [this] (auto... args) { this->on_receive(args...); };

        network.connect_callback = [this] (net_node_id id) { this->on_connect(id); return true; };
        network.disconnect_callback = [this] (net_node_id id) { this->on_disconnect(id); };

        netthread = std::thread{[this]() { network.run(); }};
    }
protected:

    void send_input(net_node_id id, string_view inputstr)
    {
        header_t header = 'E';
        network.send_data(id, inputstr.data(), inputstr.size(), &header, sizeof(header));
    }
    std::string make_input(event_t event) const
    {
        return serialize({id(), get_tick(), std::move(event)});
    }
public:
    void send_event(net_node_id id, event_t event) override
    {
        return send_input(id, make_input(std::move(event)));
    }
protected:

    void on_connect(net_node_id id)
    {
        //=- register_event(name=>'connect_req');
        send_event(id, event::connect_req{});
    }



    void on_event(const tick_input_t& input, event::connect_req)
    {
        //=- register_event(name=>'connect_ack', params=>[[qw(uint32_t tick)]]);
        send_event(input.player, event::connect_ack{input.tick});
    }

    void on_event(const tick_input_t& input, event::connect_ack ack)
    {
        tick_t ticks[3] = {ack.tick, // local tick when sent req
                           input.tick, // remote tick when sent ack
                           get_tick()}; // current tick
        NETLOG(info, "pinged:", ticks[0], ticks[1], ticks[2]);
        if(ticks[2] - ticks[0] < 6)
        {
            playeroffsets[input.player] = (int) (ticks[0] + ticks[2]) / 2 - (int) ticks[1];
        }
        else
        {
            NETLOG(warn, input.player, "has too big ping:", ticks[2] - ticks[0]);
            // disconnect
        }
    }

    template <typename E>
    void on_event(const tick_input_t& input, const E&)
    {

    }



    template <typename T>
    void operator()(const T&) {}

    //=- register_event(name=>'disconnect');
    void on_disconnect(net_node_id id)
    {
        if(playeroffsets.erase(id))
        {
            // pseudo-event
            event_callback(tick_input_t{id, get_tick(), event::disconnect{}});
        }
    }

    void on_receive(net_node_id id, const char * data, std::size_t len)
    {
        // parse header
        header_t header = *reinterpret_cast<const header_t *>(data);
        data += sizeof(header_t);
        len -= sizeof(header_t);

        // parse data depending on the header
        switch(header)
        {

            case 'E': // event
                try
                {

                    auto inp = deserialize({data, len});
                    NETLOG(debug3, "event", id, dump_event(inp.event));
                    boost::apply_visitor([this, &inp](const auto& event) -> void { this->on_event(inp, event);}, inp.event);
                    //EVENT_VISITOR_ALL(inp.event, ([this, &inp](const auto& event) -> void { this->on_event(inp, event);}));

                    if(!playeroffsets.count(id))
                    {
                        NETLOG(warn, "event from", id, ", which is disconnected");
                        break;
                    }
                    if(event_callback)
                    {
                        inp.tick += playeroffsets[id];
                        if(inp.player != id)
                            NETLOG(warn, "spoofed id", inp.player);
                        event_callback(std::move(inp));

                    }
                }
                catch(std::exception& e)
                {
                    NETLOG(error, "exc:", e.what());
                }

                break;

            default:
                NETLOG(error, "bad header:", header);
                break;
        }
    }
public:
    void broadcast_input(const tick_input_t& event) override
    {
        auto str = serialize(event);
        for(auto& p : playeroffsets)
        {
            send_input(p.first, str);
        }
    }

    void set_event_handler(std::function<void(tick_input_t)> handler) override
    {
        event_callback = std::move(handler);
    }

    net_node_id id() const override
    {
        return network.id;
    }

    std::vector<net_node_id> nodes_list() const override
    {
        std::vector<net_node_id> list;
        list.reserve(playeroffsets.size());
        for(const auto& pair : playeroffsets) list.push_back(pair.first);
        return list;
    }

    ~netgame_c()
    {
        network.receive_callback = nullptr;
        network.stop();
        netthread.join();
    }
};

#endif //ZENGINE_NETGAME_IMPL_HPP
