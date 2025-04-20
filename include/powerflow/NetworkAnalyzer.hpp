#ifndef NETWORK_ANALYZER_H
#define NETWORK_ANALYZER_H

#include "powerflow/SolverTypeEnum.hpp"
#include "powerflow/network.hpp"

SolverType determine_solver(Grid const & grid);
#endif // !NETWORK_ANALYZER_H