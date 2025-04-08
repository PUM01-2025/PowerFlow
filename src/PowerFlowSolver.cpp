#include "powerflow/PowerFlowSolver.hpp"
#include "solverEnums.cpp"
PowerFlowSolver::PowerFlowSolver(std::shared_ptr<Network> network) : network { network } { }

//PowerFlowSolver::