#include "powerflow/solvers/GaussSeidelSolver.hpp"

GaussSeidelSolver::GaussSeidelSolver(Grid* grid, Logger* const logger, 
    int maxIter, double precision) : GridSolver(grid, logger, maxIter, precision),
        y(grid->edges.size()), ySum(grid->nodes.size())
{
    // Create admittance vector.
    for (size_t edgeIdx = 0; edgeIdx < grid->edges.size(); ++edgeIdx)
    {
        complex_t z = grid->edges[edgeIdx].z_c;

        if (z == 0.0)
        {
            throw std::runtime_error("Invalid 0 impedance detected by GaussSeidelSolver");
        }
        y[edgeIdx] = (complex_t)1 / z;
    }

    // Create ySum vector (diagonal in admittance matrix).
    for (size_t nodeIdx = 0; nodeIdx < grid->nodes.size(); ++nodeIdx)
    {
        complex_t sum = 0;

        for (int edgeIdx : grid->nodes[nodeIdx].edges)
        {
            sum += y.at(edgeIdx);
        }
        ySum[nodeIdx] = sum;
    }
}

int GaussSeidelSolver::solve()
{
    bool converged = false;
    int iter = 0;

    // Update load voltages.
    while (!converged && iter++ < maxIterations) 
    {
        converged = true; // Until proven otherwise

        for (node_idx_t nodeIdx{}; nodeIdx < grid->nodes.size(); ++nodeIdx) // For each node
        {
            GridNode& node = grid->nodes[nodeIdx];

            if (node.type == NodeType::SLACK || node.type == NodeType::SLACK_EXTERNAL)
                continue; // Slack node voltage is already known

            complex_t yv = 0;

            for (size_t edgeIdx : node.edges)
            {
                GridEdge& edge = grid->edges[edgeIdx];
                int neighborIdx = edge.parent == nodeIdx ? edge.child : edge.parent;
                GridNode& neighbor = grid->nodes[neighborIdx];

                yv -= neighbor.v * y[edgeIdx];
            }
            yv += node.v * ySum[nodeIdx];
            node.v = node.v - (yv - std::conj(node.s / node.v)) / ySum[nodeIdx];

            if (std::abs(node.v * std::conj(yv) - node.s) > precision)
            {
                converged = false;
            }
        }
    }

    if (!converged)
    {
        throw std::runtime_error("GaussSeidelSolver: The solution did not converge. Maximum number of iterations reached.");
    }

    // Update slack power.
    for (node_idx_t nodeIdx{}; nodeIdx < grid->nodes.size(); ++nodeIdx)
    {
        GridNode& node = grid->nodes[nodeIdx];

        if (node.type != NodeType::SLACK && node.type != NodeType::SLACK_EXTERNAL)
            continue;

        complex_t yv = 0;

        for (edge_idx_t edgeIdx : node.edges)
        {
            GridEdge& edge = grid->edges[edgeIdx];
            int neighborIdx = edge.parent == nodeIdx ? edge.child : edge.parent;
            GridNode& neighbor = grid->nodes[neighborIdx];

            yv -= neighbor.v * y[edgeIdx];
        }
        yv += node.v * ySum[nodeIdx];
        node.s = node.v * std::conj(yv);
    }
    return iter;
}
