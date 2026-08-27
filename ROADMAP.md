# Industrial MISRA C Checker and Controlled Converter Roadmap

Version: 0.1
Planning date: 2026-08-27
Planned start: 2026-08-31
Reference program length: 78 weeks (18 months)
Initial technical baseline: MISRA C:2012 plus MISRA Compliance:2020

## 1. Objective

Build an industrial static-analysis product that:

1. analyzes C projects against a versioned MISRA rule baseline;
2. performs both single-translation-unit and whole-program analysis;
3. distinguishes definite violations, possible violations, tool limitations, and manually reviewed directives;
4. produces the evidence and workflow records required by MISRA Compliance:2020;
5. offers conservative, traceable source-code remediation only where semantic preservation can be demonstrated;
6. is engineered under a controlled lifecycle suitable for later qualification or certification under a selected functional-safety standard.

The product must never claim that a scan alone makes a project MISRA compliant. It provides enforcement evidence; the project, its processes, deviations, reviews, and delivered records support the compliance claim.

## 2. Planning assumptions

- Greenfield repository and product.
- Initial rule interpretation uses the locally available MISRA C:2012 reference. Process requirements use MISRA Compliance:2020 because it supersedes the older compliance process.
- Rule wording will not be copied into distributed product assets until suitable MISRA licensing is obtained.
- MISRA C:2025 support will be added as a separately versioned rule pack after licensing and gap analysis.
- Primary host platforms: Linux first, Windows second.
- Initial compiler profiles: GCC and Clang on ARM and x86; proprietary embedded compilers follow through configurable profiles.
- Input comes from `compile_commands.json`, explicit command lines, or a project manifest.
- The converter operates in suggestion mode by default. Automatic application is limited to approved low-risk transformations.
- Reference team: 10-12 full-time staff. Fewer than 6 staff makes the 18-month schedule unrealistic.
- An independent assessor is engaged before architecture freeze, not only at the end.

## 3. Team and ownership

| Role | Count | Primary responsibility |
|---|---:|---|
| Product and safety lead | 1 | Scope, standards, claims, roadmap, assessor interface |
| Principal compiler engineer | 1 | Parser, preprocessor, compiler profiles, source fidelity |
| Compiler/front-end engineer | 1 | Build capture, AST normalization, macro provenance |
| Static-analysis engineers | 2 | CFG, data flow, abstract interpretation, whole-program analysis |
| Rule engineers | 2 | Rule specifications, implementations, limitations, validation cases |
| Remediation engineer | 1 | Fix planning, rewriting, safety conditions, equivalence evidence |
| Verification and validation engineers | 2 | Independent tests, oracle review, qualification evidence |
| DevOps/configuration manager | 1 | Reproducible builds, CI, signing, release and change control |
| Technical writer/quality engineer | 1 | Plans, procedures, manuals, traceability, audit package |

Part-time support is also needed from legal/licensing, cybersecurity, domain experts, and an independent functional-safety assessor.

## 4. Product boundaries

### 4.1 Checker responsibilities

- Reconstruct the real compilation environment.
- Preserve preprocessor and macro-expansion provenance.
- Parse C90/C99 and configured compiler extensions.
- Model implementation-defined properties such as widths, ranks, signedness, representation, alignment, character sets, and floating-point behavior.
- Maintain both ISO C standard types and MISRA essential types.
- Build symbols, scopes, linkage relationships, CFGs, call graphs, data-flow facts, ranges, points-to information, object lifetimes, resource states, and cross-unit relationships.
- Run versioned guideline checks and record coverage and limitations.
- Produce deterministic diagnostics and compliance evidence.

### 4.2 Converter responsibilities

- Never rewrite code unless the triggering finding is stable and reproducible.
- Generate a proposed patch with explicit preconditions and assumptions.
- Preserve comments, macros, formatting, line endings, and build behavior where possible.
- Reparse and reanalyze the changed program.
- Compile and execute configured regression tests.
- Retain a full audit trail linking finding, fix, diff, verification, user approval, and tool version.

