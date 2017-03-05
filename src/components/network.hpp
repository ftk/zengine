//
// Created by fotyev on 2016-10-24.
//

#ifndef ZENGINE_NETWORK_HPP
#define ZENGINE_NETWORK_HPP

#include <boost/asio.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>


#include <boost/noncopyable.hpp>
#include <boost/endian/arithmetic.hpp>


#include <array>
#include <cstdint>
#include <atomic>
#include <map>

#include "util/optional.hpp"


#ifndef NETLOG
//#include <cstdio>
#if defined NETLOG_ENABLE
#include "util/log.hpp"
//#define NETLOG(level,...) logger(log_level:: level, #level, __FILE__, __LINE__, ## __VA_ARGS__)
#define NETLOG(level,...) LOGGER(level, #level, ## __VA_ARGS__)
//#define NETLOG(fmt,...) do {fprintf(stderr, "netlog: " fmt "\n", ## __VA_ARGS__); fflush(stderr);}while(0)
#else
#define NETLOG(...) ((void)0)
#endif
#endif

#include "util/assert.hpp"


typedef uint64_t net_node_id;

typedef boost::endian::little_uint64_t net_uint64_t;
typedef boost::endian::little_uint32_t net_uint32_t;
typedef boost::endian::little_uint16_t net_uint16_t;

constexpr const static uint32_t network_version = 1;

#pragma pack(push,1)
struct net_msg_header_t
{
    enum
    {
        CONNECT = 1,
        CONNECT_ACK,

        PEER_EXCH,

        PING,
        PING_ACK,

        DATA,
        DATA_ACK,
        DATA_UNRELIABLE,

        PACKED_MSG, // multiple msgs in one packet, each msg should be not more than 255 bytes

    };

    net_uint64_t id;
    net_uint16_t type;

    union
    {
        struct
        {
            net_uint32_t version;
        } connect;
        struct
        {
            net_uint64_t timestamp;
        } ping;
        struct
        {
            net_node_id id;
            net_uint16_t port;
            char ip[64];
        } peer_exchange;

        struct
        {
            net_uint16_t ack;
            net_uint32_t ack_bits;

        } data_ack;
        struct
        {
            /* example with parts=2
             * id[64] type=PACKED_MSG[16] parts=2[8] partlength=2+4,2+8[8*2] part1: type=CONNECT[16] version=1[32] part2: type=PING[16] timestamp[64]
             */
            uint8_t parts;
            //uint8_t partlength[]; // partlength[parts-1]
            // char packets[];
        } packed;
        struct
        {
            net_uint16_t sequence;
            uint8_t send_ack;
            char data[];
        } data;
    } msg;

};
static_assert(sizeof(net_msg_header_t) == 8 + 2 + 8 + 2 + 64, "net_msg_header_t is not packed!");
#pragma pack(pop)



struct netmsg
{
    boost::asio::ip::udp::socket& socket;
    boost::asio::ip::udp::endpoint endpoint;
    std::shared_ptr<char> ptr;
    unsigned size;

    netmsg(boost::asio::ip::udp::socket& socket, boost::asio::ip::udp::endpoint endpoint, unsigned size = sizeof(net_msg_header_t)) :
            socket(socket), endpoint(std::move(endpoint)),
            ptr(new char[size], std::default_delete<char[]>()),
            size(size)
    {}

    net_msg_header_t * operator->()
    {
        return reinterpret_cast<net_msg_header_t *>(ptr.get());
    }

    void send()
    {
        auto ptr_copy = ptr;
        socket.async_send_to(boost::asio::buffer(ptr.get(), size), endpoint, [ptr_copy]
                (boost::system::error_code ec, std::size_t transferred) {
            if(ec.value())
            {
                NETLOG(warn, "send error", ec.message());
            }
        });
    }

};

//=- register_component(class=>'network_c', name=>'network', priority=>70);
class network_c
{
private:
    struct net_node_t : private boost::noncopyable
    {
        const net_node_id id;
        boost::asio::ip::udp::endpoint endpoint;

        boost::posix_time::ptime last_ack;


        enum
        {
            NEW,
            CONNECTING,
            CONNECTED_NOT_ACKED,
            CONNECTED,
        } state;

        using packet_data = std::vector<char>;
        constexpr static const std::size_t max_packets = 1024; // max packets to send at once

