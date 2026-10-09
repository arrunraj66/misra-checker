#!/usr/bin/env bash
# Package the app image as a single file you can copy to another PC.
#   docker/export-image.sh            -> dist/misra-checker-app.tar.gz
#   docker/export-image.sh --with-llm -> also dist/ollama-rocm.tar.gz (several GB)
# On the target PC:  docker load < misra-checker-app.tar.gz
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p dist
docker compose build misra
docker save misra-checker-app:latest | gzip > dist/misra-checker-app.tar.gz
if [ "${1:-}" = "--with-llm" ]; then
  docker pull ollama/ollama:rocm
  docker save ollama/ollama:rocm | gzip > dist/ollama-rocm.tar.gz
fi
cp docker-compose.yml dist/
ls -lh dist
