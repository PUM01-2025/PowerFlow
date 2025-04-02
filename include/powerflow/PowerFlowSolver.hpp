#pragma once

#include "powerflow/network.hpp"

class PowerFlowSolver {
public:
	PowerFlowSolver(std::shared_ptr<Network> network);

	// V solve(P);
private:
	std::shared_ptr<Network> network;
};
