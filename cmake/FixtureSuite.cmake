# misra_fixture_suite(<name> <fixture-dir> <case>...)
#
# Builds a compilation database from every *.c below <fixture-dir> and registers
# one data-driven test. A case is "<rule-id>|<file>[,<file>...]|<findings>":
# the named files (relative to <fixture-dir>) are analysed together as one
# program and the rule must report exactly <findings> findings.
function(misra_fixture_suite name dir)
  # MISRA_FIXTURE_STD selects the language mode (default C99); some rules can
  # only be exercised in C90, where the compiler still accepts the construct.
  if(DEFINED MISRA_FIXTURE_STD)
    set(std "${MISRA_FIXTURE_STD}")
  else()
    set(std "c99")
  endif()
  # MISRA_FIXTURE_FLAGS: extra compiler arguments, as a quoted JSON fragment
  # such as ",\"-Wno-error=return-type\"".
  if(NOT DEFINED MISRA_FIXTURE_FLAGS)
    set(MISRA_FIXTURE_FLAGS "")
  endif()
  file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${dir}/*.c")
  set(json "[")
  set(sep "")
  foreach(source IN LISTS sources)
    get_filename_component(source_dir "${source}" DIRECTORY)
    string(
      APPEND json
      "${sep}{\"directory\":\"${source_dir}\","
      "\"file\":\"${source}\","
      "\"arguments\":[\"${CLANG_EXECUTABLE}\",\"-std=${std}\","
      "\"-I\",\"${source_dir}\"${MISRA_FIXTURE_FLAGS},\"-c\",\"${source}\"]}"
    )
    set(sep ",")
  endforeach()
  string(APPEND json "]")
  set(database "${CMAKE_CURRENT_BINARY_DIR}/tests/${name}_compile_commands.json")
  file(WRITE "${database}" "${json}")

  set(test_args "")
  foreach(case IN LISTS ARGN)
    string(REPLACE "|" ";" parts "${case}")
    list(GET parts 0 rule)
    list(GET parts 1 files)
    list(GET parts 2 count)
    string(REPLACE "," ";" file_list "${files}")
    set(joined "")
    set(file_sep "")
    foreach(file IN LISTS file_list)
      string(APPEND joined "${file_sep}${dir}/${file}")
      set(file_sep ",")
    endforeach()
    list(APPEND test_args "${rule}" "${joined}" "${count}")
  endforeach()

  add_executable(${name}-test tests/rules_integration_test.cpp)
  target_link_libraries(${name}-test PRIVATE misra-core misra-clang)
  misra_enable_warnings(${name}-test)
  add_test(NAME ${name} COMMAND ${name}-test "${database}" ${test_args})
endfunction()
