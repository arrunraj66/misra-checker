#!/usr/bin/env python3
"""Collect verified (prompt -> fixed file) pairs from `misra-checker convert` runs.

A converter output directory holds, per source file, `<name>.prompt.txt` (the
exact prompt sent to the model), `<name>.proposed` (the reply that passed every
gate) and `audit.jsonl`. Only runs whose last audit record says outcome
"proposed" and whose finding count went down are exported, so the data contains
fixes the checker (and your --verify-cmd) accepted. Review them before training:
the gates do not prove behavioural equivalence.

    build_training_data.py OUT_DIR [OUT_DIR ...] --dataset train.jsonl [--eval eval.jsonl]

Re-running appends only new pairs (deduplicated by hash). Output lines use the
chat format {"messages": [{"role": "user", ...}, {"role": "assistant", ...}]}.
"""
import argparse
import hashlib
import json
import random
import sys
from pathlib import Path

BEGIN, END = "----- BEGIN FILE -----\n", "----- END FILE -----"


def reply_text(raw: str) -> str:
    """The converter accepts a bare file or one fenced block; store the bare file."""
    text = raw.strip("\n")
    if text.startswith("```"):
        lines = text.split("\n")
        if lines[-1].strip() == "```":
            text = "\n".join(lines[1:-1])
    return text + "\n"


def pairs_from(out_dir: Path):
    audit = out_dir / "audit.jsonl"
    if not audit.is_file():
        print(f"skip {out_dir}: no audit.jsonl", file=sys.stderr)
        return
    last = {}
    for line in audit.read_text().splitlines():
        if line.strip():
            record = json.loads(line)
            last[Path(record["file"]).name] = record
    for name, record in last.items():
        prompt_file = out_dir / f"{name}.prompt.txt"
        proposed = out_dir / f"{name}.proposed"
        if (record.get("outcome") != "proposed"
                or record["findings_after"] >= record["findings_before"]
                or not prompt_file.is_file() or not proposed.is_file()):
            continue
        prompt = prompt_file.read_text()
        if BEGIN not in prompt or END not in prompt:
            continue
        yield {"messages": [{"role": "user", "content": prompt},
                            {"role": "assistant", "content": reply_text(proposed.read_text())}],
               "meta": {"fix_class": record.get("fix_class"),
                        "findings_before": record["findings_before"],
                        "findings_after": record["findings_after"]}}


def key(example: dict) -> str:
    m = example["messages"]
    return hashlib.sha256((m[0]["content"] + "\0" + m[1]["content"]).encode()).hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("out_dirs", nargs="+", type=Path)
    parser.add_argument("--dataset", type=Path, required=True)
    parser.add_argument("--eval", type=Path, help="hold out a share of NEW pairs here")
    parser.add_argument("--eval-fraction", type=float, default=0.15)
    parser.add_argument("--seed", type=int, default=1)
    args = parser.parse_args()

    seen = set()
    for path in (args.dataset, args.eval):
        if path and path.is_file():
            seen |= {key(json.loads(line)) for line in path.read_text().splitlines() if line.strip()}
    fresh = []
    for out_dir in args.out_dirs:
        for example in pairs_from(out_dir):
            if key(example) not in seen:
                seen.add(key(example))
                fresh.append(example)
    random.Random(args.seed).shuffle(fresh)
    held = int(len(fresh) * args.eval_fraction) if args.eval else 0
    for path, items in ((args.eval, fresh[:held]), (args.dataset, fresh[held:])):
        if path and items:
            with path.open("a") as handle:
                for item in items:
                    handle.write(json.dumps(item) + "\n")
    print(f"{len(fresh)} new verified pair(s): {len(fresh) - held} train, {held} eval")
    return 0


if __name__ == "__main__":
    sys.exit(main())
