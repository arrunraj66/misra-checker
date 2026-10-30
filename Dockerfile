# Reproducible build/test environment pinned to LLVM/Clang 14 (the project's
# supported frontend). Build:  docker build -t misra-checker .
# Run tests:                   docker run --rm misra-checker
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build python3 git ca-certificates \
      clang-14 llvm-14-dev libclang-14-dev libclang-cpp14-dev \
 && rm -rf /var/lib/apt/lists/*

# Make `clang` resolve to clang-14 (CMake searches clang-14 first anyway).
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-14 100

WORKDIR /src
COPY . /src
RUN cmake -S . -B build -G Ninja -DLLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm \
 && cmake --build build

CMD ["ctest", "--test-dir", "build", "--output-on-failure"]
