
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential \
      cmake \
      dpkg-dev \
      fakeroot \
      pkg-config \
      libfftw3-dev \
      nlohmann-json3-dev \
      libboost-system-dev \
      libwebsocketpp-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . .

RUN mkdir -p build && cd build && \
    cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . && \
    cpack -G DEB

CMD ["bash","-lc","ls -lh /workspace/build/*.deb"]
