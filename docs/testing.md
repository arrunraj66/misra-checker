# Testing

`ctest` (from `build/`) runs everything; about 340 tests, a few seconds each at most.

| Group | What it checks |
|---|---|
| `obs_*`, `sys_*`, `rule_*` | every rule against its violating and compliant fixtures |
| `triage_cases` | replayed labelled cases from `tests/triage` (a fixed case that regresses fails) |
| `report`, `baseline`, `compliance`, `history`, `cppcheck_cross`, `compiler_profiles` | the Python libraries and the checker's SARIF, deviation and CI-gate behaviour |
| `gui` | the server over HTTP: login, roles, CSRF, projects, editing, triage, baselines, history, tokens, reports |
| `gui_ui_smoke` | the page's script in a fake page under `gjs` (fast; needs `gjs`) |
| `e2e_browser` | the real page in Chromium: login, analysis, editor markers, export menu, triage, baselines, compare, zip project editing, publish, history chart, theme, narrow window, logout, no JavaScript errors |

## Browser test
It needs Playwright and its Chromium once (the test is skipped when they are missing):

```bash
pip install playwright
python3 -m playwright install chromium        # about 120 MB, stored under ~/.cache/ms-playwright
python3 tests/e2e_browser.py --screenshots /tmp/shots        # also saves a PNG of each screen
python3 tests/e2e_browser.py --headed                        # watch it run
```
It starts the server on a temporary data directory, so it never touches `~/.misra-gui`. The page's strict
Content-Security-Policy forbids `eval`, so the test waits with `page.evaluate` polling instead of
`wait_for_function`.

## Other checks
- `python3 scripts/validate_rules.py` : per-rule fixture validation table.
- `python3 scripts/fuzz_broken_input.py SEED ROUNDS` : the checker must not crash on code with injected errors.
- `python3 scripts/run_freertos_oracle.py --work-dir /tmp/frk` : comparison with FreeRTOS-Kernel's published
  deviations (needs internet).

## Per-rule specifications and rule cards

Every rule has annotated cases in `tests/rulespec/*.specs` (`--- ok|bad|gap|fp: description`, `/*@V*/` marks a
line that must be reported, `/*@N*/` one that must not). `python3 scripts/rulespec.py --check` runs them
(ctest `rulespec`); `gap` and `fp` cases document known false negatives and false positives and flip to
"promote" when the detector improves. `python3 scripts/gen_rule_cards.py` writes one card per rule to
[rule-cards/](rule-cards/README.md): what is detected, the known limits, and every case with its result.
