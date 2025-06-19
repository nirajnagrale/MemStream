// nodes/UDPWebSocket.cpp

#include <zmq.hpp>
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <future>
#include <utils/io_utils.h>
#include <utils/socket_utils.h>
#include<chrono>

namespace asio    = boost::asio;
using asio::ip::udp;

// Helper to hold both socket and endpoint
struct UdpSender {
    udp::socket   socket;
    udp::endpoint endpoint;
    UdpSender(asio::io_context &ioc, unsigned short port)
      : socket(ioc, udp::v4()),
        endpoint(asio::ip::make_address("127.0.0.1"), port)
    {}
};

/**
 * Spawns the Asio thread that:
 *  1) Constructs UdpSender (opens socket + endpoint),
 *  2) Sets the promise so main can proceed,
 *  3) Calls ioc.run() to service all ioc.post() sends.
 */
static std::thread start_udp_sender(asio::io_context &ioc,
                                    unsigned short port,
                                    std::promise<std::shared_ptr<UdpSender>> &p)
{
    return std::thread([&ioc, port, &p]() {
        auto sender = std::make_shared<UdpSender>(ioc, port);
        std::cout << "UDPWebSocket ready to send datagrams to "
                  << sender->endpoint << "\n";
        p.set_value(sender);
        ioc.run();
    });
}

/**
 * Generic ZMQ receive loop: pulls from each input URI and invokes handler(data).
 */
template<typename Handler>
void zmq_receive_loop(zmq::context_t &ctx,
                      const std::vector<std::string> &inputs,
                      Handler handler)
{
    auto pullers = create_pull_sockets(ctx, inputs);
    std::vector<zmq::pollitem_t> items;
    items.reserve(pullers.size());
    for (auto &s : pullers)
        items.push_back({ s.handle(), 0, ZMQ_POLLIN, 0 });

    while (true) {
        zmq::poll(items, std::chrono::milliseconds(-1));
        for (size_t i = 0; i < items.size(); ++i) {
            if (items[i].revents & ZMQ_POLLIN) {
                zmq::message_t msg;
                auto rc = pullers[i].recv(msg, zmq::recv_flags::none);
                if(!rc) continue;
                std::string data{
                    static_cast<char*>(msg.data()), msg.size()
                };
                handler(data);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // Usage: UDPWebSocket <port> --inputs <uri1> [uri2...] [--outputs <uriA>...]
    if (argc < 3) {
        std::cerr << "Usage: UDPWebSocket <port> --inputs <uri1> [uri2...] [--outputs <uriA>...]\n";
        return 1;
    }

    // 1) Parse port + I/O lists
    unsigned short port = static_cast<unsigned short>(std::stoi(argv[1]));
    std::vector<std::string> inputs, outputs;
    parse_io_args(argc, argv, inputs, outputs);
    if (inputs.empty()) {
        std::cerr << "Error: no --inputs endpoints\n";
        return 1;
    }

    // 2) Prepare downstream ZMQ pushers (if any)
    zmq::context_t zmq_ctx{1};
    auto pushers = create_push_sockets(zmq_ctx, outputs);

    // 3) Launch UDP sender in Asio thread
    asio::io_context ioc;
    std::promise<std::shared_ptr<UdpSender>> promise;
    auto future_sender = promise.get_future();
    auto asio_thread = start_udp_sender(ioc, port, promise);

    // 4) Wait until the UDP socket is ready
    auto sender = future_sender.get();

    // 5) ZMQ loop on main thread: pull → post UDP send + forward downstream
    zmq_receive_loop(zmq_ctx, inputs, [&](const std::string &data){
        // a) Post UDP send into Asio thread
        ioc.post([sender, data]() mutable {
            sender->socket.send_to(asio::buffer(data), sender->endpoint);
        });
        // b) Forward to any downstream ZMQ outputs
        for (auto &p : pushers) {
            p.send(zmq::buffer(data), zmq::send_flags::none);
        }
    });

    // (never reached in this example)
    asio_thread.join();
    return 0;
}
