#ifndef POWERFLOW_NETWORK_H
#define POWERFLOW_NETWORK_H

#include <vector>
#include <complex>

// Type definitions for the project
using complex_t = std::complex<double>;
using node_idx_t = int;
using grid_idx_t = int;
using edge_idx_t = int;


struct SolverSettings
{
    int max_iterations_gauss{10000};
    int gauss_decimal_precision{10};
    int max_iterations_bfs{10000};
    int bfs_decimal_precision{10};
    int max_iterations_total{10000};
};

// Graph edge struct.
struct GridEdge
{
    node_idx_t parent;
    node_idx_t child;
    complex_t z_c{1};
    complex_t i{}; // Ta bort?
};

// Possible node types.
enum NodeType
{
    SLACK_EXTERNAL,
    SLACK,
    MIDDLE,
    LOAD
};

// Graph node struct.
struct GridNode
{
    NodeType type = NodeType::MIDDLE;
    complex_t v = 1;
    complex_t s = 0;
    std::vector<node_idx_t> edges{};
};

// Edge between two grids.
struct GridConnection
{
    grid_idx_t slackGrid{};
    grid_idx_t pqGrid{};
    node_idx_t slackNode{};
    node_idx_t pqNode{};
};

struct Grid
{
    std::vector<GridEdge> edges{};
    std::vector<GridNode> nodes{};
    double sBase = 1;
    double vBase = 1;
};

// Network of grids with connections between them.
struct Network
{
    std::vector<Grid> grids{};
    std::vector<GridConnection> connections{};
};

#endif
