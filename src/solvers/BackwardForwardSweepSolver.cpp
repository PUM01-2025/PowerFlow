#include "powerflow/solvers/BackwardForwardSweepSolver.hpp"
#include "powerflow/network.hpp"

static const double SQRT3 = 1.73205080757;

BackwardForwardSweepSolver::BackwardForwardSweepSolver(Grid *grid, 
    Logger *const logger, int maxIter, double precision) 
    : GridSolver(grid, logger, maxIter, precision)
{
    for (node_idx_t i = 0; i < grid->nodes.size(); ++i)
    {
        if (grid->nodes[i].type == NodeType::SLACK || grid->nodes[i].type == NodeType::SLACK_EXTERNAL)
        {
            rootIdx = i; // Om inte hittar alls???????
            break;
        }
    }
    I.resize(grid->edges.size(), 0.0);
}

int BackwardForwardSweepSolver::solve()
{
    int iter = 0;
    converged = false;

    // SLACK power must be negative in the BFS algorithm.
    grid->nodes[rootIdx].s = -grid->nodes[rootIdx].s;

    while (!converged && iter++ < maxIterations) 
    {
        converged = true; // Until proven otherwise by sweep
        sweep(rootIdx, -1);
    }

    // Change SLACK power to positive value before returning.
    grid->nodes[rootIdx].s = -grid->nodes[rootIdx].s;

    if (!converged)
    {
        throw std::runtime_error("BackwardForwardSweepSolver: The solution did not converge. Maximum number of iterations reached.");
    }
    return iter;
}

complex_t BackwardForwardSweepSolver::sweep(node_idx_t nodeIdx,
                                            edge_idx_t prevEdgeIdx)
{
    GridNode &node = grid->nodes[nodeIdx];

    bool isRoot = prevEdgeIdx == -1;

    if (!isRoot)
    {
        GridEdge &prevEdge = grid->edges[prevEdgeIdx];
        node_idx_t prevNodeIdx = prevEdge.parent == nodeIdx ? prevEdge.child : prevEdge.parent;
        GridNode &prevNode = grid->nodes[prevNodeIdx];

        I[prevEdgeIdx] = std::conj((-node.s) / (SQRT3 * node.v));
        node.v = prevNode.v - SQRT3 * I[prevEdgeIdx] * prevEdge.z_c;
    }

    bool isLeaf = true;
    complex_t s = 0;

    for (node_idx_t edgeIdx : node.edges)
    {
        if (edgeIdx == prevEdgeIdx)
            continue;

        GridEdge &edge = grid->edges[edgeIdx];
        node_idx_t nextIdx = edge.parent == nodeIdx ? edge.child : edge.parent;

        isLeaf = false;
        s += sweep(nextIdx, edgeIdx);
    }

    if (!isLeaf)
    {
        if (std::abs(node.s - s) > precision)
            converged = false;
        node.s = s;
    }

    if (!isRoot)
    {
        GridEdge &prevEdge = grid->edges[prevEdgeIdx];

        return node.s - 3.0 * prevEdge.z_c * I[prevEdgeIdx] *
                            std::conj(I[prevEdgeIdx]);
    }
    else
        return (0.0);
}
