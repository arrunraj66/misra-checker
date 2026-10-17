# Control-flow-graph based detectors

Original engineering notes; licensed MISRA wording remains authoritative.

`src/clang_observations_cfg.cpp` builds a Clang `CFG` (with trivially false
edges pruned) for every function defined in the main file and runs three
analyses over it.

| Rule | Diagnostic | Analysis |
|---|---|---|
| 2.1 | `misra-c2012-2.1-unreachable-code` | Blocks not reachable from the entry block; the first code statement of each dead region is reported (null statements and bare declarations are ignored). Replaces the earlier "statement after return/break/goto" heuristic. |
| 2.2 | `misra-c2012-2.2-dead-code` | Expression statements without side effects (AST) plus assignments to local, non-volatile, non-reference objects whose value is not live afterwards (Clang `LiveVariables`). |
| 9.1 | `misra-c2012-9.1-uninitialized-read` | Clang's uninitialized-values dataflow; Definite when uninitialized on every path, Possible when on some path. |
| 14.2 | `misra-c2012-14.2-for-loop-not-well-formed` | AST subset (no CFG): a single loop counter changed only by the third clause, tested by the second, not touched by the body; `for(;;)` is exempt. |
| 17.4 | `misra-c2012-17.4-missing-return-value` | A reachable predecessor of the exit block that does not end in a return or a noreturn call (replaces the structural check). `return;` without a value in a non-void function is still reported from the AST. |

## Limitations

- Constant-folded conditions (`sizeof(int) == 4`, macro-defined switches) make
  one arm unreachable; this is reported even though the code is configuration
  dependent. Several reports can point at one source line when a macro expands
  into several blocks.
- 2.2 covers simple assignments only; dead initializers and computations
  without an assignment are not found. It is a subset of "dead code".
- 9.1 analyses scalar automatic objects; members of aggregates and objects
  whose address escapes are not tracked precisely.
- 14.2 does not model loop control flags beyond requiring a side-effect-free
  condition that mentions the counter.
- Clang 14 is the baseline; the CFG and analysis APIs differ in later releases.
