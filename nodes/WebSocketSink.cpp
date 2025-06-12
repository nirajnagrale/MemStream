#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <optional>
#include <atomic>

using json   = nlohmann::json;
using server = websocketpp::server<websocketpp::config::asio>;
using namespace std;

int main(int argc, char** argv) {
    // Pick port from argv[1], or default to 9002
    int port = (argc > 1 ? std::stoi(argv[1]) : 9002);

    server ws;
    atomic<bool> has_client{false};
    mutex client_mutex;
    optional<websocketpp::connection_hdl> client_hdl;

    ws.init_asio();
    ws.set_reuse_addr(true);

    ws.set_open_handler([&](websocketpp::connection_hdl h) {
        lock_guard lock(client_mutex);
        client_hdl = h;
        has_client = true;
        std::cout << "Client connected\n";
    });
    ws.set_close_handler([&](websocketpp::connection_hdl) {
        lock_guard lock(client_mutex);
        client_hdl.reset();
        has_client = false;
        std::cout << "Client disconnected\n";
    });

   
    try {
        ws.listen(port);
    } catch (const std::exception &e) {
        std::cerr << "ERROR: could not bind to port " 
                  << port << ": " << e.what() << "\n";
        return 1;
    }
    ws.start_accept();

   
    std::thread([&]{ ws.run(); }).detach();

    std::cout << "WebSocketSink listening on port " << port << "\n";

    double sample;
    while (cin.read(reinterpret_cast<char*>(&sample), sizeof(sample))) {
        if (!has_client) continue;
        json msg = {{"value", sample}};
        std::string payload = msg.dump();
        std::lock_guard lock(client_mutex);
        if (client_hdl) {
            ws.send(*client_hdl, payload, websocketpp::frame::opcode::text);
        }
    }

    return 0;
}
