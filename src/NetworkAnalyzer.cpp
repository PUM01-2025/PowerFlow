#include "powerflow/network.hpp"
#include "solverEnums.cpp"


Network* net;
vector<Grid> grid = net->grids;


vector<GridEdge> edges = grid->edges;
vector<GridNode> nodes = grid->nodes;

//Union Find för att hitta cykler:
//

class UnionFind {
    int* parent;
}