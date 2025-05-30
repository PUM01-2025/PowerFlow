#ifndef POWERFLOW_NETWORK_H
#define POWERFLOW_NETWORK_H

#include <vector>
#include <complex>

// Type definitions for the project
using complex_t = std::complex<double>;
using node_idx_t = int;
using grid_idx_t = int;
using edge_idx_t = int;

static const double SQRT3 = 1.73205080757;

// Graph edge struct.
struct GridEdge
{
    node_idx_t parent = -1;
    node_idx_t child = -1;
    complex_t z_c{1};
};

// Possible node types.
enum NodeType
{
    LOAD,
    LOAD_IMPLICIT,
    MIDDLE,
    SLACK,
    SLACK_IMPLICIT
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
    grid_idx_t loadImplicitGrid{};
    grid_idx_t slackImplicitGrid{};
    node_idx_t loadImplicitNode{};
    node_idx_t slackImplicitNode{};
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
