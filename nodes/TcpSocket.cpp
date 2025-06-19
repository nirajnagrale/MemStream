// nodes/TCPWebSocket.cpp

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
using tcp         = asio::ip::tcp;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: TCPWebSocket <port> --inputs <uri1> [uri2...]\n";
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
    std::vector<zmq::socket_t> pullers = create_pull_sockets(ctx,inputs);
    std::vector<zmq::socket_t> pushers = create_push_sockets(ctx,outputs);
    
    // 2) Set up Beast WebSocket server
    asio::io_context ioc;
    tcp::acceptor acceptor{ioc, {tcp::v4(), port}};
    std::cout << "TCPWebSocket listening on port " << port << "\n";

    tcp::socket sock{ioc};
    acceptor.accept(sock);
    websocket::stream<tcp::socket> ws{std::move(sock)};
    ws.accept();
    std::cout << "TCPWebSocket accepted connection\n";

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
                std::string data{ (char*)msg.data(), msg.size() };
                ws.write(asio::buffer(data));
            }
        }
    }

    return 0;
}
