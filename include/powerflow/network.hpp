#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <complex>

using complex_t = std::complex<double>;

// Graph edge struct.
struct GridEdge {
    int parent;
    int child;
    complex_t z_c{ 1 };
    complex_t i{}; // Ta bort?
};

// Possible node types.
enum NodeType {
    SLACK_EXTERNAL,
    SLACK,
    MIDDLE,
    LOAD
};

// Graph node struct.
struct GridNode {
    NodeType type = NodeType::MIDDLE;
    complex_t v = 1;
    complex_t s = 0;
    std::vector<int> edges{};
};

// Edge between two grids.
struct GridConnection {
    int slackGrid{};
    int pqGrid{};
    int slackNode{};
    int pqNode{};
};

struct Grid {
    std::vector<GridEdge> edges{};
    std::vector<GridNode> nodes{};
    double sBase = 1;
    double vBase = 1;
};

// Network of grids with connections between them.
struct Network {
    std::vector<Grid> grids{};
    std::vector<GridConnection> connections{};
};

#endif