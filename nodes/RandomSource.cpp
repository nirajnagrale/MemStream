// nodes/RandomSource.cpp

#include <zmq.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <vector>
#include <string>
#include <iostream>
#include <utils/io_utils.h>
#include <utils/socket_utils.h>

// Initializes and returns a control‐listener thread for RandomSource.
// Listens on control_ep for "TCP", "UDP", or "BOTH" and updates 'mode' (0=TCP,1=UDP,2=BOTH).
static std::thread init_control_listener(zmq::context_t &ctx,
                                         const std::string &control_ep,
                                         std::atomic<int> &mode)
{
    zmq::socket_t sub{ctx, zmq::socket_type::sub};
    sub.connect(control_ep);
    sub.set(zmq::sockopt::subscribe, "");

    return std::thread([sub = std::move(sub), &mode]() mutable {
        zmq::message_t msg;
        while (sub.recv(msg, zmq::recv_flags::none)) {
            std::string s{static_cast<char*>(msg.data()), msg.size()};
            if      (s == "TCP")  mode = 0;
            else if (s == "UDP")  mode = 1;
            else if (s == "BOTH") mode = 2;
            std::cout << "Control: switched to " << s << "\n";
        }
    });
}

int main(int argc, char* argv[]) {
    // Usage: RandomSource <interval_ms> [--inputs <uri>...] --outputs <uri1> <uri2>
    if (argc < 2) {
        std::cerr << "Usage: RandomSource <interval_ms> [--inputs <uri>...] --outputs <uri1> <uri2>\n";
        return 1;
    }

    // 1) Parse generation interval
    int interval_ms = std::stoi(argv[1]);

    // 2) Parse I/O lists
    std::vector<std::string> inputs, outputs;
    parse_io_args(argc, argv, inputs, outputs);

    if (!inputs.empty()) {
        std::cerr << "Warning: RandomSource ignores inputs.\n";
    }
    if (outputs.size() < 2) {
        std::cerr << "Error: need two outputs (TCP then UDP)\n";
        return 1;
    }

    // 3) Create ZMQ context and bind two PUSH sockets
    zmq::context_t ctx{1};
    auto pushers = create_push_sockets(ctx, outputs);
    // pushers[0] → TCP sink, pushers[1] → UDP sink

    // 4) Spawn control‐listener thread
    std::atomic<int> mode{0}; // start in TCP mode
    auto control_thread = init_control_listener(ctx,"ipc:///tmp/control.ipc",mode);

    // 5) Random‐generator setup
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, 100);

    // 6) Main loop: generate & send based on mode
    while (true) {
        int value = dist(rng);
        std::string msg = std::to_string(value);

        int m = mode.load();
        if (m == 0) {
            pushers[0].send(zmq::buffer(msg), zmq::send_flags::none);
        }
        else if (m == 1) {
            pushers[1].send(zmq::buffer(msg), zmq::send_flags::none);
        }
        else if(m>=2)
        {
            for (auto &sock : pushers) 
            {
                sock.send(zmq::buffer(msg), zmq::send_flags::none);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }

    control_thread.join();  // not reached in this infinite loop
    return 0;
}
