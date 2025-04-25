
find_package(pybind11 REQUIRED)

pybind11_add_module(python_wrappers
    python_wrappers.cpp
)

target_link_libraries(python_wrappers PRIVATE PowerFlowLib)

set_target_properties(python_wrappers PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/examples
    OUTPUT_NAME "python_wrappers"
    SUFFIX ".so"
)

message(STATUS "Python module target configured: python_wrappers")
