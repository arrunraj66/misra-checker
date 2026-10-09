#!/usr/bin/env python3
"""On-prem provider for `misra-checker convert`: a local Ollama server.

Reads the prompt on stdin and writes the model reply on stdout, like the other
providers. Source code only goes to the Ollama host you configure.
Environment: OLLAMA_HOST (default http://127.0.0.1:11434),
MISRA_AI_MODEL (default misra-fixer), MISRA_AI_NUM_CTX (default 16384).

The reply is untrusted input; the converter's deterministic gates decide
whether it is accepted.
"""
import json
import os
import sys
import urllib.error
import urllib.request

HOST = os.environ.get("OLLAMA_HOST", "http://127.0.0.1:11434").rstrip("/")
if not HOST.startswith("http"):
    HOST = "http://" + HOST
MODEL = os.environ.get("MISRA_AI_MODEL", "misra-fixer")
NUM_CTX = int(os.environ.get("MISRA_AI_NUM_CTX", "16384"))


def main() -> int:
    body = json.dumps({
        "model": MODEL,
        "prompt": sys.stdin.read(),
        "stream": False,
        "options": {"temperature": 0, "num_ctx": NUM_CTX},
    }).encode()
    request = urllib.request.Request(
        HOST + "/api/generate", data=body, method="POST",
        headers={"content-type": "application/json"})
    try:
        with urllib.request.urlopen(request, timeout=900) as response:
            reply = json.load(response)
    except urllib.error.URLError as exc:
        print(f"cannot reach Ollama at {HOST}: {exc}", file=sys.stderr)
        return 2
    if "error" in reply:
        print(f"ollama: {reply['error']}", file=sys.stderr)
        return 2
    sys.stdout.write(reply.get("response", ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
