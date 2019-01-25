//
// Created by fotyev on 2017-02-27.
//

#ifndef ZENGINE_NETGAME_IMPL_HPP
#define ZENGINE_NETGAME_IMPL_HPP

#include "components/netgame.hpp"
#include "main.hpp"
#include "components/network.hpp"
#include "util/semiconfig.hpp"

#include <thread>

#include <boost/container/flat_set.hpp>

#include <cereal/archives/binary.hpp>

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

    boost::container::flat_set<net_node_id> allpeers;

    big_msg bigev;
public:

    netgame_c()
    {
        assume(g_app->network);

        if(auto ip = static_config::at_optional<std::string>(ID("network.bind.ip")))
        {
            network.bind_ip = ip->c_str();
            network.port = SCFG(network.bind.port, 0);
        }

        network.id = SCFG(network.id, (net_node_id)0);
        while(!network.id)
            network.id = (net_node_id) rand();
        NETLOG(info, "my id", network.id);

        network.ping_rate = SCFG(network.ping_rate, 5000u);

        network.add_new_node(0,
                             SCFG(network.master.ip, "127.0.0.1").c_str(),
                             SCFG(network.master.port, (uint16_t)9999));

        network.receive_callback = [this] (auto... args) { this->on_receive(args...); };

        network.connect_callback = [this] (net_node_id id) { this->on_connect(id); return true; };
        network.disconnect_callback = [this] (net_node_id id) { this->on_disconnect(id); };

        if(SCFG(network.enabled, true))
            netthread = std::thread{[this]() { network.run(); }};
    }

    void send_input(net_node_id id, const tick_input_t& input) override
    {
        send_evstr(id, cserialize<cereal::BinaryOutputArchive>(input));
    }
    void send_input(const std::vector<net_node_id>& ids, const tick_input_t& input) override
    {
        auto str = cserialize<cereal::BinaryOutputArchive>(input);
        for(auto id : ids)
        {
            if(id != network.id)
                send_evstr(id, str);
        }
    }

protected:
    void send_evstr(net_node_id id, string_view inputstr)
    {
        if(inputstr.size() < network_c::mtu - sizeof(net_msg_header_t))
        {
            header_t header = 'E';
            network.send_data(id, inputstr.data(), inputstr.size(), &header, sizeof(header));
        }
        else
        {
            header_t header = 'B';
            big_msg::async_send(network, id, inputstr.data(), inputstr.size(), &header, sizeof(header));
        }
    }

protected:

    void on_connect(net_node_id id)
    {
        //if(id != 0)
        allpeers.insert(id);
        //=- register_event(name=>'node_connect', params=>[]);
        on_event(tick_input_t{id, 0/*???*/, event::node_connect{}});
    }

    void on_disconnect(net_node_id id)
    {
        allpeers.erase(id);
        //=- register_event(name=>'node_disconnect', params=>[]);
        on_event(tick_input_t{id, 0/*???*/, event::node_disconnect{}});
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
                    tick_input_t inp;
                    deserialize<cereal::BinaryInputArchive>({data, len}, inp);
                    NETLOG(debug3, "event from", id, "tick", inp.tick, dump_event(inp.event));
                    //EVENT_VISITOR_ALL(inp.event, ([this, &inp](const auto& event) -> void { this->on_event(inp, event);}));

                    if(inp.player != id)
                    {
                        NETLOG(warn, "spoofed id", inp.player);
                        //break;
                    }
                    on_event(std::move(inp));
                }
                catch(std::exception& e)
                {
                    NETLOG(error, "exc:", e.what());
                }

                break;
            case 'B': // state
            {
                bigev.on_new_msg(data, len);
                if(bigev.ready())
                {
                    tick_input_t inp;
                    deserialize<cereal::BinaryInputArchive>({bigev.data(), bigev.size()}, inp);
                    on_event(std::move(inp));
                    bigev.clear();
                }
                break;
            }

            default:
                NETLOG(error, "bad header:", header);
                break;
        }
    }
public:


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
        if(SCFG(network.enabled, true))
        {
            network.receive_callback = nullptr;
            network.stop();
            netthread.join();
        }
    }
};


#endif //ZENGINE_NETGAME_IMPL_HPP
