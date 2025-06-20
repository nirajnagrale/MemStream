// include/utils/dag_utils.h
#ifndef DAG_UTILS_H
#define DAG_UTILS_H

#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <vector>
#include <string>

#include "io_utils.h"   // NodeConfig, EdgeConfig

namespace dag {

/// validate_dag(nodes, edges) throws std::runtime_error on any error
inline void validate_dag(const std::vector<NodeConfig> &nodes,
                         const std::vector<EdgeConfig> &edges)
{
    // 1) unique node ids, and build a quick lookup
    std::unordered_map<std::string, std::vector<std::string>> adj;
    for (const auto &n : nodes) {
        if (adj.count(n.id))
            throw std::runtime_error("Duplicate node id: " + n.id);
        adj[n.id] = {};                    // initialise adjacency list
    }

    // 2) add directed edges, checking that endpoints exist
    for (const auto &e : edges) {
        if (!adj.count(e.from))
            throw std::runtime_error("Edge refers to unknown node: " + e.from);
        if (!adj.count(e.to))
            throw std::runtime_error("Edge refers to unknown node: " + e.to);
        adj[e.from].push_back(e.to);
    }

    // 3) depth-first search for cycles
    std::unordered_set<std::string> visited, on_stack;

    std::function<void(const std::string&)> dfs = [&](const std::string &v) {
        visited.insert(v);
        on_stack.insert(v);

        for (const auto &nbr : adj[v]) {
            if (!visited.count(nbr))
                dfs(nbr);
            else if (on_stack.count(nbr))
                throw std::runtime_error("Graph is not a DAG (cycle detected)");
        }
        on_stack.erase(v);
    };

    for (const auto &kv : adj)
        if (!visited.count(kv.first))
            dfs(kv.first);
}

} // namespace dag
#endif
