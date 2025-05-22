#include "powerflow/solvers/GridSolver.hpp"

void GridSolver::reset()
{
    for (GridNode& node : grid->nodes)
    {
        node.v = 1.0;
        node.s = 0;
    }
}
