#include "powerflow/NetworkValidator.hpp"

#include <stack>
#include <set>
#include <unordered_set>

void NetworkValidator::validateNetwork(const Network& network)
{
    validateConnections(network);
    for (grid_idx_t gridIdx = 0; gridIdx < network.grids.size(); ++gridIdx)
    {
        validateGrid(network.grids.at(gridIdx), gridIdx);
    }
}

void NetworkValidator::validateConnections(const Network& network)
{
    std::set<std::tuple<grid_idx_t, node_idx_t>> middleNodes;
    std::set<std::tuple<grid_idx_t, node_idx_t>> slackNodes;

    for (size_t connIdx = 0; connIdx < network.connections.size(); ++connIdx)
    {
        const GridConnection& conn = network.connections.at(connIdx);

        if (conn.slackGrid < 0 || conn.slackGrid >= network.grids.size())
        {
            throw std::invalid_argument("Invalid grid number " + 
                std::to_string(conn.slackGrid) + " in connection " + 
                std::to_string(connIdx));
        }
        if (conn.pqGrid < 0 || conn.pqGrid >= network.grids.size())
        {
            throw std::invalid_argument("Invalid grid number " +
                std::to_string(conn.pqGrid) + " in connection " +
                std::to_string(connIdx));
        }
        if (conn.slackGrid == conn.pqGrid)
        {
            throw std::invalid_argument("Connection in the same grid " + 
                std::to_string(conn.slackGrid) + " not allowed");
        }
        if (conn.slackNode < 0 || conn.slackNode >= network.grids.at(conn.slackGrid).nodes.size())
        {
            throw std::invalid_argument("Invalid node number " +
                std::to_string(conn.slackNode) + " for grid " +
                std::to_string(conn.slackGrid) + " in connection " +
                std::to_string(connIdx));
        }
        if (conn.pqNode < 0 || conn.pqNode >= network.grids.at(conn.pqGrid).nodes.size())
        {
            throw std::invalid_argument("Invalid node number " +
                std::to_string(conn.pqNode) + " for grid " +
                std::to_string(conn.pqGrid) + " in connection " +
                std::to_string(connIdx));
        }
        if (network.grids.at(conn.slackGrid).nodes.at(conn.slackNode).type != LOAD_IMPLICIT)
        {
            throw std::invalid_argument("Invalid node type in connection " + std::to_string(connIdx));
        }
        if (network.grids.at(conn.pqGrid).nodes.at(conn.pqNode).type != SLACK_IMPLICIT)
        {
            throw std::invalid_argument("Invalid node type in connection " + std::to_string(connIdx));
        }

        std::tuple<grid_idx_t, node_idx_t> middleNode = std::make_pair(conn.slackGrid, conn.slackNode);
        std::tuple<grid_idx_t, node_idx_t> slackNode = std::make_pair(conn.pqGrid, conn.pqNode);

        if (middleNodes.count(middleNode) != 0)
        {
            throw std::invalid_argument("Multiple connections to node " + 
                std::to_string(conn.slackNode) + " in grid " + 
                std::to_string(conn.slackGrid) + " detected");
        }
        if (slackNodes.count(slackNode) != 0)
        {
            throw std::invalid_argument("Multiple connections to node " + 
                std::to_string(conn.pqNode) + " in grid " + 
                std::to_string(conn.pqGrid) + " detected");
        }
        middleNodes.insert(middleNode);
        slackNodes.insert(slackNode);
    }

    if (networkIsDisjoint(network))
    {
        throw std::invalid_argument("Not all grids in the network are connected");
    }   
}

bool NetworkValidator::networkIsDisjoint(const Network& network)
{
    // Verify that every SLACK_IMPLICIT/LOAD_IMPLICIT node has one and only one
    // associated connection.
    for (grid_idx_t gridIdx = 0; gridIdx < network.grids.size(); ++gridIdx)
    {
        const Grid& grid = network.grids.at(gridIdx);

        for (node_idx_t nodeIdx = 0; nodeIdx < grid.nodes.size(); ++nodeIdx)
        {
            const GridNode& node = grid.nodes.at(nodeIdx);

            if (node.type == SLACK_IMPLICIT || node.type == LOAD_IMPLICIT)
            {
                int conns = 0; // Number of connections to this node

                for (const GridConnection& conn : network.connections)
                {
                    if ((node.type == SLACK_IMPLICIT && conn.pqGrid == gridIdx &&
                            conn.pqNode == nodeIdx) ||
                        (node.type == LOAD_IMPLICIT && conn.slackGrid == gridIdx &&
                            conn.slackNode == nodeIdx))
                    {
                        ++conns;
                    }
                }
                if (conns != 1)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void NetworkValidator::validateGrid(const Grid& grid, const grid_idx_t gridIdx)
{
    if (grid.sBase == 0 || grid.vBase == 0)
    {
        throw std::invalid_argument("Invalid base in grid " + std::to_string(gridIdx));
    }

    for (edge_idx_t edgeIdx = 0; edgeIdx < grid.edges.size(); ++edgeIdx)
    {
        const GridEdge& edge = grid.edges.at(edgeIdx);

        if (edge.parent < 0 || edge.parent >= grid.nodes.size())
        {
            throw std::invalid_argument("Invalid edge " + std::to_string(edgeIdx) + 
                " parent in grid " + std::to_string(gridIdx));
        }
        if (edge.child < 0 || edge.child >= grid.nodes.size())
        {
            throw std::invalid_argument("Invalid edge " + std::to_string(edgeIdx) +
                " child in grid " + std::to_string(gridIdx));
        }
        if (edge.child == edge.parent)
        {
            throw std::invalid_argument("Invalid edge " + std::to_string(edgeIdx) +
                " that connects to the same node in grid " + std::to_string(gridIdx));
        }
        for (edge_idx_t edgeIdx2 = 0; edgeIdx2 < grid.edges.size(); ++edgeIdx2)
        {
            if (edgeIdx2 == edgeIdx)
                continue;

            const GridEdge& edge2 = grid.edges.at(edgeIdx2);

            if ((edge2.child == edge.child && edge2.parent == edge.parent) ||
                (edge2.child == edge.parent && edge2.parent == edge.child))
            {
                throw std::invalid_argument("More than one edge detected between node " +
                    std::to_string(edge.parent) + " and node " + std::to_string(edge.child) +
                    " in grid " + std::to_string(gridIdx));
            }
        }
    }

    if (gridIsDisjoint(grid))
    {
        throw std::invalid_argument("Grid " + std::to_string(gridIdx) + " consists of multiple disjoint graphs");
    }
}

bool NetworkValidator::gridIsDisjoint(Grid const& grid)
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
        // Insert as visited,
        visited_nodes.insert(current_node);

        // add neighbors to stack
        GridNode current_grid_node = grid.nodes[current_node];

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
                throw std::runtime_error("Error! found edge with myself not in it!");
            }
            visited_edges.insert(id);
        }
    }
    return !(visited_nodes.size() == grid.nodes.size());
}
