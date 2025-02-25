add_library(PowerFlowLib add.cpp add.h)
target_include_directories(PowerFlowLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(PowerFlow main.cpp)
target_link_libraries(PowerFlow PRIVATE PowerFlowLib)
