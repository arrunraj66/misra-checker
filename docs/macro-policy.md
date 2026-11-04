# Macro handling policy

Status: normative for the checker implementation. Applies to every rule that
consumes Clang facts. Original engineering text; it does not reproduce any
licensed MISRA content.

## Why a policy is needed

Clang analyses code *after* preprocessing. A construct can therefore have two
locations (where a macro is written and where it is invoked) and two owners
(the project, or a system library). Without a stated policy the same source
produces different findings depending on how it is wrapped in macros.

## Rules

1. **Report at the expansion site.** A finding is located where the programmer
   invoked the macro (the *expansion location*), never inside the macro body.
   This is the line a developer edits or reviews.
2. **Tag, never hide.** If the offending construct was produced by a macro
   expansion, the location carries `from_macro = true`. The command line prints
   `[macro]` after the diagnostic and the summary counts such findings. The
   certainty (`Definite` or `Possible`) is unchanged: the expanded code is the
   code that is compiled.
3. **System-library code is not the project's code.** Locations inside system
   headers are never reported. For *structural* facts (control flow, operators,
   literals, declarations) a construct whose *spelling* is inside a system
   header is ignored even when it is invoked from project code. Examples:
   an octal constant inside `S_IRUSR`, a comma inside a library macro.
4. **Uses of a banned facility are the exception to rule 3.** A call fact, and
   a `va_arg` expression (Rule 17.1), are keyed on the call site, so a project macro such as `#define RELEASE(p) free(p)` is reported
   (tagged `[macro]`), while a library-internal macro that happens to call a
   banned function is not.
5. **One finding per rule, kind and expansion site.** Several offending
   constructs produced by a single macro invocation collapse into one finding at
   the invocation. Each invocation of the macro is a separate finding.
6. **Headers are analysed once.** Facts located in project headers are analysed
   (not only the main file) and de-duplicated by expansion location, so a header
   included by many translation units is reported once.
7. **Macros as subjects.** Rules about macros themselves (2.5, 5.4, 5.5) use the
   preprocessor event stream. Built-in and command-line macros, and macros
   defined in system headers, are out of scope. A macro counts as used if it
   is expanded or tested with `defined`, `#ifdef` or `#ifndef` in any
   analysed translation unit.

## Suppression

`--suppress-macro-findings` removes tagged findings from the report. It exists
for triage of legacy code. Because it hides real findings, the tool then prints
a warning that the result is not compliance evidence, and the exit code only
reflects the remaining findings. Do not use it for a compliance claim.

## Known limits

* Rule 3 can hide a genuine violation that a project deliberately imports from
  a third-party header passed as a system include (`-isystem`). Reclassify such
  headers as normal includes if they are part of the project.
* Constructs assembled by token pasting are located by their invocation, not by
  the pasted tokens.
* Only the preprocessed view is analysed: code removed by `#if 0` or an
  inactive configuration is not seen. Analyse each shipped configuration.
