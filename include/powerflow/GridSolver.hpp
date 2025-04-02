#pragma once

#include "powerflow/network.hpp"

class GridSolver {
public:
	GridSolver(Grid* grid) : grid{ grid } { }

	virtual void solve() = 0; // Returnera antal iterationer? Hur hantera differentiering?
protected:
	Grid* grid;
};
