matlab_add_mex(NAME PowerFlowMex SRC PowerFlowMex.cpp LINK_TO PowerFlowLib)

# Copy the PowerFlow.m file to the build directory.
add_custom_command(
        TARGET PowerFlowMex POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/PowerFlow.m
                ${CMAKE_CURRENT_BINARY_DIR}/PowerFlow.m)
