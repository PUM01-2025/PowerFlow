find_package(Matlab)
matlab_add_mex(NAME PowerFlowMex SRC matlab_mex.cpp LINK_TO PowerFlowLib)
