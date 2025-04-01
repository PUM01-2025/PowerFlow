#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <complex>

using complex_t = std::complex<double>;

// Graph edge struct
struct GridEdge {
    int parent;
    int child;
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
    NodeType type = NodeType::MIDDLE;
    complex_t v = 1;
    complex_t s = 0;
    std::vector<int> edges;
};

// Edge between two grids
struct GridConnection {
    int slack_grid;
    int pq_grid;
    int slack_node;
    int pq_node;
    double slackToPq; // Lindningsförhållande
};

// Conversion base
// struct Base {
//     double const S; // complex_t?
//     double const V;
//     //double I;
// };

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
