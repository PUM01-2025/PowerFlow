#include "powerflow/solvers/BackwardForwardSweepSolver.hpp"

static const double SQRT3 = 1.73205080757;
static const int MAX_ITER = 10000;
static const double PRECISION = 1e-10;

BackwardForwardSweepSolver::BackwardForwardSweepSolver(Grid* grid, Logger* const logger) :
	GridSolver(grid, logger) {
	/*for (int i = 0; )
		if (node.type == SLACK) {
			rootIdx = 0;
		}
	}*/
}

int BackwardForwardSweepSolver::solve() {
	int iter = 0;
	do {
		converged = true;
		sweep(0, -1); // TA REDA PÅ ROOT-INDEX!!!!!!!! Ej nödvändigtvis 0!!

	} while (!converged && iter++ < MAX_ITER);
	grid->nodes[0].s = -grid->nodes[0].s;
	return iter;
}

complex_t BackwardForwardSweepSolver::sweep(size_t nodeIdx,
	size_t prevEdgeIdx) {
	GridNode& node = grid->nodes[nodeIdx];

	bool isRoot = prevEdgeIdx == -1;

	if (!isRoot) {
		GridEdge& prevEdge = grid->edges[prevEdgeIdx];
		size_t prevNodeIdx = prevEdge.parent == nodeIdx ? prevEdge.child : 
			prevEdge.parent;
		GridNode& prevNode = grid->nodes[prevNodeIdx];

		prevEdge.i = std::conj((-node.s) / (SQRT3 * node.v));
		node.v = prevNode.v - SQRT3 * prevEdge.i * prevEdge.z_c;
	}

	bool isLeaf = true;
	complex_t s = 0;

	for (size_t edgeIdx : node.edges) {
		if (edgeIdx == prevEdgeIdx)
			continue;

		GridEdge& edge = grid->edges[edgeIdx];
		size_t nextIdx = edge.parent == nodeIdx ? edge.child : edge.parent;

		isLeaf = false;
		s += sweep(nextIdx, edgeIdx);
	}

	if (!isLeaf) {
		if (std::abs(node.s - s) > PRECISION)
			converged = false;
		node.s = s;
	}

	if (!isRoot) {
		GridEdge& prevEdge = grid->edges[prevEdgeIdx];

		return node.s - 3.0 * prevEdge.z_c * prevEdge.i *
			std::conj(prevEdge.i);
	}
	else
		return (0.0);
}
