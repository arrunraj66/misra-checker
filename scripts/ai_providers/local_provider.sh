#!/usr/bin/env bash
# Offline/on-prem provider: pipes the prompt to a local model command.
# Set MISRA_LOCAL_MODEL_CMD, e.g. "ollama run my-code-model".
# Source code never leaves the machine.
set -euo pipefail
: "${MISRA_LOCAL_MODEL_CMD:?set MISRA_LOCAL_MODEL_CMD to a command reading stdin}"
exec bash -c "${MISRA_LOCAL_MODEL_CMD}"
