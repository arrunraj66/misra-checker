# Runtime image: misra-checker + web GUI (+ CLI). Build from the repo root:
#   docker build -f docker/app.Dockerfile -t misra-checker-app .
# The test/dev image is the top-level Dockerfile.

FROM ubuntu:22.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build python3 ca-certificates \
      clang-14 llvm-14-dev libclang-14-dev libclang-cpp14-dev \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . /src
RUN cmake -S . -B build -G Ninja -DLLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm \
 && cmake --build build --target misra-checker

FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
# clang-14 supplies the compiler and builtin headers used in compile databases;
# libclang-cpp14 is the shared library misra-checker links against.
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      clang-14 libclang-cpp14 libc6-dev python3 ca-certificates \
 && rm -rf /var/lib/apt/lists/* \
 && useradd --create-home --uid 1000 misra
WORKDIR /opt/misra
COPY --from=build /src/build/misra-checker build/misra-checker
COPY gui gui
COPY scripts scripts
COPY docs/rule-implementation-status.md docs/rule-implementation-status.md
COPY tests/fixtures tests/fixtures
RUN ln -s /opt/misra/build/misra-checker /usr/local/bin/misra-checker \
 && mkdir -p /workspace /data && chown misra:misra /workspace /data
ENV MISRA_GUI_DATA=/data
USER misra
WORKDIR /workspace
EXPOSE 8765
HEALTHCHECK --interval=30s --timeout=5s --start-period=10s \
  CMD python3 -c "import urllib.request as u; u.urlopen('http://127.0.0.1:8765/api/rules', timeout=4)"
# Login is on by default. On first start an 'admin' user is created from
# MISRA_GUI_ADMIN_PASSWORD, or with a random password printed once in the log.
# Keep the port on loopback or behind a TLS proxy (see docs/docker.md).
VOLUME ["/data"]
CMD ["python3", "/opt/misra/gui/misra_gui.py", "--host", "0.0.0.0", "--no-browser", "--db-root", "/workspace"]
