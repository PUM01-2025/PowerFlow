cmake_minimum_required(VERSION 3.10.0)
project(PowerFlow VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
enable_testing()

add_subdirectory(src)
add_subdirectory(matlab)
add_subdirectory(standalone)
add_subdirectory(tests)
