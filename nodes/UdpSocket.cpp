// nodes/UDPWebSocket.cpp

#include <zmq.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <utils/io_utils.h>
#include <utils/socket_utils.h>

namespace asio    = boost::asio;
namespace websocket = boost::beast::websocket;
using udp         = asio::ip::udp;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: UDPWebSocket <port> --inputs <uri1> [uri2...]\n";
        return 1;
    }

    unsigned short port = 0;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    parse_io_args(argc,argv,inputs,outputs);
    if (!port) {
        std::cerr << "Error: invalid port\n";
        return 1;
    }
    if (inputs.empty()) {
        std::cerr << "Error: no --inputs endpoints\n";
        return 1;
    }

    // 1) Set up ZeroMQ PULL sockets
    zmq::context_t ctx{1};
    std::vector<zmq::socket_t> pullers;
    pullers.reserve(inputs.size());
    for (auto &ep : inputs) {
        zmq::socket_t s{ctx, zmq::socket_type::pull};
        s.connect(ep);
        pullers.emplace_back(std::move(s));
        std::cout << "UDPWebSocket connected PULL to " << ep << "\n";
    }

    // 2) Set up a simple UDP socket for sending (to mimic WebSocket sink)
    asio::io_context ioc;
    udp::socket socket{ioc};
    socket.open(udp::v4());
    udp::endpoint client_endpoint{asio::ip::make_address("127.0.0.1"), port};
    std::cout << "UDPWebSocket sending datagrams to 127.0.0.1:" << port << "\n";

    // 3) Poll & forward loop
    std::vector<zmq::pollitem_t> items;
    for (auto &s : pullers)
        items.push_back({ s.handle(), 0, ZMQ_POLLIN, 0 });

    while (true) {
        zmq::poll(items.data(), items.size(), -1);
        for (size_t i = 0; i < items.size(); ++i) {
            if (items[i].revents & ZMQ_POLLIN) {
                zmq::message_t msg;
                pullers[i].recv(msg, zmq::recv_flags::none);
                socket.send_to(asio::buffer(msg.data(), msg.size()), client_endpoint);
            }
        }
    }

    return 0;
}
