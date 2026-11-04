# Data-driven fixture cases: "<rule>|<files>|<expected findings>".
# Files of one case are analysed together as one program.

misra_fixture_suite(
  rules_batch2 "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/rules_batch2"
  "15.6|r15_6_compliant.c|0" "15.6|r15_6_violating.c|5"
  "15.7|r15_7_compliant.c|0" "15.7|r15_7_violating.c|1"
  "16.4|r16_4_compliant.c|0" "16.4|r16_4_violating.c|1"
  "21.3|r21_3_compliant.c|0" "21.3|r21_3_violating.c|3"
  "21.7|r21_7_compliant.c|0" "21.7|r21_7_violating.c|2"
  "21.8|r21_8_compliant.c|0" "21.8|r21_8_violating.c|2"
  "21.9|r21_9_compliant.c|0" "21.9|r21_9_violating.c|1"
)

misra_fixture_suite(
  rules_syntax "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/rules_syntax"
  "15.4|r15_4_compliant.c|0" "15.4|r15_4_violating.c|1"
  "15.5|r15_5_compliant.c|0" "15.5|r15_5_violating.c|2"
  "14.4|r14_4_compliant.c|0" "14.4|r14_4_violating.c|6"
  "12.3|r12_3_compliant.c|0" "12.3|r12_3_violating.c|3"
  "13.4|r13_4_compliant.c|0" "13.4|r13_4_violating.c|4"
  "7.1|r07_1_compliant.c|0"  "7.1|r07_1_violating.c|2"
  "7.3|r07_3_compliant.c|0"  "7.3|r07_3_violating.c|3"
  "18.8|r18_8_compliant.c|0" "18.8|r18_8_violating.c|1"
  "21.4|r21_4_compliant.c|0" "21.4|r21_4_violating.c|1"
  "21.5|r21_5_compliant.c|0" "21.5|r21_5_violating.c|1"
  "21.11|r21_11_compliant.c|0" "21.11|r21_11_violating.c|1"
  "21.6|r21_6_compliant.c|0" "21.6|r21_6_violating.c|2"
  "21.10|r21_10_compliant.c|0" "21.10|r21_10_violating.c|2"
  "8.2|r08_2_compliant.c|0"  "8.2|r08_2_violating.c|3"
  "8.4|r08_4_compliant.c|0"  "8.4|r08_4_violating.c|2"
  "8.8|r08_8_compliant.c|0"  "8.8|r08_8_violating.c|2"
  "8.9|r08_9_compliant.c|0"  "8.9|r08_9_violating.c|1"
  "8.10|r08_10_compliant.c|0" "8.10|r08_10_violating.c|1"
  "8.11|r08_11_compliant.c|0" "8.11|r08_11_violating.c|1"
  "8.12|r08_12_compliant.c|0" "8.12|r08_12_violating.c|1"
  "8.14|r08_14_compliant.c|0" "8.14|r08_14_violating.c|1"
  "2.6|r02_6_compliant.c|0"  "2.6|r02_6_violating.c|1"
  "2.7|r02_7_compliant.c|0"  "2.7|r02_7_violating.c|1"
  "5.2|r05_2_compliant.c|0" "5.2|r05_2_violating.c|2"
  "5.3|r05_3_compliant.c|0" "5.3|r05_3_violating.c|5"
  "10.1|r10_1_compliant.c|0" "10.1|r10_1_violating.c|5"
  "10.2|r10_2_compliant.c|0" "10.2|r10_2_violating.c|2"
  "10.3|r10_3_compliant.c|0" "10.3|r10_3_violating.c|7"
  "10.4|r10_4_compliant.c|0" "10.4|r10_4_violating.c|6"
  "10.6|r10_6_compliant.c|0" "10.6|r10_6_violating.c|3"
  "10.7|r10_7_compliant.c|0" "10.7|r10_7_violating.c|2"
  "10.8|r10_8_compliant.c|0" "10.8|r10_8_violating.c|3"
  "11.1|r11_1_compliant.c|0" "11.1|r11_1_violating.c|3"
  "11.2|r11_2_compliant.c|0" "11.2|r11_2_violating.c|2"
  "11.3|r11_3_compliant.c|0" "11.3|r11_3_violating.c|3"
  "11.4|r11_4_compliant.c|0" "11.4|r11_4_violating.c|2"
  "11.5|r11_5_compliant.c|0" "11.5|r11_5_violating.c|2"
  "11.6|r11_6_compliant.c|0" "11.6|r11_6_violating.c|2"
  "11.8|r11_8_compliant.c|0" "11.8|r11_8_violating.c|3"
  "11.9|r11_9_compliant.c|0" "11.9|r11_9_violating.c|3"
  "17.1|r17_1_compliant.c|0" "17.1|r17_1_violating.c|2"
  "17.2|r17_2_compliant.c|0" "17.2|r17_2_violating.c|3"
  "17.4|r17_4_compliant.c|0" "17.4|r17_4_violating.c|2"
  # Recursion through function pointers is Possible, not Definite.
  "17.2|r17_2_callback_compliant.c|0"
  "17.2|r17_2_callback_a_violating.c|2" "17.2|r17_2_callback_b_violating.c|2"
  # Constructs Clang rejects in C99 (implicit declaration, bare return) are
  # still analysed: the frontend downgrades those errors to warnings.
  "17.3|r17_3_c99_compliant.c|0" "17.3|r17_3_c99_violating.c|1"
  "17.4|r17_4_bare_compliant.c|0" "17.4|r17_4_bare_violating.c|1"
  "17.5|r17_5_compliant.c|0" "17.5|r17_5_violating.c|1"
  "17.6|r17_6_compliant.c|0" "17.6|r17_6_violating.c|1"
  "17.7|r17_7_compliant.c|0" "17.7|r17_7_violating.c|3"
  "17.8|r17_8_compliant.c|0" "17.8|r17_8_violating.c|4"
  "16.2|r16_2_compliant.c|0" "16.2|r16_2_violating.c|1"
  "16.3|r16_3_compliant.c|0" "16.3|r16_3_violating.c|2"
  "16.5|r16_5_compliant.c|0" "16.5|r16_5_violating.c|1"
  "16.6|r16_6_compliant.c|0" "16.6|r16_6_violating.c|2"
  "16.7|r16_7_compliant.c|0" "16.7|r16_7_violating.c|2"
  "12.2|r12_2_compliant.c|0" "12.2|r12_2_violating.c|5"
  "12.4|r12_4_compliant.c|0" "12.4|r12_4_violating.c|4"
  "9.1|r09_1_compliant.c|0" "9.1|r09_1_violating.c|2"
  "9.2|r09_2_compliant.c|0" "9.2|r09_2_violating.c|3"
  "9.3|r09_3_compliant.c|0" "9.3|r09_3_violating.c|3"
  "9.4|r09_4_compliant.c|0" "9.4|r09_4_violating.c|2"
  "9.5|r09_5_compliant.c|0" "9.5|r09_5_violating.c|1"
  "8.1|r08_1_compliant.c|0" "8.1|r08_1_violating.c|2"
  "8.13|r08_13_compliant.c|0" "8.13|r08_13_violating.c|4"
  "20.1|r20_1_compliant.c|0" "20.1|r20_1_violating.c|1"
  "20.2|r20_2_compliant.c|0" "20.2|r20_2_violating.c|2"
  "20.3|r20_3_compliant.c|0" "20.3|r20_3_violating.c|1"
  "20.4|r20_4_compliant.c|0" "20.4|r20_4_violating.c|2"
  "20.5|r20_5_compliant.c|0" "20.5|r20_5_violating.c|1"
  "20.7|r20_7_compliant.c|0" "20.7|r20_7_violating.c|3"
  "20.9|r20_9_compliant.c|0" "20.9|r20_9_violating.c|2"
  "20.10|r20_10_compliant.c|0" "20.10|r20_10_violating.c|2"
  "20.11|r20_11_compliant.c|0" "20.11|r20_11_violating.c|1"
  "20.12|r20_12_compliant.c|0" "20.12|r20_12_violating.c|1"
  "20.13|r20_13_compliant.c|0" "20.13|r20_13_violating.c|1"
  # Macro policy: the comma produced by PAIR() is found; the octal literal
  # inside a system-header macro (S_IRUSR) is not.
  "12.3|macro_origin.c|1" "7.1|macro_origin.c|0"
)

