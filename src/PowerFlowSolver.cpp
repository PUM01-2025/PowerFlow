#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/solvers/GaussSeidelSolver.hpp"
#include "powerflow/solvers/BackwardForwardSweepSolver.hpp"
#include "powerflow/SolverTypeEnum.hpp"
#include "powerflow/NetworkAnalyzer.hpp"

#include <iostream>

PowerFlowSolver::PowerFlowSolver(std::shared_ptr<Network> network, Logger *const logger) : network{network}, logger{logger} { }

std::tuple<std::vector<complex_t>, int> PowerFlowSolver::solve(const std::vector<complex_t>& S, const std::vector<complex_t>& V, int maxIter)
{
	if (firstRun)
    {
		// TODO: Analysera ev felaktigheter i nätverket
		createGridSolvers();
		firstRun = false;
	}
	updateLoads(S);
	updateExternalVoltages(V);
	int iter = runGridSolvers(maxIter);
	return make_tuple(getLoadVoltages(), iter);
}

void PowerFlowSolver::createGridSolvers()
{
    int grid_no{};

    for (Grid& grid : network->grids)
    {
        switch (determine_solver(grid))
        {
        case GAUSSSEIDEL:
            *logger << "Found grid number " << grid_no << " suitable for Gauss-Seidel" << std::endl;
            gridSolvers.push_back(std::make_unique<GaussSeidelSolver>(&grid, logger));
            break;
        case BACKWARDFOWARDSWEEP:
            *logger << "Found grid number " << grid_no << " suitable for BFS" << std::endl;
            gridSolvers.push_back(std::make_unique<BackwardForwardSweepSolver>(&grid, logger));
            break;
        default:
            throw std::runtime_error("No suitable solver found");
        }
        ++grid_no;
    }
}

void PowerFlowSolver::updateLoads(const std::vector<complex_t>& S)
{
    size_t pIdx = 0;

    for (Grid& grid : network->grids)
    {
        for (GridNode& node : grid.nodes)
        {
            if (node.type == NodeType::LOAD)
            {
                if (pIdx == S.size())
                {
                    throw std::runtime_error("S has too few elements");
                }
                node.s = -S.at(pIdx++); // NOTE negative sign!
            }
        }
    }
    if (pIdx != S.size())
    {
        throw std::runtime_error("S is of incorrect size");
    }
}

void PowerFlowSolver::updateExternalVoltages(const std::vector<complex_t>& V)
{
    size_t vIdx = 0;

    for (Grid& grid : network->grids)
    {
        for (GridNode& node : grid.nodes)
        {
            if (node.type == NodeType::SLACK_EXTERNAL)
            {
                if (vIdx == V.size())
                {
                    throw std::runtime_error("V has too few elements");
                }
                node.v = V.at(vIdx++);
            }
        }
    }
    if (vIdx != V.size())
    {
        throw std::runtime_error("V is of incorrect size");
    }
}

int PowerFlowSolver::runGridSolvers(int maxIter)
{
	int iter = 0;
	int maxGridIter = 0;

    do
    {
        maxGridIter = 0;
        for (std::unique_ptr<GridSolver>& solver : gridSolvers)
        {
            int gridIter = solver->solve();
            maxGridIter = std::max(gridIter, maxGridIter);

            // Update connections (simulates "fake" connection with z = 0).
            for (GridConnection& connection : network->connections)
            {
                Grid& slackGrid = network->grids[connection.slackGrid];
                Grid& pqGrid = network->grids[connection.pqGrid];
                GridNode& slackNode = slackGrid.nodes[connection.slackNode];
                GridNode& pqNode = pqGrid.nodes[connection.pqNode];

				slackNode.s = -((pqNode.s * pqGrid.sBase) / slackGrid.sBase);
				pqNode.v = slackNode.v;
			}
		}
	}
    while (maxGridIter > 1 && iter++ < (maxIter - 1));
	return iter;
}

std::vector<complex_t> PowerFlowSolver::getLoadVoltages()
{
    std::vector<complex_t> U;

    for (Grid& grid : network->grids)
    {
        for (GridNode& node : grid.nodes)
        {
            if (node.type == NodeType::LOAD)
            {
                U.push_back(node.v);
            }
        }
    }
    return U;
}
