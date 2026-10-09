# Build environment (LLVM/Clang 14)

The checker is pinned to LLVM/Clang 14 (`find_package(LLVM 14 ...)`).

## Docker (recommended, reproducible)

```bash
docker build -t misra-checker .   # configures, builds
docker run --rm misra-checker     # runs the test suite
```

The image is based on Ubuntu 22.04, which ships `llvm-14-dev`/`libclang-14-dev`.

## Without Docker

- Ubuntu 22.04: `apt-get install clang-14 llvm-14-dev libclang-14-dev libclang-cpp14-dev cmake ninja-build`
- Ubuntu 24.04 has no LLVM 14 packages. Either use the Docker image or extract
  the jammy packages (`libllvm14 llvm-14 llvm-14-dev llvm-14-linker-tools
  llvm-14-runtime llvm-14-tools libclang-14-dev libclang-common-14-dev
  libclang-cpp14 libclang-cpp14-dev libclang1-14 clang-14 libffi7 libpolly-14-dev`)
  with `dpkg -x` into `/`, then configure with
  `-DLLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm`.

## Cloud session container

A session container resets between sessions. To get LLVM 14 automatically, put
the jammy extraction steps above in the environment's setup script (see
Claude Code environment settings), or run the build inside Docker where a
daemon is available.
