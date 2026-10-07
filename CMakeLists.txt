cmake_minimum_required(VERSION 3.20)

project(
  misra_checker
  VERSION 0.4.0
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

add_library(
  misra-clang STATIC
  src/clang_frontend.cpp src/clang_observations.cpp
  src/clang_observations_extra.cpp src/clang_observations_cfg.cpp
)
target_include_directories(misra-clang PUBLIC include)
target_include_directories(
  misra-clang
  SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS}
)
target_link_libraries(misra-clang PUBLIC clang-cpp LLVM)
misra_enable_warnings(misra-clang)

set(
  MISRA_LICENSE_HMAC_KEY "development-key-do-not-ship"
  CACHE STRING "HMAC key used to verify license files (set per release)"
)
add_library(misra-converter STATIC src/license.cpp src/converter.cpp)
target_include_directories(misra-converter PUBLIC include)
target_compile_definitions(
  misra-converter PRIVATE MISRA_LICENSE_HMAC_KEY="${MISRA_LICENSE_HMAC_KEY}"
)
target_link_libraries(misra-converter PUBLIC misra-core misra-clang)
misra_enable_warnings(misra-converter)

add_executable(misra-checker src/main.cpp)
target_link_libraries(misra-checker PRIVATE misra-core misra-clang misra-converter)
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

  add_executable(rule-15-3-test tests/rule_15_3_test.cpp)
  target_link_libraries(rule-15-3-test PRIVATE misra-core)
  misra_enable_warnings(rule-15-3-test)
  add_test(NAME rule_15_3 COMMAND rule-15-3-test)

  add_executable(rule-15-4-test tests/rule_15_4_test.cpp)
  target_link_libraries(rule-15-4-test PRIVATE misra-core)
  misra_enable_warnings(rule-15-4-test)
  add_test(NAME rule_15_4 COMMAND rule-15-4-test)

  set(
    MISRA_RULE_15_1_FIXTURE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/rule_15_1"
  )
  set(
    MISRA_RULE_15_2_FIXTURE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/rule_15_2"
  )
  set(
    MISRA_RULE_15_3_FIXTURE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/rule_15_3"
  )
  set(
    MISRA_RULE_15_4_FIXTURE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/rule_15_4"
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
      "${CMAKE_COMMAND}"
      "-DCHECKER=$<TARGET_FILE:misra-checker>"
      "-DDATABASE=${CMAKE_CURRENT_BINARY_DIR}/tests/compile_commands.json"
      "-DSOURCE=${MISRA_RULE_15_1_FIXTURE_DIR}/compliant.c"
      "-DUNEXPECTED_DIAGNOSTIC=misra-c2012-15\\.1-"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_no_diagnostic.cmake"
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

  add_test(
    NAME cli_analyze_rule_15_3_compliant
    COMMAND
      "${CMAKE_COMMAND}"
      "-DCHECKER=$<TARGET_FILE:misra-checker>"
      "-DDATABASE=${CMAKE_CURRENT_BINARY_DIR}/tests/compile_commands.json"
      "-DSOURCE=${MISRA_RULE_15_3_FIXTURE_DIR}/compliant.c"
      "-DUNEXPECTED_DIAGNOSTIC=misra-c2012-15.3"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_no_diagnostic.cmake"
  )
  foreach(fixture into_block sibling_block)
    add_test(
      NAME cli_analyze_rule_15_3_${fixture}
      COMMAND
        "${CMAKE_COMMAND}"
        "-DCHECKER=$<TARGET_FILE:misra-checker>"
        "-DDATABASE=${CMAKE_CURRENT_BINARY_DIR}/tests/compile_commands.json"
        "-DSOURCE=${MISRA_RULE_15_3_FIXTURE_DIR}/${fixture}.c"
        "-DEXPECTED_DIAGNOSTIC=misra-c2012-15.3-label-not-in-enclosing-block"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_violation.cmake"
    )
  endforeach()

  add_test(
    NAME cli_analyze_rule_15_4_compliant
    COMMAND
      "${CMAKE_COMMAND}"
      "-DCHECKER=$<TARGET_FILE:misra-checker>"
      "-DDATABASE=${CMAKE_CURRENT_BINARY_DIR}/tests/compile_commands.json"
      "-DSOURCE=${MISRA_RULE_15_4_FIXTURE_DIR}/compliant.c"
      "-DUNEXPECTED_DIAGNOSTIC=misra-c2012-15.4"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_no_diagnostic.cmake"
  )
  foreach(fixture two_breaks break_and_goto)
    add_test(
      NAME cli_analyze_rule_15_4_${fixture}
      COMMAND
        "${CMAKE_COMMAND}"
        "-DCHECKER=$<TARGET_FILE:misra-checker>"
        "-DDATABASE=${CMAKE_CURRENT_BINARY_DIR}/tests/compile_commands.json"
        "-DSOURCE=${MISRA_RULE_15_4_FIXTURE_DIR}/${fixture}.c"
        "-DEXPECTED_DIAGNOSTIC=misra-c2012-15.4-multiple-loop-exits"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_violation.cmake"
    )
  endforeach()

  add_executable(license-test tests/license_test.cpp)
  target_link_libraries(license-test PRIVATE misra-converter)
  misra_enable_warnings(license-test)
  add_test(NAME license COMMAND license-test)

  add_executable(converter-test tests/converter_test.cpp)
  target_link_libraries(converter-test PRIVATE misra-converter)
  misra_enable_warnings(converter-test)
  add_test(NAME converter COMMAND converter-test)

  # Test license signed with the build's HMAC key.
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  set(MISRA_TEST_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/tests/test.lic")
  file(
    WRITE "${CMAKE_CURRENT_BINARY_DIR}/tests/test.key"
    "${MISRA_LICENSE_HMAC_KEY}"
  )
  execute_process(
    COMMAND
      "${Python3_EXECUTABLE}" -I
      "${CMAKE_CURRENT_SOURCE_DIR}/scripts/issue_license.py"
      --customer ctest --expires 2999-12-31 --features convert
      --key-file "${CMAKE_CURRENT_BINARY_DIR}/tests/test.key"
    OUTPUT_FILE "${MISRA_TEST_LICENSE}"
    COMMAND_ERROR_IS_FATAL ANY
  )

  foreach(mode good bad broken apply nolicense literal)
    add_test(
      NAME convert_${mode}
      COMMAND
        "${CMAKE_COMMAND}"
        "-DCHECKER=$<TARGET_FILE:misra-checker>"
        "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/convert"
        "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/convert_work_${mode}"
        "-DMODE=${mode}"
        "-DLICENSE=${MISRA_TEST_LICENSE}"
        "-DCLANG_EXECUTABLE=${CLANG_EXECUTABLE}"
        "-DPROVIDER_SCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/scripts/ai_providers/deterministic_provider.py"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/convert_test.cmake"
    )
  endforeach()

  # Observation-based rules: every tests/fixtures/obs/rX_Y_{bad,ok}.c file is
  # compiled via a generated database; *_bad must report the rule, *_ok must not.
  file(GLOB MISRA_OBS_FIXTURES CONFIGURE_DEPENDS
       "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/obs/*.c")
  set(MISRA_OBS_DB "${CMAKE_CURRENT_BINARY_DIR}/tests/obs_compile_commands.json")
  set(MISRA_OBS_JSON "[\n")
  set(MISRA_OBS_SEP "")
  foreach(fixture ${MISRA_OBS_FIXTURES})
    get_filename_component(fixture_dir "${fixture}" DIRECTORY)
    string(
      APPEND MISRA_OBS_JSON
      "${MISRA_OBS_SEP}  {\"directory\": \"${fixture_dir}\", \"file\": \"${fixture}\", "
      "\"arguments\": [\"${CLANG_EXECUTABLE}\", \"-std=c99\", \"-c\", \"${fixture}\"]}"
    )
    set(MISRA_OBS_SEP ",\n")
  endforeach()
  string(APPEND MISRA_OBS_JSON "\n]\n")
  file(WRITE "${MISRA_OBS_DB}" "${MISRA_OBS_JSON}")
  foreach(fixture ${MISRA_OBS_FIXTURES})
    get_filename_component(stem "${fixture}" NAME_WE)
    string(REGEX MATCH "^r([0-9]+)_([0-9]+)(_[a-z0-9]+)?_(bad|ok)$" matched "${stem}")
    if(NOT matched)
      continue()
    endif()
    set(rule_pattern "misra-c2012-${CMAKE_MATCH_1}\\.${CMAKE_MATCH_2}-")
    if(CMAKE_MATCH_4 STREQUAL "bad")
      add_test(
        NAME obs_${stem}
        COMMAND
          "${CMAKE_COMMAND}"
          "-DCHECKER=$<TARGET_FILE:misra-checker>"
          "-DDATABASE=${MISRA_OBS_DB}"
          "-DSOURCE=${fixture}"
          "-DEXPECTED_DIAGNOSTIC=${rule_pattern}"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_violation.cmake"
      )
    else()
      add_test(
        NAME obs_${stem}
        COMMAND
          "${CMAKE_COMMAND}"
          "-DCHECKER=$<TARGET_FILE:misra-checker>"
          "-DDATABASE=${MISRA_OBS_DB}"
          "-DSOURCE=${fixture}"
          "-DUNEXPECTED_DIAGNOSTIC=${rule_pattern}"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_no_diagnostic.cmake"
      )
    endif()
  endforeach()

  # Whole-program fixtures: each directory under tests/fixtures/sys is one
  # program; all of its .c files are analyzed together.
  file(GLOB MISRA_SYS_CASES LIST_DIRECTORIES true CONFIGURE_DEPENDS
       "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/sys/r*")
  foreach(case_dir ${MISRA_SYS_CASES})
    if(NOT IS_DIRECTORY "${case_dir}")
      continue()
    endif()
    get_filename_component(case_name "${case_dir}" NAME)
    string(REGEX MATCH "^r([0-9]+)_([0-9]+)_(bad|ok)$" matched "${case_name}")
    if(NOT matched)
      continue()
    endif()
    set(rule_pattern "misra-c2012-${CMAKE_MATCH_1}\\.${CMAKE_MATCH_2}-")
    set(case_kind "${CMAKE_MATCH_3}")
    file(GLOB case_sources "${case_dir}/*.c")
    set(case_json "[\n")
    set(case_sep "")
    foreach(source ${case_sources})
      string(
        APPEND case_json
        "${case_sep}  {\"directory\": \"${case_dir}\", \"file\": \"${source}\", "
        "\"arguments\": [\"${CLANG_EXECUTABLE}\", \"-std=c99\", \"-c\", \"${source}\"]}"
      )
      set(case_sep ",\n")
    endforeach()
    string(APPEND case_json "\n]\n")
    set(case_db "${CMAKE_CURRENT_BINARY_DIR}/tests/sys_${case_name}.json")
    file(WRITE "${case_db}" "${case_json}")
    if(case_kind STREQUAL "bad")
      add_test(
        NAME sys_${case_name}
        COMMAND
          "${CMAKE_COMMAND}"
          "-DCHECKER=$<TARGET_FILE:misra-checker>"
          "-DDATABASE=${case_db}"
          "-DEXPECTED_DIAGNOSTIC=${rule_pattern}"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_violation.cmake"
      )
    else()
      add_test(
        NAME sys_${case_name}
        COMMAND
          "${CMAKE_COMMAND}"
          "-DCHECKER=$<TARGET_FILE:misra-checker>"
          "-DDATABASE=${case_db}"
          "-DUNEXPECTED_DIAGNOSTIC=${rule_pattern}"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_no_diagnostic.cmake"
      )
    endif()
  endforeach()
endif()