### 4.3 Explicit non-goals for the first release

- Claiming perfect detection of undecidable rules.
- Automatically resolving directives that depend on requirements or design intent.
- Rewriting hardware-access, concurrency, volatile, assembly, ABI, pointer-lifetime, or timing-sensitive code without manual approval.
- Supporting every proprietary compiler extension in version 1.0.
- Claiming certification before an external conformity assessment is complete.

## 5. Reference architecture

```text
Build capture / project manifest
              |
Compiler profile + target model
              |
Preprocessor and parser with source provenance
              |
Normalized AST + standard type model + essential type model
              |
Local analysis ---------------- Whole-program index
     |                                |
CFG / def-use / ranges        linkage / calls / globals
     |                                |
     +---------- analysis facts ------+
                       |
              Versioned rule engine
                       |
     Diagnostics + certainty + limitations
                       |
       Review / deviations / suppressions
                       |
       Controlled remediation planner
                       |
        Rebuild / reanalyze / retest
                       |
     GEP / GRP / GCS / SARIF / audit bundle
```

Recommended implementation baseline:

- Pin a reviewed LLVM/Clang version for preprocessing, parsing, source locations, and rewriting.
- Use a narrow internal abstraction so the product is not coupled directly to unstable Clang APIs.
- Implement analysis services in memory-safe code where practical; a C++ bridge may be necessary for Clang integration.
- Store the whole-program index in a deterministic, schema-versioned database.
- Use SARIF 2.1.0, JSON, HTML, and CLI text output.
- Keep rule metadata and executable implementations separate but traceably linked.
- Make builds hermetic and reproducible with locked dependencies and generated SBOMs.

## 6. Required controlled documents

The following are product-development records, not customer-project GEP/GRP/GCS records:

1. Product requirements document (PRD)
2. Software requirements specification (SRS)
3. Software architecture description (SAD)
4. Rule interpretation specification for each guideline
5. Tool operational requirements (TOR)
6. Tool qualification/certification plan
7. Software verification plan (SVP)
8. Configuration-management plan (CMP)
9. Software quality-assurance plan (SQAP)
10. Cybersecurity plan and threat model
11. Development and coding standard
12. Problem-reporting and corrective-action procedure
13. Change-control and impact-analysis procedure
14. Release and signing procedure
15. Validation suite specification and results
16. Known-limitations and known-defects register
17. User, integration, configuration, and qualification manuals
18. Requirements-to-design-to-code-to-test traceability matrix

Customer-facing compliance records supported by the product:

- Guideline Enforcement Plan (GEP)
- Guideline Re-categorization Plan (GRP)
- Deviation permits
- Deviation records
- Diagnostic investigation records
- Guideline Compliance Summary (GCS)
- Tool/configuration manifest and scan evidence bundle

## 7. Rule specification schema

Every guideline implementation must have a version-controlled record containing:

- edition and guideline identifier;
- category and permitted project re-categorizations;
- C language applicability;
- directive or rule classification;
- decidability and analysis scope;
- normative interpretation, amplification, and exceptions;
- required compiler/target facts;
- required AST and analysis facts;
- precise violation predicate;
- diagnostic location and related locations;
- certainty level and possible false-positive conditions;
- known false-negative conditions;
- handling of macros, generated code, system headers, and adopted code;
- remediation classification and safety preconditions;
- positive, negative, boundary, interaction, and regression test identifiers;
- coverage status and reviewer approvals.

No guideline is advertised as supported until its specification, implementation, independent oracle, tests, limitations, and traceability are complete.

## 8. Definition of done

### 8.1 Per rule

