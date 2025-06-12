#include <nlohmann/json.hpp>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

using json = nlohmann::json;

// Simple struct to hold one node’s info
struct Node {
    std::string type;
    std::vector<std::string> args;
    pid_t pid = -1;
};

int main(int argc, char** argv) {
    const char* cfgPath = (argc > 1) ? argv[1] : "config/example.json";
    std::ifstream in(cfgPath);
    if (!in) {
        std::cerr << "Failed to open config: " << cfgPath << "\n";
        return 1;
    }

    json cfg = json::parse(in);
    auto jnodes = cfg["nodes"];
    size_t n = jnodes.size();
    if (n < 2) {
        std::cerr << "Need at least 2 nodes\n";
        return 1;
    }

    // Build Node vector
    std::vector<Node> nodes;
    for (auto& jn : jnodes) {
        Node node;
        node.type = jn["type"].get<std::string>();
        for (auto& a : jn["args"])
            node.args.push_back(a.dump()); // stringify each arg
        nodes.push_back(std::move(node));
    }

    // Create pipes (n-1 of them)
    std::vector<std::array<int,2>> pipes(n-1);
    for (size_t i = 0; i < n-1; ++i) {
        if (pipe(pipes[i].data()) == -1) {
            perror("pipe");
            return 1;
        }
    }

    // Fork each node
    for (size_t i = 0; i < n; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            // Child

            // If not first: connect stdin to read-end of previous pipe
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            // If not last: connect stdout to write-end of this pipe
            if (i < n-1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close all pipe fds in child
            for (auto& p : pipes) {
                close(p[0]);
                close(p[1]);
            }

            // Build argv array for exec
            std::vector<char*> exec_argv;
            exec_argv.push_back(const_cast<char*>(nodes[i].type.c_str()));
            for (auto& s : nodes[i].args)
                exec_argv.push_back(const_cast<char*>(s.c_str()));
            exec_argv.push_back(nullptr);

            execvp(nodes[i].type.c_str(), exec_argv.data());
            // If execvp returns, it failed:
            perror(("exec " + nodes[i].type).c_str());
            _exit(1);
        }
        // Parent
        nodes[i].pid = pid;
    }

    // Parent closes all pipe fds
    for (auto& p : pipes) {
        close(p[0]);
        close(p[1]);
    }

    // Wait for all children
    int status = 0;
    for (auto& node : nodes) {
        waitpid(node.pid, &status, 0);
    }
    return 0;
}
