cmake_minimum_required(VERSION 3.16)
project(linea_mini_tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMPONENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../components/linea_mini)

add_executable(test_gicar
  test_gicar.cpp
  ${COMPONENT_DIR}/gicar_protocol.cpp
  ${COMPONENT_DIR}/controller.cpp)
target_include_directories(test_gicar PRIVATE ${COMPONENT_DIR})
target_compile_definitions(test_gicar PRIVATE
  FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
target_compile_options(test_gicar PRIVATE -Wall -Wextra -Werror)

enable_testing()
add_test(NAME gicar COMMAND test_gicar)
