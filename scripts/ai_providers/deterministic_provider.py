#!/usr/bin/env python3
"""Offline provider for `misra-checker convert`: rule-based literal fixes.

Reads the converter prompt on stdin and prints the full corrected file. It
needs no model and sends nothing anywhere. It only rewrites integer literals
that the checker pointed at (by line and column):

  misra-c2012-7.1  octal constant        -> same value in decimal
  misra-c2012-7.2  unsigned without U    -> adds a U suffix
  misra-c2012-7.3  lowercase l suffix    -> uppercase L

Findings it cannot fix (for example inside macro expansions, where the
reported column is the macro use) are left untouched; the converter's gates
then decide whether the result is accepted. Use it as a reference
implementation of the provider protocol and as a safe default for these rules.
"""
import re
import sys

BEGIN = "----- BEGIN FILE -----\n"
END = "----- END FILE -----"
LITERAL = re.compile(
    r"(0[xX][0-9a-fA-F]+|[0-9]+)([uUlL]*)")


def main() -> int:
    prompt = sys.stdin.read()
    start = prompt.index(BEGIN) + len(BEGIN)
    stop = prompt.rindex(END)
    source = prompt[start:stop]
    findings = re.findall(r"^- (misra-c2012-7\.[123])-[^@\s]*@(\d+):(\d+)$",
                          prompt[:prompt.index(BEGIN)], re.M)
    lines = source.split("\n")
    # Right to left so earlier columns stay valid.
    for rule, line_no, column in sorted(
            findings, key=lambda f: (int(f[1]), -int(f[2]))):
        index, col = int(line_no) - 1, int(column) - 1
        if index >= len(lines):
            continue
        text = lines[index]
        match = LITERAL.match(text, col)
        if not match:
            continue
        digits, suffix = match.group(1), match.group(2)
        if rule == "misra-c2012-7.3":
            suffix = suffix.replace("l", "L")
        elif rule == "misra-c2012-7.1":
            if not re.fullmatch(r"0[0-7]+", digits):
                continue
            digits = str(int(digits, 8))
        elif rule == "misra-c2012-7.2":
            if "u" in suffix.lower():
                continue
            suffix = "U" + suffix
        lines[index] = text[:col] + digits + suffix + text[match.end():]
    sys.stdout.write("\n".join(lines))
    return 0


if __name__ == "__main__":
    sys.exit(main())
