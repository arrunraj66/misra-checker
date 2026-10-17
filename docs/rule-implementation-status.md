# MISRA C:2012 rule implementation inventory

This catalog records the engineering structures present in the checker. It deliberately does not reproduce the copyrighted normative rule wording. The licensed MISRA specification is the controlled source for rule predicates, amplification, exceptions, and examples.

Current milestone: No rule class is a scaffold any more: 143 rules have implemented Clang-backed syntactic detectors, but independent validation remains pending. A scaffold provides identity, metadata, an analysis plan, a factory, and a non-compliant-safe `NotImplemented` result. It is not a working compliance check.

| Rule | Category | Decidability | Scope | C90 | C99 | Topic | Status |
|---|---|---|---|---:|---:|---|---|
| 1.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard C conformance | Implemented; independent validation pending |
| 1.2 | Advisory | Undecidable | Single Translation Unit | Yes | Yes | Standard C conformance | Implemented; independent validation pending |
| 1.3 | Required | Undecidable | System | Yes | Yes | Standard C conformance | Implemented; independent validation pending |
| 2.1 | Required | Undecidable | System | Yes | Yes | Unused and unreachable code | Implemented; independent validation pending |
| 2.2 | Required | Undecidable | System | Yes | Yes | Unused and unreachable code | Implemented; independent validation pending |
| 2.3 | Advisory | Decidable | System | Yes | Yes | Unused and unreachable code | Implemented; independent validation pending |
| 2.4 | Advisory | Decidable | System | Yes | Yes | Unused and unreachable code | Implemented; independent validation pending |
| 2.5 | Advisory | Decidable | System | Yes | Yes | Unused and unreachable code | Implemented; independent validation pending |
| 2.6 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Unused and unreachable code | Implemented; independent validation pending |
| 2.7 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Unused and unreachable code | Implemented; independent validation pending |
| 3.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Comments | Implemented; independent validation pending |
| 3.2 | Required | Decidable | Single Translation Unit | No | Yes | Comments | Implemented; independent validation pending |
| 4.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Character sets and lexical elements | Implemented; independent validation pending |
| 4.2 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Character sets and lexical elements | Implemented; independent validation pending |
| 5.1 | Required | Decidable | System | Yes | Yes | Identifiers | Implemented; independent validation pending |
| 5.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Identifiers | Implemented; independent validation pending |
| 5.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Identifiers | Implemented; independent validation pending |
| 5.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Identifiers | Implemented; independent validation pending |
| 5.5 | Required | Decidable | Single Translation Unit | Yes | Yes | Identifiers | Implemented; independent validation pending |
| 5.6 | Required | Decidable | System | Yes | Yes | Identifiers | Implemented; independent validation pending |
| 5.7 | Required | Decidable | System | Yes | Yes | Identifiers | Implemented; independent validation pending |
| 5.8 | Required | Decidable | System | Yes | Yes | Identifiers | Implemented; independent validation pending |
| 5.9 | Advisory | Decidable | System | Yes | Yes | Identifiers | Implemented; independent validation pending |
| 6.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Types and bit-fields | Implemented; independent validation pending |
| 6.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Types and bit-fields | Implemented; independent validation pending |
| 7.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Literals and constants | Implemented; independent validation pending |
| 7.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Literals and constants | Implemented; independent validation pending |
| 7.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Literals and constants | Implemented; independent validation pending |
| 7.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Literals and constants | Implemented; independent validation pending |
| 8.1 | Required | Decidable | Single Translation Unit | Yes | No | Declarations and definitions | Implemented; independent validation pending |
| 8.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.3 | Required | Decidable | System | Yes | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.5 | Required | Decidable | System | Yes | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.6 | Required | Decidable | System | Yes | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.7 | Advisory | Decidable | System | Yes | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.8 | Required | Decidable | Single Translation Unit | Yes | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.9 | Advisory | Decidable | System | Yes | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.10 | Required | Decidable | Single Translation Unit | No | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.11 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.12 | Required | Decidable | Single Translation Unit | Yes | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.13 | Advisory | Undecidable | System | Yes | Yes | Declarations and definitions | Implemented; independent validation pending |
| 8.14 | Required | Decidable | Single Translation Unit | No | Yes | Declarations and definitions | Implemented; independent validation pending |
| 9.1 | Mandatory | Undecidable | System | Yes | Yes | Initialization | Implemented; independent validation pending |
| 9.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Initialization | Implemented; independent validation pending |
| 9.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Initialization | Implemented; independent validation pending |
| 9.4 | Required | Decidable | Single Translation Unit | No | Yes | Initialization | Implemented; independent validation pending |
| 9.5 | Required | Decidable | Single Translation Unit | No | Yes | Initialization | Implemented; independent validation pending |
| 10.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Implemented; independent validation pending |
| 10.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Implemented; independent validation pending |
| 10.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Implemented; independent validation pending |
| 10.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Implemented; independent validation pending |
| 10.5 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Implemented; independent validation pending |
| 10.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Implemented; independent validation pending |
| 10.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Implemented; independent validation pending |
| 10.8 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Implemented; independent validation pending |
| 11.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Implemented; independent validation pending |
| 11.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Implemented; independent validation pending |
| 11.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Implemented; independent validation pending |
| 11.4 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Implemented; independent validation pending |
| 11.5 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Implemented; independent validation pending |
| 11.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Implemented; independent validation pending |
| 11.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Implemented; independent validation pending |
| 11.8 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Implemented; independent validation pending |
| 11.9 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Implemented; independent validation pending |
| 12.1 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Expressions | Implemented; independent validation pending |
| 12.2 | Required | Undecidable | System | Yes | Yes | Expressions | Implemented; independent validation pending |
| 12.3 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Expressions | Implemented; independent validation pending |
| 12.4 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Expressions | Implemented; independent validation pending |
| 13.1 | Required | Undecidable | System | No | Yes | Side effects and evaluation order | Implemented; independent validation pending |
| 13.2 | Required | Undecidable | System | Yes | Yes | Side effects and evaluation order | Implemented; independent validation pending |
| 13.3 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Side effects and evaluation order | Implemented; independent validation pending |
| 13.4 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Side effects and evaluation order | Implemented; independent validation pending |
| 13.5 | Required | Undecidable | System | Yes | Yes | Side effects and evaluation order | Implemented; independent validation pending |
| 13.6 | Mandatory | Decidable | Single Translation Unit | Yes | Yes | Side effects and evaluation order | Implemented; independent validation pending |
| 14.1 | Required | Undecidable | System | Yes | Yes | Control statement expressions | Implemented; independent validation pending |
| 14.2 | Required | Undecidable | System | Yes | Yes | Control statement expressions | Implemented; independent validation pending |
| 14.3 | Required | Undecidable | System | Yes | Yes | Control statement expressions | Implemented; independent validation pending |
| 14.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Control statement expressions | Implemented; independent validation pending |
| 15.1 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Control flow | Implemented; independent validation pending |
| 15.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Control flow | Implemented; independent validation pending |
| 15.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Control flow | Implemented; independent validation pending |
| 15.4 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Control flow | Implemented; independent validation pending |
| 15.5 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Control flow | Implemented; independent validation pending |
| 15.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Control flow | Implemented; independent validation pending |
| 15.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Control flow | Implemented; independent validation pending |
| 16.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Implemented; independent validation pending |
| 16.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Implemented; independent validation pending |
| 16.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Implemented; independent validation pending |
| 16.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Implemented; independent validation pending |
| 16.5 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Implemented; independent validation pending |
| 16.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Implemented; independent validation pending |
| 16.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Implemented; independent validation pending |
| 17.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Functions | Implemented; independent validation pending |
| 17.2 | Required | Undecidable | System | Yes | Yes | Functions | Implemented; independent validation pending |
| 17.3 | Mandatory | Decidable | Single Translation Unit | Yes | No | Functions | Implemented; independent validation pending |
| 17.4 | Mandatory | Decidable | Single Translation Unit | Yes | Yes | Functions | Implemented; independent validation pending |
| 17.5 | Advisory | Undecidable | System | Yes | Yes | Functions | Implemented; independent validation pending |
| 17.6 | Mandatory | Decidable | Single Translation Unit | No | Yes | Functions | Implemented; independent validation pending |
| 17.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Functions | Implemented; independent validation pending |
| 17.8 | Advisory | Undecidable | System | Yes | Yes | Functions | Implemented; independent validation pending |
| 18.1 | Required | Undecidable | System | Yes | Yes | Pointers and arrays | Implemented; independent validation pending |
| 18.2 | Required | Undecidable | System | Yes | Yes | Pointers and arrays | Implemented; independent validation pending |
| 18.3 | Required | Undecidable | System | Yes | Yes | Pointers and arrays | Implemented; independent validation pending |
| 18.4 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Pointers and arrays | Implemented; independent validation pending |
| 18.5 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Pointers and arrays | Implemented; independent validation pending |
| 18.6 | Required | Undecidable | System | Yes | Yes | Pointers and arrays | Implemented; independent validation pending |
| 18.7 | Required | Decidable | Single Translation Unit | No | Yes | Pointers and arrays | Implemented; independent validation pending |
| 18.8 | Required | Decidable | Single Translation Unit | No | Yes | Pointers and arrays | Implemented; independent validation pending |
| 19.1 | Mandatory | Undecidable | System | Yes | Yes | Overlapping storage | Implemented; independent validation pending |
| 19.2 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Overlapping storage | Implemented; independent validation pending |
| 20.1 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.5 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.8 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.9 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.10 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.11 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.12 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.13 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 20.14 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Implemented; independent validation pending |
| 21.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Implemented; independent validation pending |
| 21.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Implemented; independent validation pending |
| 21.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Implemented; independent validation pending |
| 21.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Implemented; independent validation pending |
| 21.5 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Implemented; independent validation pending |
| 21.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Implemented; independent validation pending |
| 21.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Implemented; independent validation pending |
| 21.8 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Implemented; independent validation pending |
| 21.9 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Implemented; independent validation pending |
| 21.10 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Implemented; independent validation pending |
| 21.11 | Required | Decidable | Single Translation Unit | No | Yes | Standard library usage | Implemented; independent validation pending |
| 21.12 | Advisory | Decidable | Single Translation Unit | No | Yes | Standard library usage | Implemented; independent validation pending |
| 22.1 | Required | Undecidable | System | Yes | Yes | Resource management | Implemented; independent validation pending |
| 22.2 | Mandatory | Undecidable | System | Yes | Yes | Resource management | Implemented; independent validation pending |
| 22.3 | Required | Undecidable | System | Yes | Yes | Resource management | Implemented; independent validation pending |
| 22.4 | Mandatory | Undecidable | System | Yes | Yes | Resource management | Implemented; independent validation pending |
| 22.5 | Mandatory | Undecidable | System | Yes | Yes | Resource management | Implemented; independent validation pending |
| 22.6 | Mandatory | Undecidable | System | Yes | Yes | Resource management | Implemented; independent validation pending |
