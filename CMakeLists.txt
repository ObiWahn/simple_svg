cmake_minimum_required(VERSION 3.11)
project(simple_svg)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(simple_svg INTERFACE)
target_include_directories(simple_svg INTERFACE include)

add_executable(simple-svg-example EXCLUDE_FROM_ALL ./example/main.cpp)
target_link_libraries(simple-svg-example simple_svg)
configure_file(example/svg-test.html svg-test.html COPYONLY)
