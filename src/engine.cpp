// src/engine.cpp

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
#include <utils/io_utils.h>
#include <utils/dag_utils.h>

extern char **environ;


static pid_t spawn_node(const std::string &prog,
                        const std::vector<std::string> &args)
{
    // Build a NULL-terminated argv array
    std::vector<char*> argv;
    argv.reserve(args.size() + 2);
    argv.push_back(const_cast<char*>(prog.c_str()));
    for (const auto &a : args)
        argv.push_back(const_cast<char*>(a.c_str()));
    argv.push_back(nullptr);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    if (pid == 0) {
        // In child: replace with the new program
        execvp(prog.c_str(), argv.data());
        perror("execvp failed");
        _exit(1);
    }
    // In parent: return child's PID
    return pid;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: cp_engine <config.json>\n";
        return 1;
    }
    const std::string config_file = argv[1];

    // 1) Parse the JSON configuration and validate the DAG
    std::vector<NodeConfig> nodes;
    std::vector<EdgeConfig> edges;
    try {
        parse_config(config_file, nodes, edges);
        dag::validate_dag(nodes, edges);

    } catch (const std::exception &e) {
        std::cerr << "Error parsing config: " << e.what() << "\n";
        return 1;
    }

    // 2) Auto-generate IPC endpoints and build per-node I/O lists
    std::map<std::string, std::vector<std::string>> inputs, outputs;
    for (const auto &e : edges) {
        std::string ep = "ipc:///tmp/" + e.from + "_to_" + e.to + ".ipc";
        outputs[e.from].push_back(ep);
        inputs[e.to].push_back(ep);
    }

    // 3) Launch each node as a separate process
    std::vector<pid_t> pids;
    for (const auto &node : nodes) {
        std::vector<std::string> args;

        // Constructor arguments
        for (const auto &a : node.args)
            args.push_back(a);

        // --inputs
        if (!inputs[node.id].empty()) {
            args.push_back("--inputs");
            for (const auto &in_ep : inputs[node.id])
                args.push_back(in_ep);
        }

        // --outputs 
        if (!outputs[node.id].empty()) {
            args.push_back("--outputs");
            for (const auto &out_ep : outputs[node.id])
                args.push_back(out_ep);
        }

        pid_t pid = spawn_node(node.type, args);
        if (pid > 0) {
            pids.push_back(pid);
        } else {
            std::cerr << "Failed to launch node: " << node.type << "\n";
        }
    }

    // 4) Wait for all child processes to finish
    for (pid_t pid : pids) {
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}
