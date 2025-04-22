
project(PythonWrappers)

set(CMAKE_CXX_STANDARD 17)

set(Python3_EXECUTABLE "/opt/homebrew/bin/python3.11" CACHE FILEPATH "Path to Python 3.11 interpreter")

find_package(pybind11 REQUIRED)

pybind11_add_module(python_wrappers
    python_wrappers.cpp
    ../src/NetworkLoader.cpp
    ../src/PowerFlowSolver.cpp
    ../src/NetworkAnalyzer.cpp
    ../src/solvers/GaussSeidelSolver.cpp
    ../src/solvers/BackwardForwardSweepSolver.cpp
    ../src/solvers/JacobiSolver.cpp
)




target_include_directories(python_wrappers
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(python_wrappers PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/examples
    OUTPUT_NAME "python_wrappers"
    SUFFIX ".so"
)

message(STATUS "Python module target configured: python_wrappers")
