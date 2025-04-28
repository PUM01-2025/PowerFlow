#ifndef POWERFLOW_POWER_FLOW_SOLVER_H
#define POWERFLOW_POWER_FLOW_SOLVER_H

#include "powerflow/network.hpp"
#include "powerflow/solvers/GridSolver.hpp"
#include "powerflow/logger/Logger.hpp"
#include <memory>
#include <tuple>
#include <vector>

// Class responsible for solving an entire Network.
class PowerFlowSolver
{
public:
    // network - The network to solve.
	// logger - Logger object.    
    PowerFlowSolver(std::shared_ptr<Network> network, Logger *const logger);

    // RETURNERA PowerFlowSolverResult och ta emot PowerFlowSolverSettings ????
    std::vector<complex_t> solve(std::vector<complex_t> &P, std::vector<complex_t> &V);
    
    // Returns all voltages in the grid
    std::vector<complex_t> getVoltages() const;
    // Returns all currents in the grid
    std::vector<complex_t> getCurrents() const;
    // Returns all powers in the grid
    std::vector<complex_t> getPowers() const;
private:
	std::vector<std::unique_ptr<GridSolver>> gridSolvers;
	std::shared_ptr<Network> network;
	bool firstRun { true };
    Logger* const logger { nullptr };

	// Creates appropriate GridSolvers for each grid in the network.
	void createGridSolvers();

	// Updates LOAD node powers.
	void updateLoads(const std::vector<complex_t>& P);

	// Updates SLACK_EXTERNAL node voltages.
	void updateExternalVoltages(const std::vector<complex_t>& V);

	// Runs the GridSolvers and combines the result.
	int runGridSolvers(int maxIter);

	// Extracts LOAD node voltages.
	std::vector<complex_t> getLoadVoltages();
};

#endif
