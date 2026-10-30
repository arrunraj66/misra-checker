# Whole-program rule detectors

Original engineering notes; licensed MISRA wording remains authoritative.

The Clang adapter records file-scope functions and objects (`SymbolFact`:
name, linkage, declaration/definition/reference role, location, translation
unit, type text, parameter names) for every translation unit of a run and
merges them in one `AnalysisContext`. Symbols declared in system headers are
ignored. Analyze every translation unit of the program in one invocation, e.g.
`misra-checker analyze --compile-commands build/` (no `--file`).

| Rule | Diagnostic | Contract |
|---|---|---|
| 5.8 | `misra-c2012-5.8-external-name-reused` | An external-linkage name that is also an internal-linkage name elsewhere. |
| 5.9 | `misra-c2012-5.9-internal-name-reused` | Internal-linkage definitions with the same name at different locations. |
| 8.3 | `misra-c2012-8.3-inconsistent-declaration` | Declarations of one external name with different types (qualifiers kept) or parameter names. Works in a single translation unit. |
| 8.5 | `misra-c2012-8.5-declared-in-multiple-files` | An external name with non-defining declarations in more than one file. |
| 8.6 | `misra-c2012-8.6-multiple-definitions`, `-no-definition` | An external name defined in more than one translation unit, or referenced with no definition (the latter is reported as Possible). |
| 8.7 | `misra-c2012-8.7-single-unit-external` | An external definition referenced from no other translation unit (`main` excluded). |

Rules 5.8, 5.9, 8.6 and 8.7 return `Inconclusive` when fewer than two
translation units were analyzed; the CLI says so instead of reporting a
clean result.

## Limitations

- Whole-program conclusions are only valid if all translation units (and every
  library that could define or use the symbols) are part of the run. Rule 8.7
  will flag legitimate exported APIs.
- References are recorded for direct uses of the name; uses through macros are
  attributed after expansion, and uses from other languages or libraries are
  invisible.
- Rule 8.6 counts a header-defined external object once per translation unit
  that includes it.
- Tentative definitions count as definitions.
- Independent validation, mutation testing and multi-platform testing are
  outstanding.
