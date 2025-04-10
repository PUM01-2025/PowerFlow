cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(PowerFlow VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
enable_testing()

#add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(src)
add_subdirectory(matlab)
add_subdirectory(standalone)
add_subdirectory(tests)
