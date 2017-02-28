//
// Created by fotyev on 2016-10-24.
//

#include <cstdio>


#include "network.hpp"

#include <boost/bind.hpp>
#include <chrono>

void network_c::on_timer()
{
    //NETLOG(debug, "timer fired");


    for(auto it = nodes.begin(); it != nodes.end(); )
    {
        // delete old
        if(it->second.to_delete())
        {
            NETLOG(info, "disconnected", it->first);
            if(disconnect_callback)
                disconnect_callback(it->first);
            it = nodes.erase(it);
        }
        else
            ++it;
    }

    for(auto& p : nodes)
    {
        auto& node = p.second;

        NETLOG(debug, "handling", p.first);

        switch(node.state)
        {
            case net_node_t::CONNECTING:
                node.connection_tries--;
                //fall
            case net_node_t::NEW:
            {
                // send connect
                auto msg = netmsg{socket, node.endpoint};;
                msg->id = this->id;
                msg->type = net_msg_header_t::CONNECT;
                msg->msg.connect.version = network_version;
                msg.send();
                node.state = net_node_t::CONNECTING;
                break;
            }
            case net_node_t::CONNECTED:
            {
                // send ping
                // ping packet
                auto now = boost::posix_time::microsec_clock::universal_time() -
                           boost::posix_time::ptime{boost::gregorian::date(2010, 1, 1)};

                auto pingmsg = netmsg{socket, node.endpoint};
                pingmsg->id = this->id;
                pingmsg->type = net_msg_header_t::PING;

                pingmsg->msg.ping.timestamp = now.total_microseconds();

                pingmsg.send();

                // resend latest data packet

                const uint16_t seq = node.sequence - 1;
                if(node.get_packet(seq))
                {
                    NETLOG(debug, "resending", seq);
                    auto m = reinterpret_cast<net_msg_header_t *>(node.get_packet(seq)->data());
                    assert(m->type = net_msg_header_t::DATA);
                    m->msg.data.send_ack = 1;
                    socket.async_send_to(boost::asio::buffer(*(node.get_packet(seq))), node.endpoint,
                                         [](boost::system::error_code ec, std::size_t sent) {
                                             if(ec.value())
                                             {
                                                 NETLOG(warn, "error resending", ec.message());
                                             }
                                         });
                }


            }
        }

    }


    timer.expires_from_now(boost::posix_time::seconds(7));
    timer.async_wait(boost::bind(&network_c::on_timer, this));
}

void network_c::run() noexcept
{
    try
    {
        using boost::asio::ip::udp;

        socket.open(udp::v4());

        //boost::asio::ip::v6_only option(true);
        //socket.set_option(option);


        if(port != 0)
            socket.bind(udp::endpoint(udp::v4(), port));

        on_timer();

        receive();
        //timer.async_wait(boost::bind(&network_c::on_timer, this));

        //while(true)
        io_service.run();

        socket.close();

        NETLOG(debug, "network shutting down");
    }
    catch(std::exception& e)
    {
        std::cerr << "EXC in network thread: " << e.what() << std::endl;
    }

}

