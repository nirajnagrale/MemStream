// include/utils/socket_utils.h

#ifndef UTIL_SOCKET_UTILS_H
#define UTIL_SOCKET_UTILS_H

#include <zmq.hpp>
#include <string>
#include <vector>
#include <iostream>

/**
 * Create and bind PUSH sockets for each endpoint.
 */
inline std::vector<zmq::socket_t> create_push_sockets(zmq::context_t &ctx,
                                                      const std::vector<std::string> &endpoints)
{
    std::vector<zmq::socket_t> sockets;
    sockets.reserve(endpoints.size());
    for (const auto &ep : endpoints) {
        zmq::socket_t sock{ctx, zmq::socket_type::push};
        sock.bind(ep);
        std::cout << "Bound PUSH to " << ep << std::endl;
        sockets.push_back(std::move(sock));
    }
    return sockets;
}

/**
 * Create and connect PULL sockets for each endpoint.
 */
inline std::vector<zmq::socket_t> create_pull_sockets(zmq::context_t &ctx,
                                                      const std::vector<std::string> &endpoints)
{
    std::vector<zmq::socket_t> sockets;
    sockets.reserve(endpoints.size());
    for (const auto &ep : endpoints) {
        zmq::socket_t sock{ctx, zmq::socket_type::pull};
        sock.connect(ep);
        std::cout << "Connected PULL to " << ep << std::endl;
        sockets.push_back(std::move(sock));
    }
    return sockets;
}

#endif // UTIL_SOCKET_UTILS_H
