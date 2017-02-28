//
// Created by fotyev on 2016-10-26.
//

#include <cstdio>


#include "components/network.hpp"

#include <cstdlib>

#include <thread>

int main(int argc, char * argv[])
{
    uint64_t id = (argc > 1) ? std::atoll(argv[1]) : 0;
    printf("%llu\n", id);
    fflush(stdout);
    //return 0;
    network_c network;
    network.id = id;


    if(id == 0)
        network.port = 9999;
    else
        network.add_new_node(0, "127.0.0.1", 9999);

    network.receive_callback = [](net_node_id id, const char * s, std::size_t len) {
        std::cout << id << ':';
        std::cout.write(s, len);
        std::cout << std::endl;
    };

    std::thread t{[&network]() { return network.run(); }};


    std::string str;
    net_node_id to = 0;


    while(std::cin >> to >> str)
    {
        network.send_data(to, str.data(), str.size());
    }

    t.join();
}