void network_c::on_receive(boost::system::error_code ec, std::size_t bytes_recvd)
{
    if(ec.value())
    {
        //io_service.stop();
        // todo: delete node
        for(auto& p : nodes)
        {
            if(p.second.endpoint == sender)
            {
                NETLOG(info, "removing", p.second.id, ":", ec.value(), ec.message());
                p.second.remove();
                return;
            }
        }
        NETLOG(info, "rcv error!!!", ec.value(), ec.message(), sender.address().to_string(), sender.port());
        return;
    }


    if(bytes_recvd < 10)
    {
        buf[16] = '\0';
        NETLOG(info, "rcv smth strange:", buf.data());
        return;
    }
    auto msg = reinterpret_cast<net_msg_header_t *>(buf.data());

    if(!get_node(msg->id) && this->id != 0)
    {
        NETLOG(info, "rejected packet", msg->type, "from", msg->id, "(nat simulation)");
        return;
    }

    // return if sent reply, break if not
    switch(msg->type)
    {
        case net_msg_header_t::PING:
        {
            auto m = create_reply(net_msg_header_t::PING_ACK);
            m->msg.ping = msg->msg.ping;
            m.send();

            auto node = get_node(msg->id);
            if(node) // TODO: prevent timeout
                node->last_ack = boost::posix_time::microsec_clock::universal_time();
        } return;
        case net_msg_header_t::PING_ACK:
        {
            auto now = boost::posix_time::microsec_clock::universal_time() - boost::posix_time::ptime{boost::gregorian::date(2010,1,1)};
            NETLOG(info, "rtt to", msg->id, ":", now.total_microseconds() - msg->msg.ping.timestamp);
        } return;
        case net_msg_header_t::CONNECT:
        {
            if(msg->msg.connect.version != network_version)
            {
                NETLOG(info, "connect with wrong version", msg->msg.connect.version);
                break; // wrong version
            }
            auto node = get_or_add_node(msg->id);
            if(!node)
                return;

            if(node->state == net_node_t::NEW)
            {
                // node created by get_or_add_node
                node->endpoint = sender;
                // accept connection
                node->state = net_node_t::CONNECTING;

                /*auto m = create_reply(net_msg_header_t::CONNECT);
                m->msg.connect.version = network_version;
                m.send();*/
            }
            /*else*/ if(node->state == net_node_t::CONNECTING)
            {
                node->state = net_node_t::CONNECTED_NOT_ACKED;
                // send ack
                auto m = create_reply(net_msg_header_t::CONNECT_ACK);
                m.send();
            } else // bad state
            {
                NETLOG(warn, "received connect from node with state", node->state);
                break; // ignore
            }
        } return;

        case net_msg_header_t::CONNECT_ACK:
        {
            auto node = get_node(msg->id);
            if(!node)
                break;
            if(node->state != net_node_t::CONNECTED_NOT_ACKED && node->state != net_node_t::CONNECTING)
            {
                NETLOG(warn, "received connect_ack from node with state", node->state);
                break;
            }
            if(node->state == net_node_t::CONNECTING)
            {
                auto m = create_reply(net_msg_header_t::CONNECT_ACK);
                m.send();
            }

            node->state = net_node_t::CONNECTED;
            NETLOG(info, "connected to", node->id);
            if(connect_callback)
            {
                if(!connect_callback(node->id))
                {
                    // TODO: disconnect
                }
            }

            if(id == 0)
            {
                peer_exchange(node);
            }
        } return;
    }

    auto node = get_node(msg->id);
    if(!node)
    {
        NETLOG(warn, "rcvd msg", msg->type, "from bad nodeid", msg->id);
        return;
    }
    if(node->endpoint != sender)
    {
        NETLOG(warn, "spoofed", msg->id, "from", sender.address().to_string(), sender.port());
        return;
    }

    if(node->state != net_node_t::CONNECTED)
    {
        if(node->state == net_node_t::CONNECTED_NOT_ACKED)
        {
            NETLOG(warn, "acknowloging (ack lost)", msg->id);
            node->state = net_node_t::CONNECTED;
            NETLOG(info, "connected to", node->id);
            if(connect_callback)
            {
                if(!connect_callback(node->id))
                {
                    // TODO: disconnect
                }
            }
            if(id == 0)
                peer_exchange(node);
        }
        else
        {
            NETLOG(warn, "rcvd msg", msg->type, "from", msg->id, "with state", node->state);
            return;
        }
    }

    // prevent timeout
    node->last_ack = boost::posix_time::microsec_clock::universal_time();


    switch(msg->type)
    {

        case net_msg_header_t::PEER_EXCH:
        {
            char * ip = msg->msg.peer_exchange.ip;
            ip[63] = '\0';
            uint16_t port = msg->msg.peer_exchange.port;


            add_new_node(msg->msg.peer_exchange.id, ip, port);

            break;
        }


        case net_msg_header_t::DATA:
        {

            uint16_t ack = msg->msg.data.sequence;

            constexpr std::size_t offset = offsetof(net_msg_header_t, msg.data.data);

            if(!node->is_rcvd(ack))
            {
                receive_callback(msg->id, buf.data() + offset, bytes_recvd - offset);
                node->on_packet_recv(ack);
            }


            // generate ack

            if(msg->msg.data.send_ack && node->is_latest(ack))
            {
                auto m = create_reply(net_msg_header_t::DATA_ACK);
                m->msg.data_ack.ack = ack;
                uint32_t ack_bits = 0;
                for(uint16_t i = 0; i < 32; i++)
                {
                    const uint16_t seq = ack - i - 1;
                    if(node->is_rcvd(seq))
                        ack_bits |= (1 << i);
                }
                m->msg.data_ack.ack_bits = ack_bits;
                NETLOG(debug4, "sending ack", ack, std::hex, ack_bits, std::dec);

                m.send();
            }
            break;

        }

        case net_msg_header_t::DATA_ACK:
        {
            uint16_t ack = msg->msg.data_ack.ack;
            uint32_t ack_bits = msg->msg.data_ack.ack_bits;

            /*uint16_t packet = node->sequence - 1;
            for( ; packet != ack && packet != node->sequence - 32; packet--)
            {

            }*/
            NETLOG(debug4, "rcvd ACK", ack, std::hex, ack_bits, std::dec);
            node->on_ack(ack);

            for(unsigned i = 0; i < 32; i++)
            {
                const uint16_t seq = ack - i - 1;
                if(!(ack_bits & (1 << i)))
                {
                    // packet #(ack-i-1) is not acked
                    if(node->is_acked(seq))
                    {
                        NETLOG(warn, "spoofed ACK");
                        //node->remove();
                        break;
                    }
                    // resend
                    if(node->get_packet(seq))
                    {
                        NETLOG(debug, "resending", seq);
                        auto m = reinterpret_cast<net_msg_header_t *>(node->get_packet(seq)->data());
                        assert(m->type == net_msg_header_t::DATA);
                        m->msg.data.send_ack = 0;
                        socket.async_send_to(boost::asio::buffer(*(node->get_packet(seq))), node->endpoint,
                                             [](boost::system::error_code ec, std::size_t sent) {
                                                 if(ec.value())
                                                 {
                                                     NETLOG(error, "error resending", ec.message());
                                                 }
                                             });
                    }
                }
                else
                {
                    node->on_ack(seq);
                }
            }

            break;
        }
        case net_msg_header_t::DATA_UNRELIABLE:
        {
            constexpr std::size_t offset = offsetof(net_msg_header_t, msg);
            if(receive_callback_unr)
                receive_callback_unr(msg->id, buf.data() + offset, bytes_recvd - offset);
        }

        case net_msg_header_t::PACKED_MSG:
        {
            // save header, it will be overwritten

            const uint8_t parts = msg->msg.packed.parts;
            uint8_t partlength[256];
            memcpy(partlength, &msg->msg.packed.parts + 1, parts);
            // offset of the first msg
            unsigned offset = 1/*offsetof(net_msg_header_t, msg.packed.partlength)*/ + parts;
            for(uint8_t i = 0; i < parts; i++)
            {
                if(offset + partlength[i] > bytes_recvd)
                {
                    // expected less
                    NETLOG(warn, "too short packed msg from", msg->id, ":", parts, offset);
                    break;
                }
                // move msg to the beginning (excl. id)
                memmove(&buf[0] + sizeof(msg->id), &buf[0] + offset, partlength[i]);
                // receive shouldn't touch buf past bytes_recvd
                on_receive(ec, sizeof(msg->id) + partlength[i]);

                offset += partlength[i];
            }
            if(offset != bytes_recvd) // <
            {
                // expected more
                NETLOG(warn, "too big packed msg from", msg->id, ":", parts, offset);
            }

        }




        default:
            // drop packet
            NETLOG(warn, "unknown packet", msg->type, "from", msg->id);
            break;
    }
}

