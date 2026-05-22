# syntax=docker/dockerfile:1.5

ARG TARGET_EXECUTABLE
FROM ubuntu:24.04 AS builder
ARG TARGET_EXECUTABLE
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        pkg-config \
        libzmq3-dev \
        libprotobuf-dev \
        protobuf-compiler \
        libsdl2-dev \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --target ${TARGET_EXECUTABLE} -- -j$(nproc)

FROM ubuntu:24.04 AS runtime
ARG TARGET_EXECUTABLE
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libzmq5 \
        libprotobuf32 \
        libsdl2-2.0-0 \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /workspace/build/${TARGET_EXECUTABLE} ./

RUN printf '#!/bin/sh\nexec "/app/%s" "$@"\n' "${TARGET_EXECUTABLE}" > /app/entrypoint.sh \
    && chmod +x /app/entrypoint.sh

ENTRYPOINT ["/app/entrypoint.sh"]
