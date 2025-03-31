cmake_minimum_required(VERSION 3.10.0)
project(PowerFlow VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
include_directories(include) # Om fel testa att kommentera bort denna
enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
