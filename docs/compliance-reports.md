# Compliance reports

After an analysis, **Compliance report** (HTML, printable) and **Summary CSV** in the results header
produce the documents MISRA Compliance:2020 asks projects to keep. A sample is in
[docs/examples/compliance-report-sample.html](examples/compliance-report-sample.html).

## What is in the report
1. **Scope and tools**: workspace, number of translation units and lines, language standard, compiler profile
   and flags, defines, whether cppcheck cross-checked, tool versions, whether the code had compile errors.
2. **Result summary**: unresolved violations by category (mandatory / required / advisory) and how many
   guidelines have violations, are deviated, have none reported, or could not be assessed.
3. **Guideline Compliance Summary**: one row per rule (143) with its category, topic and a status:
   - *violations*: unresolved violations exist (count shown);
   - *deviated*: every violation is covered by an approved deviation;
   - *no violation reported*: the tool reported none;
   - *not assessed*: a whole-program rule that needs at least two translation units analysed together.
4. **Unresolved violations**: each with category, rule, location and the source line.
5. **Deviation register**: every deviation record with its rules, scope, justification, approver and
   expiry, and whether it is *in use*, *expired* (not applied) or *unused*, with the locations it covers.
6. **Guideline Enforcement Plan**: for each rule, what enforces it (misra-checker, plus the cppcheck add-on
   when it covers the rule) and what is left to review (undecidable rules, rules whose fixtures only
   partly validate, rules not assessed).
7. **Evidence and limits**: per-rule fixture validation counts, the independent-validation status, and a
   reminder that the tool is not qualified.

The CSV has one row per rule (status, counts, decidability, fixture validation). The JSON form is available
at `GET /api/compliance?key=<workspace>&format=json`.

## What the report does not say
It never says "compliant". It says what the tool found. A project claims MISRA compliance only when every
guideline is compliant or covered by an approved deviation, the directives and process requirements are met
(this tool checks neither the directives nor the process), and the tool's own detectors have been validated
independently. The disclaimer is printed at the top of every report.

## How to use it
1. Analyse the whole project, with the right compiler profile and the project's deviation file, so the
   whole-program rules are assessed and the deviations are accounted for.
2. Fix or deviate the unresolved violations; save a baseline to track what changes.
3. Generate the report for the release and keep it with the build.