- Interpretation approved by a rule engineer and independent reviewer.
- All normative exceptions implemented.
- Relevant compiler-profile dependencies identified.
- Single-unit/system and decidability behavior correctly represented.
- At least one focused violation test and one compliant test for each semantic clause.
- Boundary, macro, conditional-compilation, and interaction tests present where relevant.
- False-positive and false-negative limitations documented.
- Diagnostic contains guideline ID, certainty, location, explanation key, and remediation class.
- Tests run across supported language and target profiles.
- Requirements, code, tests, and results are traceably linked.

### 8.2 Per automatic fix

- Preconditions are machine-checkable.
- Transformation is idempotent.
- Original file can be restored exactly.
- Changed translation unit reparses successfully.
- No new analyzer findings are introduced under the selected GRP.
- Configured compilers accept the result.
- Required tests pass.
- Review log contains before/after hashes, diff, reason, assumptions, and approval.
- Independent V&V approves the fix class before release.

### 8.3 Per release

- All planned requirements are closed or formally deferred.
- No open release-blocking problem reports.
- Full validation suite passes on clean infrastructure.
- Reproducible-build comparison succeeds.
- SBOM, dependency notices, signed artifacts, checksums, manuals, limitations, and release notes are complete.
- Independent V&V signs the release verification record.
- Safety/product lead approves external claims.

## 9. Phase roadmap and exit gates

| Phase | Dates | Outcome | Exit gate |
|---|---|---|---|
| 0. Program inception | Weeks 1-2, 2026-08-31 to 2026-09-13 | Scope, claims, team, lifecycle, licensing actions | Approved charter and controlled baselines |
| 1. Requirements and architecture | Weeks 3-8, 2026-09-14 to 2026-10-25 | SRS, SAD, rule schema, compiler-profile model, prototypes | Architecture and assessor review passed |
| 2. Analysis platform | Weeks 9-16, 2026-10-26 to 2026-12-20 | Build capture, parser abstraction, local IR, diagnostics | Deterministic end-to-end scans on reference projects |
| 3. Local decidable rule MVP | Weeks 17-24, 2026-12-21 to 2027-02-14 | Initial high-confidence rule pack and reporting | Internal MVP with independent validation |
| 4. Whole-program analysis | Weeks 25-36, 2027-02-15 to 2027-05-09 | System index, cross-unit and flow analyses | Pilot-ready whole-program analyzer |
| 5. Controlled remediation | Weeks 37-44, 2027-05-10 to 2027-07-04 | Suggestion engine and safe fix classes | Converter safety review passed |
| 6. Coverage expansion and workflows | Weeks 45-52, 2027-07-05 to 2027-08-29 | Broader guideline coverage, deviations, GEP/GRP/GCS | Feature-complete production candidate |
| 7. Industrial hardening | Weeks 53-64, 2027-08-30 to 2027-11-21 | Performance, security, integrations, pilots | Release candidate and pilot acceptance |
| 8. Qualification/certification evidence | Weeks 65-76, 2027-11-22 to 2028-02-13 | Independent V&V and assessment closure | Assessor findings closed |
| 9. Release | Weeks 77-78, 2028-02-14 to 2028-02-27 | Signed 1.0 release and support readiness | Go-live approval |

## 10. Detailed daily plan: first 12 weeks

Each day ends with updated traceability, reviewed commits, passing tests, and a short recorded decision log. Dates assume a Monday-Friday workweek.

### Week 1: 2026-08-31 to 2026-09-04 - Charter and product claims

- **Mon:** Kickoff; assign owners; establish decision authority; define target users, industries, deployment models, and business constraints.
- **Tue:** Inventory reference documents; record licence restrictions; create standards and amendments register; open licensing action.
- **Wed:** Define allowed and prohibited product claims; distinguish project compliance, rule coverage, tool qualification, and certification.
- **Thu:** Define version-1 scope, supported hosts, input modes, output formats, and explicit non-goals.
- **Fri:** Review and approve program charter, risk register, issue taxonomy, meeting cadence, and document-control conventions.

### Week 2: 2026-09-07 to 2026-09-11 - Lifecycle and assurance foundation