        struct
        {
            bool ack = false;
            uint32_t seq = 0xffffffff;
            packet_data data;
        } sent_packets[max_packets];

        int connection_tries = 10;

        std::atomic<uint16_t> sequence{(uint16_t)0};
        //uint16_t sequence;


        packet_data& on_packet_send(packet_data packet)
        {
            auto msg = reinterpret_cast<const net_msg_header_t *>(packet.data());
            assert(msg->type == net_msg_header_t::DATA);
            const std::size_t index = msg->msg.data.sequence % max_packets;
            sent_packets[index].ack = false;
            sent_packets[index].seq = msg->msg.data.sequence;
            return sent_packets[index].data = std::move(packet);
        }

        bool is_acked(uint16_t seq)
        {
            const std::size_t index = seq % max_packets;

            return sent_packets[index].seq == seq && sent_packets[index].ack;
        }

        packet_data * get_packet(uint16_t seq)
        {
            const std::size_t index = seq % max_packets;
            if(sent_packets[index].seq != seq)
                return nullptr;
            if(sent_packets[index].data.empty())
                return nullptr;
            return &sent_packets[index].data;
        }

        void on_ack(uint16_t seq)
        {
            const std::size_t index = seq % max_packets;
            if(!sent_packets[index].ack && sent_packets[index].seq == seq)
            {
                sent_packets[index].data.clear();
                sent_packets[index].ack = true;
            }
        }

        struct
        {
            uint32_t seq = 0xffffffff;
        } rcvd_packets[max_packets];


        void on_packet_recv(uint16_t seq)
        {
            const std::size_t index = seq % max_packets;
            rcvd_packets[index].seq = seq;
        }
        bool is_rcvd(uint16_t seq)
        {
            const std::size_t index = seq % max_packets;
            return rcvd_packets[index].seq == seq;
        }

        uint16_t latest_rcvd_seq = sequence - 1;
        bool is_latest(uint16_t seq)
        {
            if(seq > latest_rcvd_seq)
            {
                latest_rcvd_seq = seq;
                return true;
            }
            if(latest_rcvd_seq > (1 << 15) && seq < (latest_rcvd_seq - (1 << 15))) // overflowed
            {
                latest_rcvd_seq = seq;
                return true;
            }
            return false;
        }

        net_node_t(net_node_id id) : id(id), state(NEW) { last_ack = boost::posix_time::microsec_clock::universal_time(); }
        bool to_delete() const {
            return (connection_tries < 0 ||
                    (boost::posix_time::microsec_clock::universal_time() - last_ack) >
                            boost::posix_time::seconds(30)
                   )
                   && id != 0;

        }

        void remove() { connection_tries = -1; };

    };

public:
    boost::asio::io_service io_service;


    net_node_id id;
    uint16_t port = 0;
    const char * bind_ip = nullptr;

    // callbacks
    std::function<void (net_node_id, const char *, std::size_t)> receive_callback;
    std::function<void (net_node_id, const char *, std::size_t)> receive_callback_unr; //

    std::function<bool (net_node_id)> connect_callback; // if returns true, allow connection
    std::function<void (net_node_id)> disconnect_callback; //

    constexpr static const std::size_t mtu = 1400;
private:
    // buffer
    std::array<char, mtu> buf;
    boost::asio::ip::udp::endpoint sender;

    boost::asio::ip::udp::socket socket;

    boost::asio::deadline_timer timer;

    std::map<net_node_id, net_node_t> nodes;
    static constexpr std::size_t max_nodes = 32;

private:

    net_node_t * get_node(net_node_id nid)
    {
        if(nid == id)
            return nullptr;
        auto it = nodes.find(nid);
        if(it != nodes.end())
        {
            if(it->second.to_delete())
            {
                NETLOG(info, "deleted", nid);
                if(disconnect_callback)
                    disconnect_callback(it->first);
                nodes.erase(it);
            } else
            {
                assert(it->second.id == nid);
                return &it->second;
            }
        }

        // not found
        return nullptr;
    }

    net_node_t * get_or_add_node(net_node_id nid)
    {
        auto ptr = get_node(nid);
        if(ptr)
            return ptr;

        if(nid == id)
            return nullptr;

        if(nodes.size() >= max_nodes)
            return nullptr;
        // create new node
        // well, this is embarassing
        return &nodes.emplace(std::piecewise_construct,
                              std::forward_as_tuple(nid),
                              std::forward_as_tuple(nid)).first->second;

    }
public:

