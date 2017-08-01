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

#include <boost/container/flat_set.hpp>

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


    std::function<void (tick_input_t)> event_callback;

    boost::container::flat_set<net_node_id> allpeers;
public:

    netgame_c()
    {
        assume(g_app->network);

        if(auto ip = g_app->config->get_optional<std::string>("network.bind.ip"))
        {
            network.bind_ip = ip->c_str();
            network.port = g_app->config->get("network.bind.port", 0);
        }

        network.id = g_app->config->get<net_node_id>("network.id", 0);
        while(!network.id)
            network.id = (net_node_id) rand();
        NETLOG(info, "my id", network.id);

        network.add_new_node(0,
                             g_app->config->get("network.master.ip", "127.0.0.1").c_str(),
                             (uint16_t)g_app->config->get("network.master.port", 9999));

        network.receive_callback = [this] (auto... args) { this->on_receive(args...); };

        network.connect_callback = [this] (net_node_id id) { this->on_connect(id); return true; };
        network.disconnect_callback = [this] (net_node_id id) { this->on_disconnect(id); };

        netthread = std::thread{[this]() { network.run(); }};
    }

    void send_input(net_node_id id, const tick_input_t& input) override
    {
        send_evstr(id, serialize(input));
    }
    void send_input(const std::vector<net_node_id>& ids, const tick_input_t& input) override
    {
        auto str = serialize(input);
        for(auto id : ids)
        {
            if(id != network.id)
                send_evstr(id, str);
        }
    }

protected:
    void send_evstr(net_node_id id, string_view inputstr)
    {
        header_t header = 'E';
        network.send_data(id, inputstr.data(), inputstr.size(), &header, sizeof(header));
    }

protected:

    void on_connect(net_node_id id)
    {
        //if(id != 0)
        allpeers.insert(id);
        //=- register_event(name=>'node_connect', params=>[]);
        if(event_callback)
            event_callback(tick_input_t{id, 0/*???*/, event::node_connect{}});
    }

    void on_disconnect(net_node_id id)
    {
        allpeers.erase(id);
        //=- register_event(name=>'node_disconnect', params=>[]);
        if(event_callback)
            event_callback(tick_input_t{id, 0/*???*/, event::node_disconnect{}});
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
                    NETLOG(debug3, "event from", id, "tick", inp.tick, dump_event(inp.event));
                    //EVENT_VISITOR_ALL(inp.event, ([this, &inp](const auto& event) -> void { this->on_event(inp, event);}));

                    if(event_callback)
                    {
                        if(inp.player != id)
                        {
                            NETLOG(warn, "spoofed id", inp.player);
                            //break;
                        }
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
        //std::vector<net_node_id> list;
        return std::vector<net_node_id>(allpeers.begin(), allpeers.end());
    }

    ~netgame_c()
    {
        network.receive_callback = nullptr;
        network.stop();
        netthread.join();
    }
};


#endif //ZENGINE_NETGAME_IMPL_HPP
