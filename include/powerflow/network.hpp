#pragma once

#include <vector>
#include <complex>

using complex_t = std::complex<double>;

// Graph edge struct.
struct GridEdge {
    int parent;
    int child;
    complex_t z_c = 1;
    complex_t i; // Ta bort?
};

// Possible node types.
enum NodeType {
    SLACK,
    MIDDLE,
    LOAD
};

// Graph node struct.
struct GridNode {
    NodeType type = NodeType::MIDDLE;
    complex_t v = 1; // Ta bort?
    complex_t s = 0; // Ta bort?
    std::vector<int> edges;
};

// Edge between two grids.
struct GridConnection {
    int slackGrid;
    int pqGrid;
    int slackNode;
    int pqNode;
};

struct Grid {
    std::vector<GridEdge> edges;
    std::vector<GridNode> nodes;
};

// Network of grids with connections between them.
struct Network {
    std::vector<Grid> grids;
    std::vector<GridConnection> connections;
};