bool network_c::send_data(net_node_id nid, const void * buf, std::size_t len, const void * header,
                          std::size_t header_len)
{
    auto node = get_node(nid);
    if(!node || node->state != net_node_t::CONNECTED)
    {
        NETLOG(error, "trying to send msg to invalid node", nid);
        return false;
    }
    auto seq = node->sequence++;

    constexpr std::size_t offset = offsetof(net_msg_header_t, msg.data.data);

    net_node_t::packet_data packet(offset + header_len + len);
    auto msg = reinterpret_cast<net_msg_header_t *>(packet.data());
    msg->id = this->id;
    msg->type = net_msg_header_t::DATA;
    msg->msg.data.sequence = seq;

    // request ack?
    msg->msg.data.send_ack = 1;

    if(header_len)
    {
        std::memcpy(packet.data()+offset, header, header_len);
    }
    std::memcpy(packet.data()+offset+header_len, buf, len);

    assert(msg->id == this->id);
    assert(msg->msg.data.sequence == seq);
    //std::copy(packet.begin()+13, packet.end(), buf); // cryptic!


    //NETLOG("sending %d: %s", seq, buf);
    const auto& p = node->on_packet_send(std::move(packet));
    socket.async_send_to(boost::asio::buffer(p), node->endpoint,
                         [](boost::system::error_code ec, std::size_t transferred) {
                             if(ec.value())
                             {
                                 NETLOG(error, "send error", ec.message());
                             }
                         });
    return true;
}

