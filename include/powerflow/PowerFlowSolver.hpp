#ifndef POWER_FLOW_SOLVER_H
#define POWER_FLOW_SOLVER_H

#include "powerflow/network.hpp"
#include "powerflow/GridSolver.hpp"
#include <memory>

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

#endif