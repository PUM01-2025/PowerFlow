add_executable(UnitTests main.cpp)
target_link_libraries(UnitTests PRIVATE PowerFlowLib)
add_test(NAME UnitTests COMMAND UnitTests)