void network_c::add_new_node(net_node_id new_id, const char * ip, uint16_t port)
{
    auto new_node = get_node(new_id);
    if(new_node)
    {
        NETLOG(warn, "trying to add existing node", ip, port);
        return;
    }

    new_node = get_or_add_node(new_id);
    if(!new_node)
        return;

    new_node->endpoint = boost::asio::ip::udp::endpoint(
            boost::asio::ip::address::from_string(ip),
            port);
    NETLOG(debug, "added new node", new_node->endpoint.address().to_string(), port);

}

void network_c::peer_exchange(net_node_t * node)
{
    broadcast(node);
    send_peers(node);
}

void network_c::broadcast(network_c::net_node_t * node)
{
    NETLOG(debug, "broadcastin", node->id);
    auto m = create_reply(net_msg_header_t::PEER_EXCH);
    m->msg.peer_exchange.id = node->id;
    auto str = node->endpoint.address().to_string();
    assert(str.size() < sizeof(m->msg.peer_exchange.ip));
    std::copy(str.begin(), str.end(), m->msg.peer_exchange.ip);
    m->msg.peer_exchange.ip[str.size()] = '\0';
    m->msg.peer_exchange.port = node->endpoint.port();
    for(const auto& p : nodes)
    {
        if(p.second.to_delete() || p.second.state != net_node_t::CONNECTED)
            continue;
        m.endpoint = p.second.endpoint;
        m.send();
    }

}

void network_c::send_peers(network_c::net_node_t * node)
{
    NETLOG(debug, "sending peers to", node->id);
    // TODO: pack, encapsulate in DATA
    for(const auto& p : nodes)
    {
        if(p.second.to_delete() || p.second.state != net_node_t::CONNECTED || p.first == node->id)
            continue;

        auto m = create_reply(net_msg_header_t::PEER_EXCH);
        m->msg.peer_exchange.id = p.second.id;

        auto str = p.second.endpoint.address().to_string();
        assert(str.size() < sizeof(m->msg.peer_exchange.ip));
        std::copy(str.begin(), str.end(), m->msg.peer_exchange.ip);
        m->msg.peer_exchange.ip[str.size()] = '\0';
        m->msg.peer_exchange.port = p.second.endpoint.port();
        m.send();
    }
}

void big_msg::on_new_msg(const char * str, std::size_t len)
{
    if(len < sizeof(ord_header))
    {
        NETLOG(error, "too short");
        return;
    }
    auto packet = reinterpret_cast<const ord_header *>(str);
    if(packet->length != size())
    {
        if(packet->length > max_size)
        {
            NETLOG(error, "too big");
            return; // ignore
        }
        clear();
        msg.resize(packet->length);
    }
    const std::size_t data_len = len - offsetof(ord_header, data);
    if(packet->offset + data_len > size())
    {
        NETLOG(error, "bad offset");
        return;
    }
    std::copy(packet->data, packet->data + data_len, msg.begin() + packet->offset);
    rcvd += data_len;
    if(rcvd > size())
    {
        NETLOG(warn, "rcvd more than expected");
        rcvd = size();
    }
}

void big_msg::async_send(network_c& net, net_node_id id, const char * data, std::size_t len, const void * header,
                         size_t header_len)
{
    if(len > max_size)
        throw std::runtime_error("too big for big_msg");
    auto new_header = std::vector<char>(header_len + sizeof(ord_header));
    if(header_len)
        std::memcpy(&new_header[0], header, header_len);
    auto hdr = reinterpret_cast<ord_header *>(new_header.data() + header_len);
    hdr->length = len;
    hdr->offset = 0;

    while(len)
    {
        // TODO: rate limit
        const std::size_t part_size = std::min(network_c::mtu - 64, len);
        net.send_data(id, data, part_size, new_header.data(), new_header.size());

        data += part_size;
        len -= part_size;
        hdr->offset += part_size;
    }
}
