//Syfte: Hitta cykler i graf med DFS
#include <vector>
#include <iostream>
#include "powerflow/network.hpp"
#include "solverEnums.cpp"
using namespace std;

Network* net;
vector<Grid> grid = net->grids;
vector<GridEdge> edges = grid->edges;
vector<GridNode> nodes = grid->nodes;

vector<GridEdge> visited;
//DFS för att hitta cykler
//

