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

add_executable(misra-checker src/main.cpp)

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
endif()
