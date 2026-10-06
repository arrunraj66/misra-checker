execute_process(
  COMMAND "${CHECKER}" analyze --compile-commands "${DATABASE}" --file "${SOURCE}"
  RESULT_VARIABLE checker_result
  OUTPUT_VARIABLE checker_output
  ERROR_VARIABLE checker_error
)
if(checker_result GREATER 1)
  message(FATAL_ERROR "analysis failed (${checker_result}):\n${checker_error}")
endif()
if(checker_output MATCHES "${UNEXPECTED_DIAGNOSTIC}")
  message(FATAL_ERROR "Unexpected diagnostic ${UNEXPECTED_DIAGNOSTIC}:\n${checker_output}")
endif()
