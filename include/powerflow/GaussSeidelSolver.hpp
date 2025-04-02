#pragma once

#include "powerflow/GridSolver.hpp"

class GaussSeidelSolver : public GridSolver {
public:
	GaussSeidelSolver(Grid* grid);
	void solve();
};
