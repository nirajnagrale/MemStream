## cpp-pipeline
A minimal C++ data‑pipeline engine that chains independent nodes (processes) through ZeroMQ sockets to form a Directed Acyclic Graph (DAG).

RandomGeneratorNode – emits pseudo‑random numbers on demand
TcpSocketNode – republishes data on an available TCP port (default 9001)
UdpSocketNode – republishes data on an available UDP port (default 9002)
controller.py – tells the Random Node which outlet (TCP or UDP) to use at run‑time


# Build & Run
docker build -t cpp-pipeline:2.0.0 .

2 – Copy the package out of the image
CID=$(docker create cpp-pipeline:2.0.0)
docker cp "$CID":/workspace/build/cpp-pipeline-2.0.0-Linux.deb .
docker rm "$CID"

3 – Install on your host (Ubuntu / Debian)
sudo apt install ./cpp-pipeline-2.0.0-Linux.deb
or: sudo dpkg -i cpp-pipeline-2.0.0-Linux.deb && sudo apt -f install

Run:cp_engine /etc/cpp-pipeline/config.json




