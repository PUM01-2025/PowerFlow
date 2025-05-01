pybind11_add_module(PowerFlowPython PowerFlowPython.cpp)

target_link_libraries(PowerFlowPython PRIVATE PowerFlowLib)

set_target_properties(PowerFlowPython PROPERTIES
    OUTPUT_NAME "PowerFlowPython"
)

message(STATUS "Python module target configured: PowerFlowPython")
