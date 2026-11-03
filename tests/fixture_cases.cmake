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
  # Macro policy: the comma produced by PAIR() is found; the octal literal
  # inside a system-header macro (S_IRUSR) is not.
  "12.3|macro_origin.c|1" "7.1|macro_origin.c|0"
)

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
