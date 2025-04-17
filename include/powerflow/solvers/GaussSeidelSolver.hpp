#ifndef GAUSS_SEIDEL_SOLVER_H
#define GAUSS_SEIDEL_SOLVER_H

#include "powerflow/solvers/GridSolver.hpp"

class GaussSeidelSolver : public GridSolver {
public:
	GaussSeidelSolver(Grid* grid, Logger* const logger);
	int solve();
private:
	std::vector<complex_t> y{}; // Admittances
	std::vector<complex_t> ySum{}; // "Self-admittances" (diagonal elements in admittance matrix)
};

#endif