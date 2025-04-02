#include "powerflow/GaussSeidelSolver.hpp"

GaussSeidelSolver::GaussSeidelSolver(Grid* grid) : GridSolver(grid) { }

void GaussSeidelSolver::solve()
{
    for (int i = 0; i < 100; ++i) { // Needs proper convergence check...
        for (int nodeIndex = 0; nodeIndex < grid->nodes.size(); ++nodeIndex) { // For each node
            GridNode& node = grid->nodes[nodeIndex];
            if (node.type != NodeType::SLACK) { // Ignore Generator node, where voltage is known
                std::complex<double> ySum = (0, 0);
                std::complex<double> newV = std::conj(node.s) / std::conj(node.v);
                for (int edgeIndex : node.edges) { // For each edge
                    GridEdge& edge = grid->edges[edgeIndex];
                    int neighborIndex = edge.parent == nodeIndex ? edge.child : edge.parent;
                    GridNode& neighbor = grid->nodes[neighborIndex];
                    std::complex<double> y = (std::complex<double>)1 / edge.z_c;
                    newV += neighbor.v * y;
                    ySum += y;
                }
                newV /= ySum;
                node.v = newV;
            }
            else {
                // Kanske inte behöver göra detta varje iteration??
                std::complex<double> ySum = (0, 0);
                std::complex<double> newS = (0, 0);

                for (int edgeIndex : node.edges) {
                    GridEdge& edge = grid->edges[edgeIndex];
                    int neighborIndex = edge.parent == nodeIndex ? edge.child : edge.parent;
                    GridNode& neighbor = grid->nodes[neighborIndex];
                    std::complex<double> y = (std::complex<double>)1 / edge.z_c;
                    newS += neighbor.v * y;
                    ySum += y;
                }
                node.s = std::conj(std::conj(node.v) * (node.v * ySum - newS));
            }
        }
    }
}