- **Mon:** Select provisional certification domain; create standards applicability matrix and assessor engagement plan.
- **Tue:** Define lifecycle states for requirements, design, code, tests, problem reports, and releases.
- **Wed:** Define configuration identification, baselines, branching, reviews, signatures, artifact retention, and supplier controls.
- **Thu:** Write independence rules for development and V&V; establish rule-oracle review process.
- **Fri:** Baseline CMP, SQAP, verification strategy, problem-reporting process, and initial qualification strategy.

### Week 3: 2026-09-14 to 2026-09-18 - User and system requirements

- **Mon:** Conduct user-workflow analysis for developer, reviewer, safety manager, CI administrator, and auditor personas.
- **Tue:** Specify project import, compilation capture, scan, triage, deviation, remediation, and report workflows.
- **Wed:** Define functional requirements for diagnostics, baselines, suppressions, approvals, audit logs, and exports.
- **Thu:** Define non-functional requirements: determinism, performance, scalability, availability, security, usability, and portability.
- **Fri:** Review SRS requirements for atomicity, verifiability, traceability, ambiguity, and certification relevance.

### Week 4: 2026-09-21 to 2026-09-25 - Rule model and coverage planning

- **Mon:** Create the rule-specification template and lifecycle from draft through independently validated.
- **Tue:** Build the internal metadata catalogue using identifiers and attributes without redistributing protected text.
- **Wed:** Group rules by required capability: lexical, syntax, type, essential type, linkage, CFG, data flow, pointer, resource, preprocessing, library, and manual review.
- **Thu:** Score implementation complexity, false-positive risk, whole-program needs, and remediation suitability.
- **Fri:** Select the MVP rule set and approve a coverage roadmap with supported, assisted, manual, and unsupported states.

### Week 5: 2026-09-28 to 2026-10-02 - Compiler and target model

- **Mon:** Specify compiler-profile schema: language mode, target triple, widths, ranks, signedness, alignment, representation, built-ins, extensions, and predefined macros.
- **Tue:** Specify preprocessing fidelity, include resolution, conditional branches, macro expansions, tokens, comments, and source ranges.
- **Wed:** Specify build-capture inputs and normalized compile-command representation.
- **Thu:** Define system-header, adopted-code, generated-code, and manually injected-code provenance.
- **Fri:** Review GCC, Clang, ARM, and x86 example profiles and document unsupported properties.

### Week 6: 2026-10-05 to 2026-10-09 - Analysis architecture

- **Mon:** Define normalized AST entities, stable IDs, source locations, types, symbols, declarations, and definitions.
- **Tue:** Define standard-type and essential-type services, integer constant evaluation, enum identity, ranks, and conversions.
- **Wed:** Define CFG, def-use, dominance, reachability, side-effect, and constant/range fact APIs.
- **Thu:** Define whole-program index for linkage, calls, global objects, typedefs, tags, macros, and translation-unit configurations.
- **Fri:** Conduct architecture threat/failure analysis and record fault-containment boundaries.

### Week 7: 2026-10-12 to 2026-10-16 - Diagnostics and evidence architecture

- **Mon:** Define diagnostic schema, certainty, severity, category, rule edition, primary and related locations, fingerprints, and message keys.
- **Tue:** Define baseline matching resilient to harmless line movement while preventing accidental finding loss.
- **Wed:** Define suppression and deviation records, approval roles, expiry, scope, risk, and immutable audit events.
- **Thu:** Define GEP, GRP, GCS, SARIF, JSON, HTML, and evidence-bundle schemas.
- **Fri:** Prototype a sample end-to-end evidence bundle and review it from an auditor's perspective.

### Week 8: 2026-10-19 to 2026-10-23 - Architecture prototypes and gate

- **Mon:** Prototype Clang preprocessing with token and macro provenance on representative embedded C.
- **Tue:** Prototype AST normalization and stable source identifiers.
- **Wed:** Prototype cross-unit symbol indexing from two differently configured translation units.
- **Thu:** Prototype one local rule, one system rule, and one manually reviewed directive through the common result interface.
- **Fri:** Hold architecture gate with independent V&V and assessor; close critical findings or replan before implementation.

