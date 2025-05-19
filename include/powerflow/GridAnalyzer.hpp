#ifndef POWERFLOW_GRID_ANALYZER_H
#define POWERFLOW_GRID_ANALYZER_H

#include "powerflow/SolverTypeEnum.hpp"
#include "powerflow/network.hpp"

// Class responsible for determining suitable solver for a Grid.
class GridAnalyzer
{
public:
    // Returns Solvertype Enum depending on grid structure.
    SolverType determineSolver(Grid const& grid);
private:
    // Checks if the grid is suitable for Backward-Forward-Sweep:
    // - The grid must contain no cycles.
    // - The LOAD nodes must be leaf nodes.
    // - The grid must contain exactly one SLACK_IMPLICIT/SLACK node.
    bool isSuitableForBFS(Grid const& grid);

    // Checks if the grid has an edge with impedance set to 0.
    bool hasZeroImpedance(Grid const& grid);

    // Checks if the grid has only one SLACK_IMPLICIT/SLACK node.
    bool hasSingleSlackNode(Grid const& grid);
};

#endif
