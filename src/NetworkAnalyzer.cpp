#include <vector>
#include <iostream>
#include "powerflow/network.hpp"
#include "powerflow/SolverTypeEnum.hpp"
#include <stack>
#include <unordered_set>

// Checks if the grid has only one SLACK/SLACK_EXTERNAL node.
static bool hasSingleSlackNode(Grid const& grid)
{
    int slackNodes = 0;

    for (const GridNode& node : grid.nodes)
    {
        if (node.type == SLACK || node.type == SLACK_EXTERNAL)
        {
            slackNodes++;
        }
    }
    return slackNodes == 1;
}

// Checks if the grid is suitable for Backward-Forward-Sweep:
// - The grid must contain no cycles.
// - The LOAD nodes must be leaf nodes.
// - The grid must contain exactly one SLACK/SLACK_EXTERNAL node.
static bool isSuitableForBFS(Grid const& grid)
{
    std::stack<node_idx_t> todo_list{};
    std::unordered_set<node_idx_t> visited_nodes{};
    std::unordered_set<edge_idx_t> visited_edges{};

    // Push root node
    todo_list.push(0);

    while (!todo_list.empty())
    {
        // Get current node
        node_idx_t current_node = todo_list.top();
        todo_list.pop();

        // Insert as visited, if not successful we have already visited, cycle!
        auto [_, success] = visited_nodes.insert(current_node);

        if (!success)
        {
            return false;
        }

        // add neighbors to stack
        GridNode current_grid_node = grid.nodes[current_node];

        // Verify that LOAD nodes are leaf nodes.
        if (current_grid_node.type == NodeType::LOAD && 
            current_grid_node.edges.size() != 1)
        {
            return false;
        }

        // for all edges of current node, add neighbours to stack
        for (edge_idx_t id : current_grid_node.edges)
        {

            // If edge exists in visited, we shall not go there again
            if (visited_edges.find(id) != visited_edges.end())
            {
                continue;
            }

            node_idx_t parent{ grid.edges[id].parent };
            node_idx_t child{ grid.edges[id].child };
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
                std::cerr << "Error! found edge with myself not in it!" << std::endl; // KASTA FEL HÄR?
            }

            visited_edges.insert(id);
        }
    }

    return hasSingleSlackNode(grid);
}

// Returns Solvertype Enum depending on grid structure.
SolverType determineSolver(Grid const& grid)
{
    if (isSuitableForBFS(grid))
    {
        return BACKWARDFOWARDSWEEP;
    }
    else if (hasSingleSlackNode(grid) && grid.nodes.size() < 10000) // "MAGISK" KONSTANT!!!!!
    {
        return ZBUSJACOBI;
    }
    else
    {
        return GAUSSSEIDEL;
    }
}
