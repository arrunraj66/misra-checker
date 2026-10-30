#!/usr/bin/env python3
"""AI provider for `misra-checker convert`: Claude API over HTTPS.

Reads the prompt on stdin and writes the model reply on stdout.
Environment: ANTHROPIC_API_KEY (required), MISRA_AI_MODEL (default below),
ANTHROPIC_BASE_URL (optional, e.g. an approved internal gateway).

The reply is untrusted input; the converter's deterministic gates decide
whether it is accepted. Do not send proprietary code to a cloud endpoint
without customer approval - use an on-prem provider instead (see
local_provider.sh).
"""
import json
import os
import sys
import urllib.request

MODEL = os.environ.get("MISRA_AI_MODEL", "claude-sonnet-5-5")
BASE = os.environ.get("ANTHROPIC_BASE_URL", "https://api.anthropic.com")


def main() -> int:
    key = os.environ.get("ANTHROPIC_API_KEY")
    if not key:
        print("ANTHROPIC_API_KEY is not set", file=sys.stderr)
        return 2
    body = json.dumps({
        "model": MODEL,
        "max_tokens": 16000,
        "messages": [{"role": "user", "content": sys.stdin.read()}],
    }).encode()
    request = urllib.request.Request(
        BASE + "/v1/messages", data=body, method="POST",
        headers={"x-api-key": key, "anthropic-version": "2023-06-01",
                 "content-type": "application/json"})
    with urllib.request.urlopen(request, timeout=300) as response:
        reply = json.load(response)
    sys.stdout.write("".join(
        block.get("text", "") for block in reply.get("content", [])))
    return 0


if __name__ == "__main__":
    sys.exit(main())
