# Controlled AI converter

`misra-checker convert` proposes source fixes with an AI model and accepts a
proposal only if deterministic gates pass. The AI output is untrusted input.

## Flow

1. Analyze the file; stop if there is no finding in a supported fix class.
2. Build a prompt (rule keys, strategy, source; no MISRA wording) and pipe it
   to the operator-supplied `--provider-cmd`.
3. Trial the proposal in place (original always restored on failure).
4. Gates: (a) the file still compiles with its exact compile_commands flags;
   (b) the targeted findings are gone; (c) no new or increased findings;
   (d) optional `--verify-cmd` (build/tests) exits 0.
5. Write `<out>/<file>.proposed`, `.patch`, and an `audit.jsonl` record
   (outcome, finding counts, SHA-256 before/after). Source is left unchanged.

## Safety policy

- Suggestion mode is the default. `--apply` is refused unless the fix class has
  `auto_apply_approved` set, which requires independent validation (roadmap
  W44/W70). No class is approved today.
- Fix classes (none auto-apply approved); without `--fix-class <id>` the first
  class that matches a finding is used, in this order:
  `literal-hygiene` (7.1, 7.2, 7.3), `goto-elimination` (15.1-15.3),
  `structured-control-flow` (14.2, 15.5-15.7, 16.3-16.6),
  `boolean-and-side-effects` (12.3, 13.4-13.6, 14.4),
  `call-and-parameter-hygiene` (2.7, 17.7, 17.8),
  `declaration-hygiene` (8.2, 8.8, 8.10, 8.11, 8.14),
  `pointer-cast-hygiene` (7.4, 11.5, 11.8, 11.9).
  The prompt lists each finding as `key@line:column`.
- Gates prove "compiles, findings reduced"; they do NOT prove behavioral
  equivalence. Run `--verify-cmd` with your real tests and review every patch.

## Providers

- Cloud: `scripts/ai_providers/claude_provider.py` (needs `ANTHROPIC_API_KEY`;
  sends source code to the API - get customer approval first).
- On-prem: `scripts/ai_providers/local_provider.sh` with `MISRA_LOCAL_MODEL_CMD`.
- Offline, no model: `scripts/ai_providers/deterministic_provider.py` rewrites
  the integer literals flagged by Rules 7.1, 7.2 and 7.3 (octal to decimal,
  add `U`, uppercase `L`) using the reported line and column. It is also the
  reference for the provider protocol (prompt on stdin, whole file on stdout).

```bash
misra-checker convert --compile-commands build/ --file src/foo.c \
  --provider-cmd scripts/ai_providers/claude_provider.py \
  --verify-cmd "make -C build test" --output-dir out --license acme.lic
```

## Licensing

- Product licence: proprietary (`LICENSE`, placeholder text - needs legal review).
- Runtime: `convert` requires a signed licence file (`--license` or
  `MISRA_LICENSE_FILE`) with the `convert` feature; exit code 77 otherwise.
  Issue with `scripts/issue_license.py`.
- Build release binaries with `-DMISRA_LICENSE_HMAC_KEY=<secret>`. Limitation:
  HMAC is symmetric, so the key lives in the binary and a determined attacker
  can extract it. For commercial release move to asymmetric signatures
  (e.g. Ed25519, public key only in the binary) and add hardware/host binding.
- MISRA guideline text needs its own licence from the MISRA consortium.
