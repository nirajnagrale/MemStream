// include/utils/io_utils.h

#ifndef UTIL_IO_UTILS_H
#define UTIL_IO_UTILS_H

#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

using json = nlohmann::json;

// Data structures for engine configuration
struct NodeConfig {
    std::string id;
    std::string type;
    std::vector<std::string> args;  // constructor arguments
};

struct EdgeConfig {
    std::string from;
    std::string to;
};
inline void parse_config(const std::string &file,
                         std::vector<NodeConfig> &nodes,
                         std::vector<EdgeConfig> &edges)
{
    std::ifstream in(file);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open config file: " + file);
    }

    json j;
    in >> j;

    for (auto &jn : j.at("nodes")) {
        NodeConfig n;
        n.id   = jn.at("id").get<std::string>();
        n.type = jn.at("type").get<std::string>();
        for (auto &a : jn.at("args")) {
            if (a.is_string())      n.args.push_back(a.get<std::string>());
            else                    n.args.push_back(a.dump());
        }
        nodes.push_back(std::move(n));
    }

    for (auto &je : j.at("edges")) {
        edges.push_back({
            je.at("from").get<std::string>(),
            je.at("to").get<std::string>()
        });
    }
}

inline void parse_io_args(int argc, char* argv[],
                          std::vector<std::string> &inputs,
                          std::vector<std::string> &outputs)
{
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--inputs") {
            ++i;
            while (i < argc && argv[i][0] != '-') {
                inputs.emplace_back(argv[i++]);
            }
            --i;
        }
        else if (arg == "--outputs") {
            ++i;
            while (i < argc && argv[i][0] != '-') {
                outputs.emplace_back(argv[i++]);
            }
            --i;
        }
    }
}

#endif // UTIL_IO_UTILS_H
