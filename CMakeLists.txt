cmake_minimum_required(VERSION 2.8)
project(libpypa)

add_subdirectory(src)
add_subdirectory(test)

# check
add_custom_target(check-libpypa COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure DEPENDS pypa parser-test lexer-test WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test)
