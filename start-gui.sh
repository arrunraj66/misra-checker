#!/usr/bin/env bash
# Start the MISRA Checker web GUI on this machine (no Docker).
#   ./start-gui.sh            build the checker if needed, then open http://127.0.0.1:8765
#   ./start-gui.sh --port 9000 --no-browser     (extra options go to gui/misra_gui.py)
# Users are stored in ~/.misra-gui (create one:  python3 gui/misra_gui.py --add-user NAME --role admin).
set -euo pipefail
cd "$(dirname "$0")"

need() { command -v "$1" >/dev/null 2>&1 || { echo "missing: $1 ($2)" >&2; exit 1; }; }
need python3 "sudo apt install python3"

if [ ! -x build/misra-checker ]; then
  echo "First run: building the checker (a few minutes)..."
  need cmake "sudo apt install cmake"
  need ninja "sudo apt install ninja-build"
  if [ ! -d /usr/lib/llvm-14 ]; then
    echo "LLVM/Clang 14 is required: sudo apt install clang-14 llvm-14-dev libclang-14-dev libclang-cpp14-dev" >&2
    exit 1
  fi
  cmake -S . -B build -G Ninja -DLLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm
  cmake --build build --target misra-checker
fi

command -v cppcheck >/dev/null 2>&1 || \
  echo "note: cppcheck not found, the 'Cross-check with cppcheck' option will be disabled (sudo apt install cppcheck)"

exec python3 gui/misra_gui.py "$@"
