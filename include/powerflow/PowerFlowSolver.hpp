#ifndef POWERFLOW_POWER_FLOW_SOLVER_H
#define POWERFLOW_POWER_FLOW_SOLVER_H

#include "powerflow/network.hpp"
#include "powerflow/solvers/GridSolver.hpp"
#include "powerflow/logger/Logger.hpp"
#include <memory>
#include <tuple>
#include <vector>
#include <string>

// Settings that can be passed to the solver.
struct SolverSettings
{
    int max_iterations_gauss{100000};
    double gauss_seidel_precision{1e-10};
    int max_iterations_bfs{10000};
    double bfs_precision{1e-10};
    int max_iterations_zbusjacobi{10000};
    double zbusjacobi_precision{1e-10};
    int max_iterations_total{10000};
};

// Class responsible for solving an entire Network.
class PowerFlowSolver
{
public:
    // network - The network to solve.
	// logger - Logger object.    
    PowerFlowSolver(std::shared_ptr<Network> network, SolverSettings settings, Logger *const logger);

    // Solve network.
    void solve(const std::vector<complex_t> &P, const std::vector<complex_t> &V);
    
	// Returns all LOAD voltages in the network.
	std::vector<complex_t> getLoadVoltages() const;

    // Returns all voltages in the network.
    std::vector<complex_t> getAllVoltages() const;

    // Returns all currents in the network.
    std::vector<complex_t> getCurrents() const;

    // Returns all SLACK_IMPLICIT/SLACK powers in the network.
    std::vector<complex_t> getSlackPowers() const;

    // Resets powers to 0 and voltages to 1.
    void reset();
private:
	std::vector<std::unique_ptr<GridSolver>> gridSolvers;
	std::shared_ptr<Network> network;
	SolverSettings settings;
	bool firstRun { true };
    Logger* const logger { nullptr };

	// Creates appropriate GridSolvers for each grid in the network.
	void createGridSolvers();

	// Updates LOAD node powers.
	void updateLoads(const std::vector<complex_t>& P);

	// Updates SLACK node voltages.
	void updateExternalVoltages(const std::vector<complex_t>& V);

	// Runs the GridSolvers and combines the result.
	void runGridSolvers();
};

#endif
