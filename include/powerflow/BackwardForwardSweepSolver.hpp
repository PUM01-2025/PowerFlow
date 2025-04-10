#ifndef BACKWARD_FORWARD_SWEEP_H
#define BACKWARD_FORWARD_SWEEP_H

#include "powerflow/GridSolver.hpp"

class BackwardForwardSweepSolver : public GridSolver {
public:
	BackwardForwardSweepSolver(Grid* grid);
	int solve();
private:
	complex_t sweep(size_t nodeIdx, size_t prevEdgeIdx = -1);
	bool converged {false};
};

#endif