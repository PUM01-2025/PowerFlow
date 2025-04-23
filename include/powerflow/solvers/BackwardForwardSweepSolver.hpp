#ifndef BACKWARD_FORWARD_SWEEP_H
#define BACKWARD_FORWARD_SWEEP_H

#include "powerflow/solvers/GridSolver.hpp"
#include "powerflow/network.hpp"

class BackwardForwardSweepSolver : public GridSolver
{
public:
    BackwardForwardSweepSolver(Grid *grid, Logger *logger);
    int solve();

private:
    complex_t sweep(node_idx_t nodeIdx, node_idx_t prevEdgeIdx = -1);
    bool converged{false};
};

#endif