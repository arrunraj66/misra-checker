cmake_minimum_required(VERSION 3.20)

project(
  misra_checker
  VERSION 0.1.0
  DESCRIPTION "MISRA C checker and controlled converter"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(rules/c2012)

add_library(misra-core STATIC src/rule_registry.cpp)
target_include_directories(misra-core PUBLIC include)
target_link_libraries(misra-core PUBLIC misra-c2012-rules)

add_executable(misra-checker src/main.cpp)
target_link_libraries(misra-checker PRIVATE misra-core)

if(MSVC)
  target_compile_options(misra-checker PRIVATE /W4 /WX)
else()
  target_compile_options(
    misra-checker
    PRIVATE -Wall -Wextra -Wpedantic -Werror
  )
endif()

include(CTest)

if(BUILD_TESTING)
  add_test(NAME cli_version COMMAND misra-checker --version)

  add_executable(rule-registry-test tests/rule_registry_test.cpp)
  target_link_libraries(rule-registry-test PRIVATE misra-core)
  add_test(NAME rule_registry COMMAND rule-registry-test)
endif()
