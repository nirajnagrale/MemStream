FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /workspace

# ── 1. system packages we DO have ─────────────────────────────────────────
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential cmake pkg-config \
        dpkg-dev fakeroot \
        nlohmann-json3-dev \
        libboost-system-dev \
        libzmq3-dev \
        git ca-certificates && \
    rm -rf /var/lib/apt/lists/*

# ── 2. build-install cppzmq (header-only) ─────────────────────────────────
RUN git clone --depth 1 https://github.com/zeromq/cppzmq.git && \
    cmake -S cppzmq -B cppzmq/build -DCPPZMQ_BUILD_TESTS=OFF \
          -DCMAKE_INSTALL_PREFIX=/usr && \
    cmake --install cppzmq/build && \
    rm -rf cppzmq

# ── 3. copy source and build project ─────────────────────────────────────
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build && \
    (cd build && cpack -G DEB)

# ── 4. show resulting .deb when container runs ───────────────────────────
CMD ["bash", "-lc", "ls -lh /workspace/build/*.deb"]
