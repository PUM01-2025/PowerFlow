#ifndef POWER_FLOW_SOLVER_H
#define POWER_FLOW_SOLVER_H

#include "powerflow/network.hpp"
#include "powerflow/solvers/GridSolver.hpp"
#include "powerflow/logger/Logger.hpp"
#include <memory>

class PowerFlowSolver {
public:
	PowerFlowSolver(std::shared_ptr<Network> network, Logger* const logger);

	std::vector<complex_t> solve(std::vector<complex_t>& P, std::vector<complex_t>& V);
private:
	std::shared_ptr<Network> network;
	std::vector<std::unique_ptr<GridSolver>> gridSolvers;
	bool firstRun = true;
    Logger* const logger{nullptr};
	void createGridSolvers();
	void updateLoads(std::vector<complex_t>& P);
	void updateExternalVoltages(std::vector<complex_t>& V);
	void runGridSolvers();
	std::vector<complex_t> getLoadVoltages();
};

#endif