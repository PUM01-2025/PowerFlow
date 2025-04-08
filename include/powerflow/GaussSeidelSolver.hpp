#pragma once

#include "powerflow/GridSolver.hpp"

class GaussSeidelSolver : public GridSolver {
public:
	GaussSeidelSolver(Grid* grid);
	int solve();
private:
	std::vector<complex_t> y; // Admittances
	std::vector<complex_t> ySum; // "Self-admittances" (diagonal elements in admittance matrix)
};
