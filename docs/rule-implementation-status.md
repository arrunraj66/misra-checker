# MISRA C:2012 rule implementation inventory

This catalog records the engineering structures present in the checker. It deliberately does not reproduce the copyrighted normative rule wording. The licensed MISRA specification is the controlled source for rule predicates, amplification, exceptions, and examples.

Current milestone: all 143 rule classes are **scaffolds**. A scaffold provides identity, metadata, an analysis plan, a factory, and a non-compliant-safe `NotImplemented` result. It is not a working compliance check.

| Rule | Category | Decidability | Scope | C90 | C99 | Topic | Status |
|---|---|---|---|---:|---:|---|---|
| 1.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard C conformance | Scaffold |
| 1.2 | Advisory | Undecidable | Single Translation Unit | Yes | Yes | Standard C conformance | Scaffold |
| 1.3 | Required | Undecidable | System | Yes | Yes | Standard C conformance | Scaffold |
| 2.1 | Required | Undecidable | System | Yes | Yes | Unused and unreachable code | Scaffold |
| 2.2 | Required | Undecidable | System | Yes | Yes | Unused and unreachable code | Scaffold |
| 2.3 | Advisory | Decidable | System | Yes | Yes | Unused and unreachable code | Scaffold |
| 2.4 | Advisory | Decidable | System | Yes | Yes | Unused and unreachable code | Scaffold |
| 2.5 | Advisory | Decidable | System | Yes | Yes | Unused and unreachable code | Scaffold |
| 2.6 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Unused and unreachable code | Scaffold |
| 2.7 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Unused and unreachable code | Scaffold |
| 3.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Comments | Scaffold |
| 3.2 | Required | Decidable | Single Translation Unit | No | Yes | Comments | Scaffold |
| 4.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Character sets and lexical elements | Scaffold |
| 4.2 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Character sets and lexical elements | Scaffold |
| 5.1 | Required | Decidable | System | Yes | Yes | Identifiers | Scaffold |
| 5.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Identifiers | Scaffold |
| 5.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Identifiers | Scaffold |
| 5.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Identifiers | Scaffold |
| 5.5 | Required | Decidable | Single Translation Unit | Yes | Yes | Identifiers | Scaffold |
| 5.6 | Required | Decidable | System | Yes | Yes | Identifiers | Scaffold |
| 5.7 | Required | Decidable | System | Yes | Yes | Identifiers | Scaffold |
| 5.8 | Required | Decidable | System | Yes | Yes | Identifiers | Scaffold |
| 5.9 | Advisory | Decidable | System | Yes | Yes | Identifiers | Scaffold |
| 6.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Types and bit-fields | Scaffold |
| 6.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Types and bit-fields | Scaffold |
| 7.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Literals and constants | Scaffold |
| 7.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Literals and constants | Scaffold |
| 7.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Literals and constants | Scaffold |
| 7.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Literals and constants | Scaffold |
| 8.1 | Required | Decidable | Single Translation Unit | Yes | No | Declarations and definitions | Scaffold |
| 8.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Declarations and definitions | Scaffold |
| 8.3 | Required | Decidable | System | Yes | Yes | Declarations and definitions | Scaffold |
| 8.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Declarations and definitions | Scaffold |
| 8.5 | Required | Decidable | System | Yes | Yes | Declarations and definitions | Scaffold |
| 8.6 | Required | Decidable | System | Yes | Yes | Declarations and definitions | Scaffold |
| 8.7 | Advisory | Decidable | System | Yes | Yes | Declarations and definitions | Scaffold |
| 8.8 | Required | Decidable | Single Translation Unit | Yes | Yes | Declarations and definitions | Scaffold |
| 8.9 | Advisory | Decidable | System | Yes | Yes | Declarations and definitions | Scaffold |
| 8.10 | Required | Decidable | Single Translation Unit | No | Yes | Declarations and definitions | Scaffold |
| 8.11 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Declarations and definitions | Scaffold |
| 8.12 | Required | Decidable | Single Translation Unit | Yes | Yes | Declarations and definitions | Scaffold |
| 8.13 | Advisory | Undecidable | System | Yes | Yes | Declarations and definitions | Scaffold |
| 8.14 | Required | Decidable | Single Translation Unit | No | Yes | Declarations and definitions | Scaffold |
| 9.1 | Mandatory | Undecidable | System | Yes | Yes | Initialization | Scaffold |
| 9.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Initialization | Scaffold |
| 9.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Initialization | Scaffold |
| 9.4 | Required | Decidable | Single Translation Unit | No | Yes | Initialization | Scaffold |
| 9.5 | Required | Decidable | Single Translation Unit | No | Yes | Initialization | Scaffold |
| 10.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Scaffold |
| 10.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Scaffold |
| 10.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Scaffold |
| 10.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Scaffold |
| 10.5 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Scaffold |
| 10.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Scaffold |
| 10.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Scaffold |
| 10.8 | Required | Decidable | Single Translation Unit | Yes | Yes | Essential type model | Scaffold |
| 11.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Scaffold |
| 11.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Scaffold |
| 11.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Scaffold |
| 11.4 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Scaffold |
| 11.5 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Scaffold |
| 11.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Scaffold |
| 11.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Scaffold |
| 11.8 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Scaffold |
| 11.9 | Required | Decidable | Single Translation Unit | Yes | Yes | Pointer conversions | Scaffold |
| 12.1 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Expressions | Scaffold |
| 12.2 | Required | Undecidable | System | Yes | Yes | Expressions | Scaffold |
| 12.3 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Expressions | Scaffold |
| 12.4 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Expressions | Scaffold |
| 13.1 | Required | Undecidable | System | No | Yes | Side effects and evaluation order | Scaffold |
| 13.2 | Required | Undecidable | System | Yes | Yes | Side effects and evaluation order | Scaffold |
| 13.3 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Side effects and evaluation order | Scaffold |
| 13.4 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Side effects and evaluation order | Scaffold |
| 13.5 | Required | Undecidable | System | Yes | Yes | Side effects and evaluation order | Scaffold |
| 13.6 | Mandatory | Decidable | Single Translation Unit | Yes | Yes | Side effects and evaluation order | Scaffold |
| 14.1 | Required | Undecidable | System | Yes | Yes | Control statement expressions | Scaffold |
| 14.2 | Required | Undecidable | System | Yes | Yes | Control statement expressions | Scaffold |
| 14.3 | Required | Undecidable | System | Yes | Yes | Control statement expressions | Scaffold |
| 14.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Control statement expressions | Scaffold |
| 15.1 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Control flow | Scaffold |
| 15.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Control flow | Scaffold |
| 15.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Control flow | Scaffold |
| 15.4 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Control flow | Scaffold |
| 15.5 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Control flow | Scaffold |
| 15.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Control flow | Scaffold |
| 15.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Control flow | Scaffold |
| 16.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Scaffold |
| 16.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Scaffold |
| 16.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Scaffold |
| 16.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Scaffold |
| 16.5 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Scaffold |
| 16.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Scaffold |
| 16.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Switch statements | Scaffold |
| 17.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Functions | Scaffold |
| 17.2 | Required | Undecidable | System | Yes | Yes | Functions | Scaffold |
| 17.3 | Mandatory | Decidable | Single Translation Unit | Yes | No | Functions | Scaffold |
| 17.4 | Mandatory | Decidable | Single Translation Unit | Yes | Yes | Functions | Scaffold |
| 17.5 | Advisory | Undecidable | System | Yes | Yes | Functions | Scaffold |
| 17.6 | Mandatory | Decidable | Single Translation Unit | No | Yes | Functions | Scaffold |
| 17.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Functions | Scaffold |
| 17.8 | Advisory | Undecidable | System | Yes | Yes | Functions | Scaffold |
| 18.1 | Required | Undecidable | System | Yes | Yes | Pointers and arrays | Scaffold |
| 18.2 | Required | Undecidable | System | Yes | Yes | Pointers and arrays | Scaffold |
| 18.3 | Required | Undecidable | System | Yes | Yes | Pointers and arrays | Scaffold |
| 18.4 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Pointers and arrays | Scaffold |
| 18.5 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Pointers and arrays | Scaffold |
| 18.6 | Required | Undecidable | System | Yes | Yes | Pointers and arrays | Scaffold |
| 18.7 | Required | Decidable | Single Translation Unit | No | Yes | Pointers and arrays | Scaffold |
| 18.8 | Required | Decidable | Single Translation Unit | No | Yes | Pointers and arrays | Scaffold |
| 19.1 | Mandatory | Undecidable | System | Yes | Yes | Overlapping storage | Scaffold |
| 19.2 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Overlapping storage | Scaffold |
| 20.1 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.5 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.8 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.9 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.10 | Advisory | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.11 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.12 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.13 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 20.14 | Required | Decidable | Single Translation Unit | Yes | Yes | Preprocessing directives | Scaffold |
| 21.1 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Scaffold |
| 21.2 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Scaffold |
| 21.3 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Scaffold |
| 21.4 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Scaffold |
| 21.5 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Scaffold |
| 21.6 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Scaffold |
| 21.7 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Scaffold |
| 21.8 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Scaffold |
| 21.9 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Scaffold |
| 21.10 | Required | Decidable | Single Translation Unit | Yes | Yes | Standard library usage | Scaffold |
| 21.11 | Required | Decidable | Single Translation Unit | No | Yes | Standard library usage | Scaffold |
| 21.12 | Advisory | Decidable | Single Translation Unit | No | Yes | Standard library usage | Scaffold |
| 22.1 | Required | Undecidable | System | Yes | Yes | Resource management | Scaffold |
| 22.2 | Mandatory | Undecidable | System | Yes | Yes | Resource management | Scaffold |
| 22.3 | Required | Undecidable | System | Yes | Yes | Resource management | Scaffold |
| 22.4 | Mandatory | Undecidable | System | Yes | Yes | Resource management | Scaffold |
| 22.5 | Mandatory | Undecidable | System | Yes | Yes | Resource management | Scaffold |
| 22.6 | Mandatory | Undecidable | System | Yes | Yes | Resource management | Scaffold |
