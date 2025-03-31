#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <complex>

using complex_t = std::complex<double>;

// Graph edge struct
struct GridEdge {
    size_t parent;
    size_t child;
    complex_t z_c;
    complex_t i;
};

// Possible node types
enum NodeType {
    SLACK,
    MIDDLE,
    LOAD
};

// Graph node struct
struct GridNode {
    NodeType type = MIDDLE;
    complex_t v = 1;
    complex_t s = 0;
    std::vector<int> edges;
};

// Edge between two grids
struct GridConnection {
    size_t slack_grid;
    size_t pq_grid;
    size_t slack_node;
    size_t pq_node;
    double slackToPq; // Lindningsförhållande
};

// Conversion base
struct Base {
    double const S; // complex_t?
    double const V;
    //double I;
};

// Grid
struct Grid {
    std::vector<GridEdge> edges;
    std::vector<GridNode> nodes;
    // Base base;
};

// Network of grids with connections between them
struct Network {
    std::vector<Grid> grids;
    std::vector<GridConnection> connections;

    // Här måste sparas undan vilka noder som ska skickas till matlab etc...
};

#endif /* NETWORK_H */
