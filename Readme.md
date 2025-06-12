# cpp-pipeline

**Minimal, real-time in-memory data-pipeline engine**

This project provides a Kafka-style, UNIX-domain-socket based C++ pipeline engine (`cp_engine`) that wires independent node processes together, plus three example nodes:

* **RandomSource**: continuously emits random samples at a configurable rate.
* **FFTTransform**: consumes fixed-size windows of samples, computes an FFT via FFTW, forwards magnitudes.
* **WebSocketSink**: streams incoming data over a WebSocket endpoint for real-time visualization.

## Table of Contents

* [Prerequisites](#prerequisites)
* [Build & Install](#build--install)
* [Running the Pipeline](#running-the-pipeline)
* [Packaging (DEB)](#packaging-deb)
* [Docker Build](#docker-build)
* [Example Configuration](#example-configuration)
* [Testing](#testing)
* [Project Layout](#project-layout)
* [License](#license)

## Prerequisites

Choose one of the following approaches:

### 1 Native Build (on Ubuntu 22.04)

Install the required tools and libraries on your host:

```bash
sudo apt update
sudo apt install -y \
    build-essential cmake dpkg-dev fakeroot \
    pkg-config libfftw3-dev nlohmann-json3-dev \
    libboost-system-dev libwebsocketpp-dev
```

### 2 Docker Build (no host dependencies)


## Build & Install

```bash
# Clone repo
git clone <your-repo-url> cpp-pipeline
cd cpp-pipeline

# Out-of-source build
mkdir build && cd build
cmake -S .. -B .
cmake --build .

# Install binaries + config
sudo cmake --install .
```

## Running the Pipeline

1. Edit `/etc/cpp-pipeline/example.json` to set the WebSocket port (e.g. `9002`).
2. Launch:

   ```bash
   cp_engine /etc/cpp-pipeline/example.json
   ```
3. Connect a WebSocket client to `ws://localhost:<port>` (see [Example Configuration](#example-configuration)).

## Packaging (DEB)

To generate a `.deb` package:

```bash
# from build/ directory
cpack -G DEB
```

Install it via:

```bash
sudo dpkg -i cpp-pipeline-1.0.0-Linux.deb
sudo apt-get install -f    
```

## Docker Build

Reproduce the build + packaging in Docker:
```bash
docker build -t cpp-pipeline-builder .
# Extract .deb
CID=$(docker create cpp-pipeline-builder)
docker cp $CID:/workspace/build/cpp-pipeline-1.0.0-Linux.deb .
docker rm $CID
```

Run inside container:

```bash
docker run --rm -it \
  -v $(pwd)/config:/workspace/config \
  cpp-pipeline-builder \
  bash -lc "cd build && ./cp_engine ../config/example.json"
```

## Example Configuration
`/etc/cpp-pipeline/example.json`:
```json
{
  "nodes": [
    { "id": "src",  "type": "RandomSource",   "args": [1000] },
    { "id": "fft",  "type": "FFTTransform",   "args": [256] },
    { "id": "sink", "type": "WebSocketSink",  "args": [9002] }
  ],
  "edges": [
    { "from": "src", "to": "fft" },
    { "from": "fft", "to": "sink" }
  ]
}
```