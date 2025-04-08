#pragma once

#include "powerflow/network.hpp"
#include "powerflow/GridSolver.hpp"

class PowerFlowSolver {
public:
	PowerFlowSolver(std::shared_ptr<Network> network);

	std::vector<complex_t> solve(std::vector<complex_t>& P);
private:
	std::shared_ptr<Network> network;
	std::vector<std::unique_ptr<GridSolver>> gridSolvers;
	bool firstRun = true;

	void createGridSolvers();
	void updateLoads(std::vector<complex_t>& P);
	void runGridSolvers();
	std::vector<complex_t> getLoadVoltages();
};
