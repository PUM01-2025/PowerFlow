add_executable(PowerFlowStandalone main.cpp)
target_include_directories(PowerFlowLib PUBLIC ${PROJECT_SOURCE_DIR}/include PRIVATE ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(PowerFlowStandalone PRIVATE PowerFlowLib)