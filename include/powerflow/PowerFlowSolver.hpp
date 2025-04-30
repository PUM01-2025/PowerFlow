#ifndef POWERFLOW_POWER_FLOW_SOLVER_H
#define POWERFLOW_POWER_FLOW_SOLVER_H

#include "powerflow/network.hpp"
#include "powerflow/solvers/GridSolver.hpp"
#include "powerflow/logger/Logger.hpp"
#include <memory>
#include <tuple>
#include <vector>
#include <string>

struct PowerFlowSolverSettings
{
	// Max number of iterations for the entire network.
	int maxCombinedIterations = 10000;

	// Gauss-Seidel solver settings.
	int gaussSeidelMaxIterations = 100000;
	double gaussSeidelPrecision = 1e-10;

	// Backward-Forward-Sweep solver settings.
	int backwardForwardSweepMaxIterations = 10000;
	double backwardForwardSweepPrecision = 1e-10;
};

// Class responsible for solving an entire Network.
class PowerFlowSolver
{
public:
    // network - The network to solve.
	// logger - Logger object.    
    PowerFlowSolver(std::shared_ptr<Network> network, PowerFlowSolverSettings settings, Logger *const logger);

    // Solve network.
    void solve(const std::vector<complex_t> &P, const std::vector<complex_t> &V);
    
	// Returns all LOAD voltages in the network.
	std::vector<complex_t> getLoadVoltages() const;

    // Returns all voltages in the network.
    std::vector<complex_t> getAllVoltages() const;

    // Returns all currents in the network.
    std::vector<complex_t> getCurrents() const;

    // Returns all SLACK/SLACK_EXTERNAL powers in the network.
    std::vector<complex_t> getSlackPowers() const;
private:
	std::vector<std::unique_ptr<GridSolver>> gridSolvers;
	std::shared_ptr<Network> network;
	PowerFlowSolverSettings settings;
	bool firstRun { true };
    Logger* const logger { nullptr };

	// Creates appropriate GridSolvers for each grid in the network.
	void createGridSolvers();

	// Updates LOAD node powers.
	void updateLoads(const std::vector<complex_t>& P);

	// Updates SLACK_EXTERNAL node voltages.
	void updateExternalVoltages(const std::vector<complex_t>& V);

	// Runs the GridSolvers and combines the result.
	void runGridSolvers();
};

#endif
