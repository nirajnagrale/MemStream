# MemStream
A minimal C++ data‑pipeline ver 2.0.0 engine that chains independent nodes (processes) through ZeroMQ sockets to form a Directed Acyclic Graph (DAG).<br>

RandomGeneratorNode – emits pseudo‑random numbers on demand<br>
TcpSocketNode – republishes data on an available TCP port (default 9001)<br>
UdpSocketNode – republishes data on an available UDP port (default 9002)<br>
controller.py – tells the Random Node which outlet (TCP or UDP) to use at run‑time<br>


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
  Engine starts the pipeline mentioned in the JSON file and validates DAG and passes --input and --output  parameters which are inputs to the particular nodes


# Nodes
  Every node is designed so that the main thread pulls and pushes into the socket, and the worker thread processes and notifies the main thread.

# Helper Scripts
1. controller.py<br>
  Tell RandomGeneratorNode to send future samples either to the TCP or the UDP outlet.<br>
  run: python3 controller.py TCP|UDP|BOTH<br>

2. udp_client.py<br>
  connects with the UdpSocket node to receive the data from the port mentioned in config.json. Also, change the port in the  udp_client.py file accordingly<br>
  run: python3 udp_client.py <br>

3. ws_client.py<br>
  connects with the TcpSocket node to receive the data from the port mentioned in config.json. Also, change the port in the  ws_client.py file accordingly<br>
  run: python3 ws_client.py <br>



