cmake_minimum_required(VERSION 3.20)

project(
  misra_checker
  VERSION 0.3.0
  DESCRIPTION "MISRA C checker and controlled converter"
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(LLVM 14 CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED HINTS "${LLVM_DIR}/../clang")
find_program(CLANG_EXECUTABLE NAMES clang-14 clang REQUIRED)

message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION} from ${LLVM_DIR}")
message(STATUS "Using Clang frontend: ${CLANG_EXECUTABLE}")

function(misra_enable_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  else()
    target_compile_options(
      ${target}
      PRIVATE -Wall -Wextra -Wpedantic -Werror
    )
  endif()
endfunction()

add_subdirectory(rules/c2012)

add_library(misra-core STATIC src/rule_registry.cpp)
target_include_directories(misra-core PUBLIC include)
target_link_libraries(misra-core PUBLIC misra-c2012-rules)
misra_enable_warnings(misra-core)

add_library(misra-clang STATIC src/clang_frontend.cpp)
target_include_directories(misra-clang PUBLIC include)
target_include_directories(
  misra-clang
  SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS}
)
target_link_libraries(misra-clang PUBLIC clang-cpp LLVM)
misra_enable_warnings(misra-clang)

add_executable(misra-checker src/main.cpp)
target_link_libraries(misra-checker PRIVATE misra-core misra-clang)
misra_enable_warnings(misra-checker)

include(CTest)

if(BUILD_TESTING)
  add_test(NAME cli_version COMMAND misra-checker --version)

  add_executable(rule-registry-test tests/rule_registry_test.cpp)
  target_link_libraries(rule-registry-test PRIVATE misra-core)
  misra_enable_warnings(rule-registry-test)
  add_test(NAME rule_registry COMMAND rule-registry-test)

  add_executable(rule-15-1-test tests/rule_15_1_test.cpp)
  target_link_libraries(rule-15-1-test PRIVATE misra-core)
  misra_enable_warnings(rule-15-1-test)
  add_test(NAME rule_15_1 COMMAND rule-15-1-test)

  add_executable(rule-15-2-test tests/rule_15_2_test.cpp)
  target_link_libraries(rule-15-2-test PRIVATE misra-core)
  misra_enable_warnings(rule-15-2-test)
  add_test(NAME rule_15_2 COMMAND rule-15-2-test)

  set(
    MISRA_RULE_15_1_FIXTURE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/rule_15_1"
  )
  set(
    MISRA_RULE_15_2_FIXTURE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/rule_15_2"
  )
  configure_file(
    tests/fixtures/compile_commands.json.in
    tests/compile_commands.json
    @ONLY
  )

  add_executable(clang-frontend-test tests/clang_frontend_test.cpp)
  target_link_libraries(clang-frontend-test PRIVATE misra-clang)
  misra_enable_warnings(clang-frontend-test)
  add_test(
    NAME clang_frontend
    COMMAND
      clang-frontend-test
      "${CMAKE_CURRENT_BINARY_DIR}/tests/compile_commands.json"
      "${MISRA_RULE_15_1_FIXTURE_DIR}/compliant.c"
      "${MISRA_RULE_15_1_FIXTURE_DIR}/violating.c"
      "${MISRA_RULE_15_1_FIXTURE_DIR}/macro_expansion.c"
      "${MISRA_RULE_15_1_FIXTURE_DIR}/lexical_boundary.c"
  )

  add_executable(
    rule-15-2-integration-test
    tests/rule_15_2_integration_test.cpp
  )
  target_link_libraries(
    rule-15-2-integration-test
    PRIVATE misra-core misra-clang
  )
  misra_enable_warnings(rule-15-2-integration-test)
  add_test(
    NAME rule_15_2_integration
    COMMAND
      rule-15-2-integration-test
      "${CMAKE_CURRENT_BINARY_DIR}/tests/compile_commands.json"
      "${MISRA_RULE_15_2_FIXTURE_DIR}/forward.c"
      "${MISRA_RULE_15_2_FIXTURE_DIR}/backward.c"
      "${MISRA_RULE_15_2_FIXTURE_DIR}/macro_forward.c"
      "${MISRA_RULE_15_2_FIXTURE_DIR}/macro_backward.c"
  )

  add_test(
    NAME cli_analyze_compliant
    COMMAND
      misra-checker analyze
      --compile-commands "${CMAKE_CURRENT_BINARY_DIR}/tests/compile_commands.json"
      --file "${MISRA_RULE_15_1_FIXTURE_DIR}/compliant.c"
  )
  set_tests_properties(
    cli_analyze_compliant
    PROPERTIES PASS_REGULAR_EXPRESSION "0 finding\\(s\\)"
  )

  add_test(
    NAME cli_analyze_violation
    COMMAND
      "${CMAKE_COMMAND}"
      "-DCHECKER=$<TARGET_FILE:misra-checker>"
      "-DDATABASE=${CMAKE_CURRENT_BINARY_DIR}/tests/compile_commands.json"
      "-DSOURCE=${MISRA_RULE_15_1_FIXTURE_DIR}/violating.c"
      "-DEXPECTED_DIAGNOSTIC=misra-c2012-15.1-goto"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_violation.cmake"
  )

  add_test(
    NAME cli_analyze_rule_15_2_violation
    COMMAND
      "${CMAKE_COMMAND}"
      "-DCHECKER=$<TARGET_FILE:misra-checker>"
      "-DDATABASE=${CMAKE_CURRENT_BINARY_DIR}/tests/compile_commands.json"
      "-DSOURCE=${MISRA_RULE_15_2_FIXTURE_DIR}/backward.c"
      "-DEXPECTED_DIAGNOSTIC=misra-c2012-15.2-backward-goto"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_violation.cmake"
  )
endif()
