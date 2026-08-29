execute_process(
  COMMAND
    "${CHECKER}" analyze
    --compile-commands "${DATABASE}"
    --file "${SOURCE}"
  RESULT_VARIABLE checker_result
  OUTPUT_VARIABLE checker_output
  ERROR_VARIABLE checker_error
)

if(NOT checker_result EQUAL 1)
  message(
    FATAL_ERROR
    "Expected finding exit code 1, received ${checker_result}.\n"
    "stdout:\n${checker_output}\n"
    "stderr:\n${checker_error}"
  )
endif()

if(NOT checker_output MATCHES "${EXPECTED_DIAGNOSTIC}")
  message(
    FATAL_ERROR
    "Expected diagnostic ${EXPECTED_DIAGNOSTIC}.\n${checker_output}"
  )
endif()