    network_c(net_node_id id = 1) : id(id), socket(io_service), timer(io_service, boost::posix_time::seconds(15))
    {

    }

    void run() noexcept;
    void stop() { io_service.stop(); } // signals to stop


    /*template <typename Callable>
    void for_each_node(Callable f)
    {
        std::for_each(nodes.begin(), nodes.end(), [this, &f](auto & pair) {
            if(this->get_node(pair.first))
                f(pair.first);
        });
    }*/

    bool send_data(net_node_id nid, const void * buf, std::size_t len,
                   const void * header = nullptr, std::size_t header_len = 0);

    void add_new_node(net_node_id new_id, const char * ip, uint16_t port);

private:
    void receive()
    {
        //using namespace std::placeholders;
        //socket.async_receive_from(boost::asio::buffer(buf), sender, std::bind(on_receive, this, _1, _2));
        socket.async_receive_from(boost::asio::buffer(buf), sender, [this] (auto ec, auto rcvd) {
            this->on_receive(ec, rcvd);
            this->receive();
        });
    }



    void on_timer();

    void on_receive(boost::system::error_code ec, std::size_t bytes_recvd);


    netmsg create_reply(uint32_t type)
    {
        netmsg msg{socket, sender};

        msg->id = this->id;
        msg->type = type;
        //NETLOG("creating reply %d to %s:%d", type, sender.address().to_string().c_str(), sender.port());
        return msg;
    }


    void peer_exchange(net_node_t * node);

    void broadcast(net_node_t * node);
    void send_peers(net_node_t * node);
};


class big_msg
{

    struct ord_header
    {
        net_uint32_t length; // total msg len
        net_uint32_t offset; // offset of the current part
        char data[];
    };
    static_assert(sizeof(ord_header) == 4+4, "ord_header not packed!");

    std::vector<char> msg;
    uint32_t rcvd = 0;



public:
    static constexpr const std::size_t max_size = 1024 * 1024; // 1mb

#if 0/*% # export methods
     my @funs = qw(begin begin_c end end_c cbegin_c cend_c
     rbegin rbegin_c rend rend_c crbegin_c crend_c
     size_c empty_c data_c data);
     "\t" . join "\n\t", map { my $c=''; $c = "const" if s/_c$//; "auto $_() $c -> decltype(msg.$_()) { return msg.$_(); }" } @funs;
     #*/
#else
	auto begin()  -> decltype(msg.begin()) { return msg.begin(); }
	auto begin() const -> decltype(msg.begin()) { return msg.begin(); }
	auto end()  -> decltype(msg.end()) { return msg.end(); }
	auto end() const -> decltype(msg.end()) { return msg.end(); }
	auto cbegin() const -> decltype(msg.cbegin()) { return msg.cbegin(); }
	auto cend() const -> decltype(msg.cend()) { return msg.cend(); }
	auto rbegin()  -> decltype(msg.rbegin()) { return msg.rbegin(); }
	auto rbegin() const -> decltype(msg.rbegin()) { return msg.rbegin(); }
	auto rend()  -> decltype(msg.rend()) { return msg.rend(); }
	auto rend() const -> decltype(msg.rend()) { return msg.rend(); }
	auto crbegin() const -> decltype(msg.crbegin()) { return msg.crbegin(); }
	auto crend() const -> decltype(msg.crend()) { return msg.crend(); }
	auto size() const -> decltype(msg.size()) { return msg.size(); }
	auto empty() const -> decltype(msg.empty()) { return msg.empty(); }
	auto data() const -> decltype(msg.data()) { return msg.data(); }
	auto data()  -> decltype(msg.data()) { return msg.data(); }
#endif

void clear()
{
    msg.clear();
    rcvd = 0;
}
    uint32_t received() const noexcept
    {
        return rcvd;
    }
    bool ready() const noexcept
    {
        return !empty() && rcvd == size();
    }


    /*
     * on_new_msg expects : ord_header CHUNK
     */
    void on_new_msg(const char * str, std::size_t len);



    /* sends [data, data+len) in chunks:
     * resulting messages : net_msg_header HEADER ord_header CHUNK
     *
     */

    static void async_send(network_c& net, net_node_id id, const char * data, std::size_t len,
        const void * header = nullptr, std::size_t header_len = 0);

};

#endif //ZENGINE_NETWORK_HPP