### Week 9: 2026-10-26 to 2026-10-30 - Repository and build foundation

- **Mon:** Create repository layout, build system, dependency lock, coding standard, formatter, linter, and commit checks.
- **Tue:** Create hermetic developer container and clean CI build on Linux.
- **Wed:** Add unit, integration, end-to-end, negative-build, sanitizer, and coverage test stages.
- **Thu:** Add SBOM generation, licence scanning, artifact hashing, signing placeholders, and reproducibility checks.
- **Fri:** Validate clean-room checkout-to-test workflow and baseline build evidence.

### Week 10: 2026-11-02 to 2026-11-06 - Project ingestion

- **Mon:** Implement and test `compile_commands.json` ingestion.
- **Tue:** Implement normalized command-line parsing and response-file handling.
- **Wed:** Implement include-path, macro, language-mode, sysroot, and target extraction.
- **Thu:** Implement project manifest validation and friendly configuration diagnostics.
- **Fri:** Run ingestion against small GCC and Clang projects and record deterministic manifests.

### Week 11: 2026-11-09 to 2026-11-13 - Front-end abstraction

- **Mon:** Implement translation-unit driver and pinned Clang adapter.
- **Tue:** Implement source-file, token, comment, include, and macro-expansion provenance.
- **Wed:** Implement normalized declarations, expressions, statements, and type nodes.
- **Thu:** Implement stable IDs and serialization with schema-version compatibility tests.
- **Fri:** Test malformed input, partial builds, generated headers, conditional compilation, and extension diagnostics.

### Week 12: 2026-11-16 to 2026-11-20 - Type services and vertical slice

- **Mon:** Implement compiler-derived integer width, rank, representation, character, and floating-point configuration.
- **Tue:** Implement standard C type classification and conversion service.
- **Wed:** Implement the initial essential-type categories and expression propagation framework.
- **Thu:** Connect one rule specification to parsing, analysis, diagnostic, SARIF, HTML, and test evidence.
- **Fri:** Demonstrate the vertical slice; conduct requirements/design/code/test traceability audit; approve Phase 2 backlog.

## 11. Weekly delivery plan: weeks 13-78

The daily pattern for every week is:

- **Monday:** refine requirements and rule interpretations; conduct impact analysis.
- **Tuesday:** implement analysis or workflow capability with unit tests.
- **Wednesday:** complete implementation and developer integration tests.
- **Thursday:** independent V&V adds adversarial, boundary, and regression tests; performance/security checks run.
- **Friday:** resolve findings, update limitations and traceability, demonstrate the increment, and baseline accepted evidence.

The specific weekly outcomes are:

### Weeks 13-16: Analysis platform completion

- **W13:** Symbol tables, scopes, linkage, declaration/definition relationships, and identifier indexing.
- **W14:** CFG construction, reachability, dominance, loops, switch structure, and control-transfer modeling.
- **W15:** Constant evaluation, side effects, sequence relationships, def-use chains, and local range facts.
- **W16:** Diagnostic fingerprints, baseline comparison, deterministic reporting, schema migration, and Phase 2 gate.

### Weeks 17-24: Local decidable rule MVP

- **W17:** Lexical, comment, escape, literal, and token-level rule family.
- **W18:** Identifier scope, hiding, distinctness, and local declaration rule family.
- **W19:** Declaration, definition, initialization, qualifier, and prototype rule family.
- **W20:** Standard and essential-type core rule family with exhaustive conversion tables.
- **W21:** Pointer-conversion and expression-shape rule family.
- **W22:** Control-statement, loop, switch, and function-structure rule family.
- **W23:** Preprocessor and restricted-library rule family.
- **W24:** Independent MVP validation, false-positive review, user documentation, performance baseline, and internal release gate.

