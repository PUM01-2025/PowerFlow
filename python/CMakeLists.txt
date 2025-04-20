
pybind11_add_module(python_wrappers python_wrappers.cpp)

set_target_properties(python_wrappers PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "examples"
    OUTPUT_NAME "python_wrappers"              
    SUFFIX ".so"                               
)