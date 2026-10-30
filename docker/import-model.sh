#!/usr/bin/env bash
# Register a fine-tuned GGUF (e.g. exported from Colab) with the Ollama container.
#   docker/import-model.sh models/misra-fixer-q4_k_m.gguf [model-name]
set -euo pipefail
gguf="${1:?usage: $0 path/to/model.gguf [name]}"
name="${2:-misra-fixer}"
[ -f "$gguf" ] || { echo "no such file: $gguf" >&2; exit 1; }
mkdir -p models
[ "$(dirname "$(realpath "$gguf")")" = "$(realpath models)" ] || cp "$gguf" models/
file="$(basename "$gguf")"
docker compose --profile llm up -d llm
docker compose exec -T llm sh -c "printf 'FROM /models/%s\nPARAMETER temperature 0\nPARAMETER num_ctx 16384\n' '$file' > /tmp/Modelfile && ollama create '$name' -f /tmp/Modelfile"
echo "Model '$name' ready. Test: docker compose exec llm ollama run $name 'int x = 010;'"
