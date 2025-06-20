// nodes/TCPWebSocket.cpp

#include <zmq.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <future>
#include <utils/io_utils.h>
#include <utils/socket_utils.h>
#include <chrono>

namespace asio      = boost::asio;
namespace beast     = boost::beast;
namespace websocket = beast::websocket;
using tcp           = asio::ip::tcp;

// Shared pointer to the WebSocket stream
using WsStream    = websocket::stream<tcp::socket>;
using WsStreamPtr = std::shared_ptr<WsStream>;

/**
 * Starts a thread that:
 *  1) Accepts a TCP connection on `port` using `ioc`,
 *  2) Upgrades it to a WebSocket,
 *  3) Fulfills `ws_promise` with the ready WebSocket,
 *  4) Runs `ioc.run()` to service all posted write() calls.
 */
static std::thread start_websocket_thread(asio::io_context &ioc,
                                          unsigned short port,
                                          std::promise<WsStreamPtr> &ws_promise)
{
    return std::thread([&ioc, port, &ws_promise]() {
        tcp::acceptor acceptor{ioc, {tcp::v4(), port}};
        std::cout << "TCPWebSocket listening on port " << port << "\n";
        tcp::socket socket{ioc};
        acceptor.accept(socket);

        auto ws = std::make_shared<WsStream>(std::move(socket));
        ws->accept();
        ws->text(true); 
        std::cout << "WebSocket connected on port " << port << "\n";

        ws_promise.set_value(ws);
        auto work_guard = asio::make_work_guard(ioc);
        ioc.run();
    });
}

/**
 * Generic ZeroMQ receive loop:
 *  - Connects PULL sockets to each URI in `inputs`
 *  - Polls them forever
 *  - Invokes `handler(data)` on each received message
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
        zmq::poll(items, std::chrono::milliseconds::max());
        for (size_t i = 0; i < items.size(); ++i) {
            if (items[i].revents & ZMQ_POLLIN) {
                zmq::message_t msg;
                auto rc = pullers[i].recv(msg, zmq::recv_flags::none);
                if(!rc) continue;
                std::string data{static_cast<char*>(msg.data()), msg.size()};
               std::cout << "[TCP-BRIDGE] pulled " << data << '\n';
                handler(data);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // Usage: TCPWebSocket <port> --inputs <uri1> [uri2...] [--outputs <uriA>...]
    if (argc < 3) {
        std::cerr << "Usage: TCPWebSocket <port> --inputs <uri1> [uri2...] [--outputs <uriA>...]\n";
        return 1;
    }

    // 1) Parse port and I/O endpoints
    unsigned short port = static_cast<unsigned short>(std::stoi(argv[1]));
    std::vector<std::string> inputs, outputs;
    parse_io_args(argc, argv, inputs, outputs);
    if (inputs.empty()) {
        std::cerr << "Error: no --inputs endpoints\n";
        return 1;
    }

    // ── DEBUG ───────────────────────────────────────────────
    std::cout << "[TCP] argv[0] = " << argv[0] << '\n';
    std::cout << "[TCP] inputs:";
    for (auto &s : inputs)  std::cout << ' ' << s;
    std::cout << "   outputs:";
    for (auto &s : outputs) std::cout << ' ' << s;
    std::cout << std::endl;
        
    // 2) Prepare downstream ZMQ PUSH sockets (may be empty)
    zmq::context_t zmq_ctx{1};
    auto pushers = create_push_sockets(zmq_ctx, outputs);

    // 3) Launch WebSocket accept+handshake in its own thread
    asio::io_context ioc;
    std::promise<WsStreamPtr> ws_promise;
    auto ws_future = ws_promise.get_future();
    auto ws_thread = start_websocket_thread(ioc, port, ws_promise);

    // 4) Wait until WebSocket is ready
    WsStreamPtr ws = ws_future.get();

    // 5) Enter the ZMQ receive loop on the main thread
    zmq_receive_loop(zmq_ctx, inputs, [&](const std::string &data) {
        // a) Post WebSocket write to the Asio thread
        //    (this is safe because ws is shared_ptr)
        ioc.post([ws, data]() mutable {
        std::cout << "[TCP-BRIDGE]  →WS  " << data << '\n';
        ws->async_write(asio::buffer(data), 
            [](boost::beast::error_code ec, std::size_t bytes) {
                if (ec) {
                    std::cout << "[TCP-BRIDGE] Async write error: " << ec.message() << '\n';
                }
            });
    });
        // b) Forward to any downstream ZMQ nodes
        for (auto &p : pushers) {
            p.send(zmq::buffer(data), zmq::send_flags::none);
        }
    });

    ws_thread.join();
    return 0;
}