### Weeks 25-36: Whole-program and undecidable analysis

- **W25:** Whole-program database, incremental invalidation, and configuration partitioning.
- **W26:** External linkage consistency, uniqueness, and definition analysis.
- **W27:** Cross-unit call graph, indirect-call representation, recursion, and reachability.
- **W28:** Interprocedural def-use summaries and side-effect summaries.
- **W29:** Flow-sensitive range analysis and configurable widening/narrowing.
- **W30:** Points-to and alias-analysis foundation with documented precision modes.
- **W31:** Object lifetime, initialization state, nullness, and invalid pointer state.
- **W32:** Buffer extent, pointer arithmetic, overlapping storage, and bounds facts.
- **W33:** Resource-state analysis for allocation, streams, acquisition, release, and sequencing.
- **W34:** Undefined and critical-unspecified behavior mapping and coverage tests.
- **W35:** Undecidable-result model: definite, possible, proven compliant, inconclusive, and analysis-limit outcomes.
- **W36:** Large-project pilot, precision/performance tuning, independent system-analysis validation, and gate.

### Weeks 37-44: Controlled remediation

- **W37:** Remediation policy, transformation IR, precondition language, patch model, and rollback.
- **W38:** Whitespace-independent token rewrites and safe literal/suffix transformations.
- **W39:** Parenthesization and brace transformations with macro and precedence guards.
- **W40:** Declaration and qualifier suggestions; automatic application only for independently approved cases.
- **W41:** Essential-type remediation suggestions with explicit range and conversion evidence.
- **W42:** Control-flow remediation suggestions; prohibit automatic application where behavior may change.
- **W43:** Reparse, reanalyze, compile, test, diff, hash, approval, and evidence pipeline.
- **W44:** Mutation and metamorphic testing of fix classes, human-factors review, converter safety gate.

### Weeks 45-52: Compliance workflows and feature completion

- **W45:** GRP authoring, validation, inheritance, multiple component profiles, and project freezing.
- **W46:** GEP coverage mapping, tool/configuration evidence, and manual procedure linkage.
- **W47:** Deviation permits, deviation records, scoped suppressions, expiry, and technical-authority approval.
- **W48:** Diagnostic investigation workflow for definite, possible, false-positive, and non-MISRA findings.
- **W49:** Native, adopted, binary, generated, standard-library, and header ownership models.
- **W50:** GCS aggregation and worst-case merging across component GRPs.
- **W51:** CI quality gates, SARIF upload, IDE integration, REST/CLI stability, and access controls.
- **W52:** Feature-complete validation, documentation review, unsupported-coverage disclosure, and production-candidate gate.

### Weeks 53-64: Industrial hardening and customer pilots

- **W53:** Performance profiling on one-million-line project; establish memory/time budgets.
- **W54:** Parallel and incremental analysis with deterministic equivalence tests.
- **W55:** Corrupt cache, crash recovery, cancellation, partial results, and resumability.
- **W56:** Windows port, filesystem/case behavior, path normalization, and reproducibility.
- **W57:** Proprietary compiler profile SDK and one representative embedded compiler profile.
- **W58:** Security threat model closure, fuzzing, dependency review, secrets, sandboxing, and malicious-source tests.
- **W59:** Installer, offline deployment, licence enforcement, upgrade, rollback, and data migration.
- **W60:** Pilot 1 onboarding, scan, triage workshop, measurement, and problem reports.
- **W61:** Pilot 1 corrections and regression closure.
- **W62:** Pilot 2 onboarding on a different compiler/target and adopted-code mix.
- **W63:** Pilot 2 corrections, performance closure, and user-documentation validation.
- **W64:** Release-candidate audit, complete validation run, reproducible build, and hardening gate.

### Weeks 65-76: Qualification/certification package

