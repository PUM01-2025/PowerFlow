#include "powerflow/PowerFlowSolver.hpp"

#include "powerflow/GaussSeidelSolver.hpp"

PowerFlowSolver::PowerFlowSolver(std::shared_ptr<Network> network) : network { network } { }

std::vector<complex_t> PowerFlowSolver::solve(std::vector<complex_t>& P) {
	if (firstRun) {
		// TODO: Analysera ev felaktigheter i n�tverket

		createGridSolvers();
		firstRun = false;
	}
	updateLoads(P);
	runGridSolvers();
	return getLoadVoltages();
}

void PowerFlowSolver::createGridSolvers() {
	for (Grid& grid : network->grids) {
		// TODO: Analysera respektive Grid och v�lj den Solver som �r l�mpligast

		// Tillf�llig l�sning: Skapa en GaussSeidelSolver f�r varje Grid:
		gridSolvers.push_back(std::make_unique<GaussSeidelSolver>(&grid));
	}
}

void PowerFlowSolver::updateLoads(std::vector<complex_t>& P) {
	// TODO: S�tt node.s = P[i] f�r alla LOAD-noder.
}

void PowerFlowSolver::runGridSolvers() {
	// TODO: Parallellisering

	static const int MAX_ITER = 10000;
	int iter = 0;
	int maxGridIter = 0;

	do {
		for (std::unique_ptr<GridSolver>& solver : gridSolvers) {
			int gridIter = solver->solve();
			maxGridIter = std::max(gridIter, maxGridIter);

			// Update connections (simulates "fake" connection with z=0).
			for (GridConnection& connection : network->connections) {
				network->grids[connection.slack_grid].nodes[connection.slack_node].s =   -  network->grids[connection.pq_grid].nodes[connection.pq_node].s;
				network->grids[connection.pq_grid].nodes[connection.pq_node].v = network->grids[connection.slack_grid].nodes[connection.slack_node].v;
			}
		}
	} while (maxGridIter > 1 && iter++ < MAX_ITER);
}

std::vector<complex_t> getLoadVoltages() {
	// TODO: H�mta ut alla node.v f�r alla LOAD-noder och returnera.

	std::vector<complex_t> U;

	return U;
}
