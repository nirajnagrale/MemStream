# cpp-pipeline

**Minimal, real-time in-memory data-pipeline engine**

## Build & Install

```bash
# Clone repo
git clone https://github.com/nirajnagrale/cpp-pipeline 
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
   # Run : cp_engine /etc/cpp-pipeline/example.json
3. Connect a WebSocket client to `ws://localhost:<port>` ([ws_client.py](#example-configuration)).
## Note: Use ws_client.py to check for data . Run python3 ws_client.py to check the WebSocket data upate the code uri ws://localhost:<port> in ws_client.py

## Packaging (DEB)
# To generate a `.deb` package:
```bash
cpack -G DEB
```
# Install it via:
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

## Run inside container:
```bash
docker run --rm -it \
  -v $(pwd)/config:/workspace/config \
  cpp-pipeline-builder \
  bash -lc "cd build && ./cp_engine ../config/example.json"
```

## Example Configuration
# `/etc/cpp-pipeline/example.json`:
{
  "nodes": [
    { "id": "src",  "type": "RandomSource",   "args": [1000] },
    { "id": "fft",  "type": "FFTTransform",   "args": [256] },
    { "id": "sink", "type": "WebSocketSink",  "args": [9002] } //provide the port number here
  ],
  "edges": [
    { "from": "src", "to": "fft" },
    { "from": "fft", "to": "sink" }
  ]
}

