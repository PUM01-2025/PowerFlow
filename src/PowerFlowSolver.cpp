#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/solvers/GaussSeidelSolver.hpp"
#include "powerflow/solvers/BackwardForwardSweepSolver.hpp"
#include "powerflow/SolverTypeEnum.hpp"
#include "powerflow/NetworkAnalyzer.hpp"

#include <iostream>

PowerFlowSolver::PowerFlowSolver(std::shared_ptr<Network> network, PowerFlowSolverSettings settings, Logger* const logger) : 
    network{ network }, settings{ std::move(settings) }, logger { logger } {
    if (settings.maxCombinedIterations <= 0)
    {
        throw std::invalid_argument("Invalid maxCombinedIterations value");
    }
    if (settings.gaussSeidelMaxIterations <= 0)
    {
        throw std::invalid_argument("Invalid gaussSeidelMaxIterations value");
    }
    if (settings.gaussSeidelPrecision <= 0)
    {
        throw std::invalid_argument("Invalid gaussSeidelPrecision value");
    }
    if (settings.backwardForwardSweepMaxIterations <= 0)
    {
        throw std::invalid_argument("Invalid backwardForwardSweepMaxIterations value");
    }
    if (settings.backwardForwardSweepPrecision <= 0)
    {
        throw std::invalid_argument("Invalid backwardForwardSweepPrecision value");
    }
}

void PowerFlowSolver::solve(const std::vector<complex_t>& S, const std::vector<complex_t>& V)
{
	if (firstRun)
    {
		// TODO: Analysera ev felaktigheter i nätverket
		createGridSolvers();
		firstRun = false;
	}
	updateLoads(S);
	updateExternalVoltages(V);
	runGridSolvers();
	// return make_tuple(getLoadVoltages(), iter);
}

void PowerFlowSolver::createGridSolvers()
{
    int grid_no{};

    for (Grid& grid : network->grids)
    {
        switch (determine_solver(grid))
        {
            case GAUSSSEIDEL:
            {
                *logger << "Found grid number " << grid_no << " suitable for Gauss-Seidel" << std::endl;
                std::unique_ptr<GaussSeidelSolver> gs = std::make_unique<GaussSeidelSolver>(&grid, logger, 
                    settings.gaussSeidelMaxIterations, settings.gaussSeidelPrecision);
                gridSolvers.push_back(std::move(gs));
                break;
            }
            case BACKWARDFOWARDSWEEP:
            {
                *logger << "Found grid number " << grid_no << " suitable for BFS" << std::endl;
                std::unique_ptr<BackwardForwardSweepSolver> bfs = std::make_unique<BackwardForwardSweepSolver>(&grid, logger,
                    settings.backwardForwardSweepMaxIterations, settings.backwardForwardSweepPrecision);
                gridSolvers.push_back(std::move(bfs));
                break;
            }
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

void PowerFlowSolver::runGridSolvers()
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
    while (maxGridIter > 1 && iter++ < (settings.maxCombinedIterations - 1));
}

std::vector<complex_t> PowerFlowSolver::getLoadVoltages() const
{
    std::vector<complex_t> U;

    for (Grid const &grid : network->grids)
    {
        for (GridNode const &node : grid.nodes)
        {
            if (node.type == NodeType::LOAD)
            {
                U.push_back(node.v);
            }
        }
    }
    return U;
}
std::vector<complex_t> PowerFlowSolver::getAllVoltages() const
{
    std::vector<complex_t> result{};

    for (Grid const &g : network->grids)
    {
        for (GridNode const &n : g.nodes)
        {
            result.push_back(n.v);
        }
    }
    return result;
}
std::vector<complex_t> PowerFlowSolver::getCurrents() const
{
    std::vector<complex_t> result{};

    for (Grid const &g : network->grids)
    {
        for (GridEdge const &e : g.edges)
        {
            GridNode p{g.nodes[e.parent]}, c{g.nodes[e.child]};
            complex_t current{e.z_c / (p.v - c.v)}; // FEL: Stämmer nog inte i och med att det är komplexa tal + trefas!
            result.push_back(current);
        }
    }

    return result;
}
std::vector<complex_t> PowerFlowSolver::getSlackPowers() const
{
    std::vector<complex_t> result{};

    for (Grid const &g : network->grids)
    {
        for (GridNode const &n : g.nodes)
        {
            if (n.type == NodeType::SLACK || n.type == NodeType::SLACK_EXTERNAL)
            {
                result.push_back(n.s);
            }
        }
    }

    return result;
}
