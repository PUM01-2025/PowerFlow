add_library(PowerFlowLib STATIC powerflow.cpp NetworkLoader.cpp)
target_include_directories(PowerFlowLib PUBLIC ${PROJECT_SOURCE_DIR}/include PRIVATE ${PROJECT_SOURCE_DIR}/lib)
