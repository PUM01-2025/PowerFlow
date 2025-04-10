#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/GaussSeidelSolver.hpp"
#include "powerflow/BackwardForwardSweepSolver.hpp"
#include "powerflow/solverEnums.hpp"
#include "powerflow/NetworkAnalyzer.hpp"

#include <iostream>

PowerFlowSolver::PowerFlowSolver(std::shared_ptr<Network> network) : network { network } { }

std::vector<complex_t> PowerFlowSolver::solve(std::vector<complex_t>& P) {
	if (firstRun) {
		// TODO: Analysera ev felaktigheter i nätverket

		createGridSolvers();
		firstRun = false;
	}
	updateLoads(P);
	runGridSolvers();
	return getLoadVoltages();
}

void PowerFlowSolver::createGridSolvers() {

    int grid_no{};

	for (Grid& grid : network->grids) {
		// TODO: Analysera respektive Grid och välj den Solver som är lämpligast

        switch (determine_solver(grid))
        {
        case GAUSSSEIDEL:
            std::cout << "Found grid number " << grid_no << " suitable for Gauss-Seidel" << std::endl;
            gridSolvers.push_back(std::make_unique<GaussSeidelSolver>(&grid));
            break;
        case BACKWARDFOWARDSWEEP:
        std::cout << "Found grid number " << grid_no << " suitable for BFS" << std::endl;
            gridSolvers.push_back(std::make_unique<BackwardForwardSweepSolver>(&grid));
            break;
        default:
            std::cerr << "No suitable solver found!" << std::endl;
            break;
        }

		// Tillfällig läsning: Skapa en GaussSeidelSolver för varje Grid:
        ++grid_no;
	}
}

void PowerFlowSolver::updateLoads(std::vector<complex_t>& P) {
	// TODO: Sätt node.s = P[i] för alla LOAD-noder.
	size_t pIdx = 0;

	for (Grid& grid : network->grids) {
		for (GridNode& node : grid.nodes) {
			if (node.type == NodeType::LOAD) {
				if (pIdx == P.size()) {
					throw std::runtime_error("P has too few elements");
				}
				node.s = -P.at(pIdx++); // NOTE negative sign!
			}
		}
	}
	if (pIdx != P.size()) {
		throw std::runtime_error("P has incorrect size");
	}
}

void PowerFlowSolver::runGridSolvers() {
	// TODO: Parallellisering

	static const int MAX_ITER = 10000;
	int iter = 0;
	int maxGridIter = 0;

	do {
		maxGridIter = 0;
		for (std::unique_ptr<GridSolver>& solver : gridSolvers) {
			int gridIter = solver->solve();
			maxGridIter = std::max(gridIter, maxGridIter);

			// Update connections (simulates "fake" connection with z=0).
			for (GridConnection& connection : network->connections) {
				network->grids[connection.slackGrid].nodes[connection.slackNode].s =   -  network->grids[connection.pqGrid].nodes[connection.pqNode].s;
				network->grids[connection.pqGrid].nodes[connection.pqNode].v = network->grids[connection.slackGrid].nodes[connection.slackNode].v;
			}
		}
	} while (maxGridIter > 1 && iter++ < MAX_ITER);
}

std::vector<complex_t> PowerFlowSolver::getLoadVoltages() {
	// TODO: H�mta ut alla node.v f�r alla LOAD-noder och returnera.

	std::vector<complex_t> U;

	for (Grid& grid : network->grids) {
		for (GridNode& node : grid.nodes) {
			if (node.type == NodeType::LOAD)
				U.push_back(node.v);
		}
	}
	return U;
}
