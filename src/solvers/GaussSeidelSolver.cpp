#include "powerflow/solvers/GaussSeidelSolver.hpp"

static const int MAX_ITER = 10000;
static const double PRECISION = 1e-12;

GaussSeidelSolver::GaussSeidelSolver(Grid* grid, Logger* const logger)
    : GridSolver(grid, logger), y(grid->edges.size()), ySum(grid->nodes.size()) {
    // Create admittance vector.
    for (size_t edgeIdx = 0; edgeIdx < grid->edges.size(); ++edgeIdx) {
        complex_t z = grid->edges[edgeIdx].z_c;

        if (z == (complex_t)0) {
            throw std::runtime_error("Invalid 0 impedance!");
        }
        y[edgeIdx] = (complex_t)1 / z;
    }

    // Create ySum vector (diagonal in admittance matrix).
    for (size_t nodeIdx = 0; nodeIdx < grid->nodes.size(); ++nodeIdx) {
        complex_t sum = 0;

        for (int edgeIdx : grid->nodes[nodeIdx].edges) {
            sum += y.at(edgeIdx);
        }
        ySum[nodeIdx] = sum;
    }
}

int GaussSeidelSolver::solve() {
    bool converged = false;
    int iter = 0;

    *logger << "Test";

    // Update load voltages.
    do {
        converged = true; // Until proven otherwise

        for (size_t nodeIdx = 0; nodeIdx < grid->nodes.size(); ++nodeIdx) { // For each node
            GridNode& node = grid->nodes[nodeIdx];

            if (node.type == NodeType::SLACK || node.type == NodeType::SLACK_EXTERNAL)
                continue; // Slack node voltage is already known

            complex_t i = std::conj(node.s) / std::conj(node.v); // Om node.v == 0 ????

            for (size_t edgeIdx : node.edges) {
                GridEdge& edge = grid->edges[edgeIdx];
                int neighborIdx = static_cast<size_t>(edge.parent) == nodeIdx ? edge.child : edge.parent; // DUMT MED NAMN "CHILD" OCH "PARENT"?
                GridNode& neighbor = grid->nodes[neighborIdx];

                i += neighbor.v * y[edgeIdx];
            }
            complex_t newV = i / ySum[nodeIdx];
            complex_t diff = node.v - newV;

            if (diff.real() > PRECISION || diff.imag() > PRECISION) {
                converged = false;
            }
            node.v = newV; // Update node voltage
        }
    } while (!converged && iter++ < MAX_ITER);

    // Update slack power.
    for (size_t nodeIdx = 0; nodeIdx < grid->nodes.size(); ++nodeIdx) {
        GridNode& node = grid->nodes[nodeIdx];

        if (node.type != NodeType::SLACK && node.type != NodeType::SLACK_EXTERNAL)
            continue;

        complex_t i{0, 0};

        for (size_t edgeIdx : node.edges) {
            GridEdge& edge = grid->edges[edgeIdx];
            int neighborIdx = static_cast<size_t>(edge.parent) == nodeIdx ? edge.child : edge.parent;
            GridNode& neighbor = grid->nodes[neighborIdx];

            i += neighbor.v * y[edgeIdx];
        }
        node.s = std::conj(std::conj(node.v) * (node.v * ySum[nodeIdx] - i));
    }
    return iter;
}
