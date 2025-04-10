cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(PowerFlow VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
enable_testing()

# Add compiler flags conditionally
if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Matlab COMPONENTS MX MEX)

if(Matlab_FOUND)
    message(STATUS "Matlab found: Adding matlab subdirectory.")
    add_subdirectory(matlab)
else()
    message(WARNING "Matlab not found: Skipping matlab subdirectory.")
endif()


add_subdirectory(src)
add_subdirectory(standalone)
add_subdirectory(tests)
 