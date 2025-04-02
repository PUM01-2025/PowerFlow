add_library(PowerFlowLib STATIC  NetworkLoader.cpp "PowerFlowSolver.cpp" "solvers/GaussSeidelSolver.cpp" "solvers/BackwardForwardSweepSolver.cpp" "solvers/JacobiSolver.cpp")
target_include_directories(PowerFlowLib PUBLIC ${PROJECT_SOURCE_DIR}/include PRIVATE ${PROJECT_SOURCE_DIR}/lib)
