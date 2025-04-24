#ifndef POWERFLOW_BACKWARD_FORWARD_SWEEP_H
#define POWERFLOW_BACKWARD_FORWARD_SWEEP_H

#include "powerflow/solvers/GridSolver.hpp"
#include "powerflow/network.hpp"

// GridSolver implementing the Backward-Forward-Sweep algorithm.
class BackwardForwardSweepSolver : public GridSolver
{
public:
    BackwardForwardSweepSolver(Grid *grid, Logger *logger);
    int solve();

private:
    // Recursive function that performs a single Backward-Forward sweep.
    // nodeIdx - Index of current node in the grid.
    // prevEdgeIdx - Index of upstream edge (-1 for root node).
    complex_t sweep(node_idx_t nodeIdx, edge_idx_t prevEdgeIdx = -1);
    bool converged{false};
};

#endif
