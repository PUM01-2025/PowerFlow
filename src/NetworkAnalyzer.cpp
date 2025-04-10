// Syfte: Hitta cykler i graf med DFS
#include <vector>
#include <iostream>
#include "powerflow/network.hpp"
#include <solverEnums.hpp>
#include <stack>
#include <unordered_set>
// using namespace std;

// DFS för att hitta cykler
//
// procedure DFS_cycle(G, v) is
//     label v as discovered
//     for all directed edges from v to w that are in G.adjacentEdges(v) do
//         if vertex w is not labeled as discovered then
//             recursively call DFS(G, w)
//         else
//             return "cycle"
//      return "no cycle"
// 1. Start DFS traversal on each unvisited vertex (in case the Graph is not connected).
// 2. During DFS, mark vertices as visited, and run DFS on the adjacent vertices (recursively).
// 3.If an adjacent vertex is already visited and is not the parent of the current vertex, a cycle is detected, and True is returned.
// 4. If DFS traversal is done on all vertices and no cycles are detected, False is returned.

using node_id_t = int;
using edge_id_t = int;

bool has_cycles(Grid const &grid)
{
    // Returns true if the Grid contains a cycle
    std::stack<node_id_t> todo_list{};
    std::unordered_set<node_id_t> visited_nodes{};
    std::unordered_set<edge_id_t> visited_edges{};

    // Push root node
    todo_list.push(0);

    while (!todo_list.empty())
    {
        // Get current node
        node_id_t current_node = todo_list.top();
        todo_list.pop();

        // Insert as visited, if not successful we have already visited, cycle!
        auto [_, success] = visited_nodes.insert(current_node);

        if (!success)
        {
            return true;
        }

        // add neighbors to stack
        GridNode current_grid_node = grid.nodes[current_node];

        // for all edges of current node, add neighbours to stack
        for (edge_id_t id : current_grid_node.edges)
        {

            // If edge exists in visited, we shall not go there again
            if (visited_edges.find(id) != visited_edges.end())
            {
                continue;
            }

            node_id_t parent{grid.edges[id].parent};
            node_id_t child{grid.edges[id].child};
            // If node is child --> push parent to stack
            if (parent != current_node)
            {
                todo_list.push(parent);
            } // If node is parent --> push child to stack
            else if (child != current_node)
            {
                todo_list.push(child);
            }
            else
            {
                std::cerr << "Error! found edge with myself not in it!" << std::endl;
            }

            visited_edges.insert(id);
        }
    }

    return false;
}

SolverType determine_solver(Grid const &grid)
{
    // Returns Solvertype Enum depending on grid structure
    if (has_cycles(grid))
    {
        return GAUSSSEIDEL;
    }

    return BACKWARDFOWARDSWEEP;
}