- **W65:** Freeze tool operational requirements and intended-use/qualification boundary.
- **W66:** Complete requirements traceability and requirements-based coverage review.
- **W67:** Complete structural coverage review and justify unreachable/deactivated tool code.
- **W68:** Perform independent robustness, stress, malformed-input, and fault-injection testing.
- **W69:** Validate every advertised rule against the approved oracle suite.
- **W70:** Validate every advertised remediation class and ensure forbidden classes cannot auto-apply.
- **W71:** Audit configuration management, third-party components, SBOM, build chain, and release reproducibility.
- **W72:** Audit problem reports, known limitations, user notification, and change-impact evidence.
- **W73:** Complete safety/qualification manuals and operational constraints.
- **W74:** Conduct internal mock assessment and resolve major findings.
- **W75:** External assessor review and evidence walkthrough.
- **W76:** Close assessor findings and approve the qualification/certification baseline.

### Weeks 77-78: Release and operational readiness

- **W77:** Build and sign version 1.0; publish checksums, SBOM, release notes, manuals, limitations, and support knowledge base.
- **W78:** Final go/no-go review; release; monitor installation and first scans; open controlled maintenance baseline.

## 12. Daily operating cadence after week 12

### Engineering

- 09:00: 15-minute cross-workstream blocker and safety-impact check.
- 09:15-12:30: focused implementation or rule-specification work.
- 13:30-15:30: tests, code review, integration, and performance measurement.
- 15:30-16:30: defect triage and independent V&V handoff.
- 16:30-17:00: update traceability, decision records, limitations, and evidence.

### Independent V&V

- Review requirements and rule interpretations before relying on implementation details.
- Design tests from normative semantics and documented tool operational requirements.
- Maintain an independent expected-result oracle.
- Attempt to falsify both violation detection and claimed absence of violations.
- Reject release evidence that is not reproducible from a clean baseline.

### Weekly governance

- Monday: backlog and change-control board.
- Wednesday: architecture/rule clinic and problem-report review.
- Friday: demonstration, metrics, risks, evidence baseline, and release-readiness review.
- Monthly: independent assessor checkpoint beginning no later than month 3.

## 13. Verification strategy

Use multiple complementary test classes:

1. **Unit tests:** parser adapter, types, essential types, CFG, ranges, aliases, diagnostics, serialization.
2. **Rule conformance tests:** focused compliant and violating programs for every semantic clause.
3. **Boundary tests:** integer widths, ranks, enum values, translation limits, macro depth, array extents, pointer states.
4. **Interaction tests:** multiple rules, headers, macro expansions, compiler extensions, generated/manual code boundaries.
5. **System tests:** multiple translation units, libraries, stubs, incomplete source, conflicting profiles.
6. **Metamorphic tests:** semantics-preserving source variations should preserve relevant results.
7. **Mutation tests:** seeded violations must be detected where coverage is claimed.
8. **Differential tests:** compare compilers and reputable analyzers as evidence discovery, never as the sole oracle.
9. **Fuzzing:** preprocessing tokens, declarations, expressions, control flow, manifests, databases, and SARIF/JSON inputs.
10. **Performance tests:** small interactive scans through million-line embedded systems.
11. **Converter tests:** idempotence, rollback, formatting preservation, recompilation, reanalysis, tests, and patch auditability.
12. **Qualification tests:** requirements-based, independently reviewed, cleanly reproducible, and retained as controlled evidence.

## 14. Metrics and release thresholds

Track these from the first vertical slice:

- Advertised guidelines fully validated / total advertised guidelines: 100%.
- Requirements with bidirectional traceability: 100% at release.
- Automatic fix classes independently approved: 100% before exposure.
- Open severity-1 defects: 0 for every external release.
- Open severity-2 defects affecting claimed rule correctness: 0, or affected claim removed.
- Deterministic result match across repeated clean runs: 100%.
- Reproducible artifact hash match: 100% for release builds.
- Validation-suite pass rate: 100% for supported profiles.
- Diagnostic precision and recall: measured per rule family on labelled pilot corpora; thresholds approved rather than hidden behind a single aggregate.
- Performance target for 1 million preprocessed lines: establish at W24, enforce at W52, and retest at every release.
- Crash-free analysis rate and mean time to failure: measured during pilots and fuzzing.
- Suppression/deviation traceability: 100% of accepted violations linked to an approved record.

