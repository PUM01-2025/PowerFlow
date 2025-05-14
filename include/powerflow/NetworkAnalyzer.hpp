#ifndef NETWORK_ANALYZER_H
#define NETWORK_ANALYZER_H

#include "powerflow/SolverTypeEnum.hpp"
#include "powerflow/network.hpp"

// Determines which solver is suitable for the given grid.
SolverType determineSolver(Grid const & grid);

#endif // !NETWORK_ANALYZER_H
