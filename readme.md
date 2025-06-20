# cpp-pipeline
A minimal C++ data‑pipeline engine that chains independent nodes (processes) through ZeroMQ sockets to form a Directed Acyclic Graph (DAG).

RandomGeneratorNode – emits pseudo‑random numbers on demand
TcpSocketNode – republishes data on an available TCP port (default 9001)
UdpSocketNode – republishes data on an available UDP port (default 9002)
controller.py – tells the Random Node which outlet (TCP or UDP) to use at run‑time


# Build & Run

1- docker build -t cpp-pipeline:2.0.0 .

2 – Copy the package out of the image

CID=$(docker create cpp-pipeline:2.0.0)
docker cp "$CID":/workspace/build/cpp-pipeline-2.0.0-Linux.deb .
docker rm "$CID"

3 – Install on your host (Ubuntu / Debian)

sudo apt install ./cpp-pipeline-2.0.0-Linux.deb
or: sudo dpkg -i cpp-pipeline-2.0.0-Linux.deb && sudo apt -f install

Run:cp_engine /etc/cpp-pipeline/config.json

# Engine
Engine starts the pipeline mentioned in json file and validates DAG and passes --input and --output  parameter which are inputs to the particular nodes


# Nodes
Every node is designed in such a way that the main thread pull and pushes into the socket and worker thread either process and notifies the main thread.


Every 
# helper Scripts
1. controller.py

Tell RandomGeneratorNode to send future samples either to the TCP or the UDP outlet.

run: python3 controller.py TCP|UDP|BOTH

2. udp_client.py

connects with UdpSocket node to receive the data from the port mentioned in config.json. Also change the port in udp_client.py file accordingly

run: python3 udp_client.py 

3. ws_client.py

connects with TcpSocket node to receive the data from the port mentioned in config.json. Also change the port in ws_client.py file accordingly

run: python3 ws_client.py 