## 15. Primary risks and controls

| Risk | Control |
|---|---|
| Copyright/licensing prevents distribution of rule text | Keep rule assets internal; use identifiers/message keys; secure vendor licence before commercial release |
| Incorrect interpretation of a guideline | Two-person interpretation review, forum/corrigenda tracking, independent oracle, documented ambiguity |
| Clang behavior differs from customer compiler | Explicit compiler profiles, conformance probes, extension adapters, compiler-diagnostic ingestion |
| False negatives create unjustified confidence | Disclose coverage and limits per rule; qualification tests; no scan-only compliance claim |
| Excessive false positives make tool unusable | Certainty levels, precision modes, path/range analysis, labelled pilot measurements |
| Whole-program scale is too slow | Incremental index, summaries, parallelism, performance budgets, representative corpora |
| Converter changes behavior | Allow-listed fix classes, formal preconditions, reanalysis/build/test gates, default suggestion mode |
| Certification work starts too late | Engage assessor by architecture phase; maintain evidence continuously |
| Proprietary compiler ecosystem fragments scope | Profile SDK, supported-profile matrix, staged target expansion |
| Third-party dependency defect affects results | Pinned versions, SBOM, supplier assessment, wrappers, regression probes, known-defect register |

## 16. Infrastructure and access needed before Week 3

- Source-control and protected-branch administration.
- Issue tracker with requirement, defect, risk, change, and assessment item types.
- CI runners for Linux and Windows.
- Artifact repository with immutability and retention.
- Code signing keys backed by controlled secrets storage.
- Dependency proxy/cache for reproducible offline builds.
- Representative GCC, Clang, ARM, and proprietary compiler installations and documentation.
- Small, medium, and large legally usable embedded C corpora.
- Fuzzing and sanitizer infrastructure.
- Requirements and traceability system, or a controlled repository-based equivalent.
- Access for independent V&V and external assessor.

## 17. Budgetary planning ranges

These are planning ranges, not quotations:

- 10-12 staff for 18 months: 180-216 person-months.
- External assessor: engage early and reserve architecture, mid-program, and final assessment effort.
- MISRA guideline and tool-vendor licensing: obtain a formal quotation.
- Functional-safety standards and training: budget for named-user or organizational access.
- Compiler licences and target hardware: at least two representative embedded toolchains.
- CI, signing, artifact retention, fuzzing, and security scanning infrastructure.
- Contingency: 20-30% for rule ambiguities, compiler extensions, precision work, and assessment findings.

## 18. First release content recommendation

Version 1.0 should prioritize correctness and evidence over headline coverage:

- Complete infrastructure for all guideline metadata and compliance workflows.
- Strong coverage of decidable lexical, syntactic, declaration, essential-type, control-structure, preprocessing, and restricted-library rules.
- Carefully scoped whole-program identifier, linkage, call-graph, initialization, resource, and pointer analyses.
- Manual-assistance workflows for directives and unsupported undecidable cases.
- Only independently approved low-risk automatic fixes; all other remediation is advisory.
- Transparent per-guideline coverage and limitation report.

Do not market partial, heuristic support as full guideline enforcement.

## 19. Immediate actions

1. Appoint the product/safety lead and configuration manager.
2. Confirm team size and whether the first certification target is automotive, industrial, aerospace, rail, or medical.
3. Open the MISRA commercial tool-licensing request.
4. Acquire organization-licensed copies of the selected guideline edition and applicable standards.
5. Select and contact an independent assessor.
6. Approve the 78-week charter or rebaseline it against available staffing.
7. Start Week 1 on 2026-08-31 and do not begin rule coding until the rule schema, lifecycle, and claims policy are approved.
