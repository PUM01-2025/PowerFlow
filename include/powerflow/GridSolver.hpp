#pragma once

#include "powerflow/network.hpp"

class GridSolver {
public:
	GridSolver(Grid* grid) : grid{ grid } { }
	virtual ~GridSolver() { };

	virtual int solve() = 0; // Returnera antal iterationer
protected:
	Grid* grid;
};