# C90 suite: the same constructs in the language mode that actually accepts
# them (no downgrade of compiler errors is involved here).
set(MISRA_FIXTURE_STD c90)
misra_fixture_suite(
  rules_c90 "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/rules_c90"
  "17.3|r17_3_compliant.c|0" "17.3|r17_3_violating.c|1"
  "17.4|r17_4_return_compliant.c|0" "17.4|r17_4_return_violating.c|1"
)
unset(MISRA_FIXTURE_STD)

misra_fixture_suite(
  rules_program "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/rules_program"
  "5.1|p5_1_a.c,p5_1_b.c|1" "5.1|p5_1_ok_a.c,p5_1_ok_b.c|0"
  "5.4|p5_4_viol.c|1" "5.4|p5_4_ok.c|0"
  "5.5|p5_5_viol.c|1" "5.5|p5_5_ok.c|0"
  "5.6|p5_6_a.c,p5_6_b.c|1" "5.6|p5_6_ok_a.c,p5_6_ok_b.c|0"
  "5.7|p5_7_a.c,p5_7_b.c|1" "5.7|p5_7_ok_a.c,p5_7_ok_b.c|0"
  "5.8|p5_8_a.c,p5_8_b.c|1" "5.8|p5_8_ok_a.c,p5_8_ok_b.c|0"
  "5.9|p5_9_a.c,p5_9_b.c|1" "5.9|p5_9_ok_a.c,p5_9_ok_b.c|0"
  "2.3|p2_3_viol.c|1" "2.3|p2_3_ok_a.c,p2_3_ok_b.c|0"
  "2.4|p2_4_viol.c|2" "2.4|p2_4_ok.c|0"
  "2.5|p2_5_viol.c|1" "2.5|p2_5_ok.c|0"
  "8.3|p8_3_a.c,p8_3_b.c|2" "8.3|p8_3_ok_a.c,p8_3_ok_b.c|0"
  "8.3|p8_3_arr_ok_a.c,p8_3_arr_ok_b.c|0" "8.3|p8_3_arr_bad_a.c,p8_3_arr_bad_b.c|1"
  "8.5|p8_5_a.c,p8_5_b.c,p8_5_c.c|1" "8.5|p8_5_ok_a.c,p8_5_ok_b.c|0"
  "8.6|p8_6_a.c,p8_6_b.c|1" "8.6|p8_6_missing.c|1" "8.6|p8_6_ok_a.c,p8_6_ok_b.c|0"
  "8.7|p8_7_viol.c|2" "8.7|p8_7_ok_a.c,p8_7_ok_b.c|0"
)

# CLI behaviour of the macro policy.
set(
  MISRA_MACRO_FIXTURE
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/rules_syntax/macro_origin.c"
)
add_test(
  NAME cli_macro_tag
  COMMAND
    misra-checker analyze
    --compile-commands
    "${CMAKE_CURRENT_BINARY_DIR}/tests/rules_syntax_compile_commands.json"
    --file "${MISRA_MACRO_FIXTURE}"
)
set_tests_properties(
  cli_macro_tag PROPERTIES
  PASS_REGULAR_EXPRESSION "Rule 12.3 \\[misra-c2012-12.3-comma\\] \\[macro\\]"
)
add_test(
  NAME cli_macro_suppress
  COMMAND
    misra-checker analyze
    --compile-commands
    "${CMAKE_CURRENT_BINARY_DIR}/tests/rules_syntax_compile_commands.json"
    --file "${MISRA_MACRO_FIXTURE}" --suppress-macro-findings
)
set_tests_properties(
  cli_macro_suppress PROPERTIES
  PASS_REGULAR_EXPRESSION "suppressed; the result is for triage only"
  FAIL_REGULAR_EXPRESSION "12\\.3-comma"
)
