#include "powerflow/solvers/GaussSeidelSolver.hpp"

GaussSeidelSolver::GaussSeidelSolver(Grid* grid, Logger* const logger, 
    int maxIter, double precision) : GridSolver(grid, logger, maxIter, precision),
        y(grid->edges.size())
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
}

int GaussSeidelSolver::solve()
{
    if (hasConverged())
    {
        return 0;
    }

    bool converged = false;
    int iter = 0;

    // Update load voltages.
    while (!converged && iter++ < maxIterations) 
    {
        converged = true; // Until proven otherwise.

        for (node_idx_t nodeIdx{}; nodeIdx < grid->nodes.size(); ++nodeIdx)
        {
            GridNode& node = grid->nodes[nodeIdx];

            if (node.type == NodeType::SLACK_IMPLICIT || node.type == NodeType::SLACK)
                continue; // Slack node voltage is already known

            complex_t yv = 0;
            complex_t ySum = 0;

            for (size_t edgeIdx : node.edges)
            {
                GridEdge& edge = grid->edges[edgeIdx];
                int neighborIdx = edge.parent == nodeIdx ? edge.child : edge.parent;
                GridNode& neighbor = grid->nodes[neighborIdx];

                yv -= neighbor.v * y[edgeIdx];
                ySum += y[edgeIdx];
            }
            yv += node.v * ySum;
            node.v = node.v - (yv - std::conj(node.s / node.v)) / ySum;

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

        if (node.type != NodeType::SLACK_IMPLICIT && node.type != NodeType::SLACK)
            continue;

        complex_t yv = 0;
        complex_t ySum = 0;

        for (edge_idx_t edgeIdx : node.edges)
        {
            GridEdge& edge = grid->edges[edgeIdx];
            int neighborIdx = edge.parent == nodeIdx ? edge.child : edge.parent;
            GridNode& neighbor = grid->nodes[neighborIdx];

            yv -= neighbor.v * y[edgeIdx];
            ySum += y[edgeIdx];
        }
        yv += node.v * ySum;
        node.s = node.v * std::conj(yv);
    }
    return iter;
}

bool GaussSeidelSolver::hasConverged()
{
    for (node_idx_t nodeIdx{}; nodeIdx < grid->nodes.size(); ++nodeIdx)
    {
        GridNode& node = grid->nodes[nodeIdx];

        if (node.type == NodeType::SLACK_IMPLICIT || node.type == NodeType::SLACK)
            continue;

        complex_t yv = 0;
        complex_t ySum = 0;

        for (size_t edgeIdx : node.edges)
        {
            GridEdge& edge = grid->edges[edgeIdx];
            int neighborIdx = edge.parent == nodeIdx ? edge.child : edge.parent;
            GridNode& neighbor = grid->nodes[neighborIdx];

            yv -= neighbor.v * y[edgeIdx];
            ySum += y[edgeIdx];
        }
        yv += node.v * ySum;

        if (std::abs(node.v * std::conj(yv) - node.s) > precision)
        {
            return false;
        }
    }
    return true;
}
