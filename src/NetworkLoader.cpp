#include "powerflow/NetworkLoader.hpp"
#include "powerflow/NetworkLoaderError.hpp"
#include <set>

/** Loads a network file, checks if the network is acceptable if it is an 
unique pointer is returned, if not an error is thrown*/
NetworkLoader::NetworkLoader(std::istream& file) : file{ file } { }

std::unique_ptr<Network> NetworkLoader::loadNetwork()
{
    std::unique_ptr<Network> network = std::make_unique<Network>();
    std::string line;

    while (getNextLine(line))
    {
        if (line == "grid")
        {
            network->grids.push_back(loadGrid());
        }
        else if (line == "connections")
        {
            network->connections = loadConnections(*network);
        }
        else
        {
            throw NetworkLoaderError("Invalid command", curLine);
        }
    }
    return network;
}

Grid NetworkLoader::loadGrid()
{
    Grid grid;
    // Load Grid base.
    getGridBase(grid);

    std::string line;
    std::stringstream sstream{};
    int nodeCount = 0; // Number of nodes in the grid
    std::set<std::pair<int, int>> uniqueEdges;
    

    // Get edges.
    while (getNextLine(line))
    {
        if (line == "%")
            // End of edges list
            break;

        sstream << line;
        GridEdge edge;
        if (!(sstream >> edge.parent) || edge.parent < 0)
        {
            throw NetworkLoaderError("Invalid edge parent index", curLine);
        }
        if (!(sstream >> edge.child) || edge.child < 0 || edge.child == edge.parent)
        {
            throw NetworkLoaderError("Invalid edge child index", curLine);
        }
        if (!(sstream >> edge.z_c) || edge.z_c == (complex_t)0)
        {
            throw NetworkLoaderError("Invalid edge impedance", curLine);
        }
        std::pair newEdge = std::make_pair(edge.parent, edge.child);
        if (uniqueEdges.find(newEdge) != uniqueEdges.end())
        {
            throw NetworkLoaderError("Multiple edges between node " + std::to_string(edge.parent) + " and " + std::to_string(edge.child), curLine);
        }
        else
        {
            uniqueEdges.insert(newEdge);
        }

        // edge.z_c = edge.z_c / ((grid.vBase * grid.vBase) / grid.sBase); // Convert to per-unit

        grid.edges.push_back(edge);
        nodeCount = std::max(nodeCount, std::max(edge.parent + 1, edge.child + 1));
        // Clear the stringstream for the next line
        sstream.str("");
        sstream.clear();
    }

    // Clear the stringstream
    sstream.str("");
    sstream.clear();

    if (nodeCount == 0)
    {
        throw NetworkLoaderError("Empty grid", curLine);
    }

    grid.nodes.resize(nodeCount);

    for (node_idx_t edgeIdx = 0; edgeIdx < grid.edges.size(); ++edgeIdx)
    {
        GridEdge& edge = grid.edges[edgeIdx];

        grid.nodes.at(edge.parent).edges.push_back(edgeIdx);
        grid.nodes.at(edge.child).edges.push_back(edgeIdx);
    }

    // Get load/slack nodes
    while (getNextLine(line))
    {
        if (line == "%") // End of node list
            break;

        sstream << line;
        int nodeIdx = 0;
        std::string type;

        if (!(sstream >> nodeIdx) || nodeIdx < 0 || static_cast<typename std::vector<GridNode>::size_type>(nodeIdx) >= grid.nodes.size())
        {
            throw NetworkLoaderError("Invalid node index", curLine);
        }
        if (!(sstream >> type))
        {
            throw NetworkLoaderError("Missing or invalid node type", curLine);
        }
        if (type == "s")
        {
            grid.nodes.at(nodeIdx).type = NodeType::SLACK;
        }
        else if (type == "l")
        {
            grid.nodes.at(nodeIdx).type = NodeType::LOAD;
        }
        else if (type == "e")
        {
            grid.nodes.at(nodeIdx).type = NodeType::SLACK_EXTERNAL;
        }
        else
        {
            throw NetworkLoaderError("Invalid node type", curLine);
        }
        // Clear the stringstream for the next line
        sstream.str("");
        sstream.clear();
    }
    return grid;
}

void NetworkLoader::getGridBase(Grid& grid)
{
    std::string line;
    std::stringstream sstream{};

    getNextLine(line);
    sstream << line;
    if (!(sstream >> grid.sBase) || grid.sBase == 0)
    {
        throw NetworkLoaderError("Invalid S base", curLine);
    }
    if (!(sstream >> grid.vBase) || grid.vBase == 0)
    {
        throw NetworkLoaderError("Invalid V base", curLine);
    }

    std::string rest;
    std::getline(sstream, rest);
    if (rest.find_first_not_of(" ") != std::string::npos)
    {
        throw NetworkLoaderError("Invalid base line", curLine);
    }
}

std::vector<GridConnection> NetworkLoader::loadConnections(const Network& network)
{
    std::vector<GridConnection> connections;
    std::string line;
    std::stringstream sstream{};

    while (getNextLine(line))
    {
        if (line == "%") // End of connections list
            break;

        GridConnection connection;
        sstream << line;

        if (!(sstream >> connection.slackGrid))
        {
            throw NetworkLoaderError("Invalid slack grid index", curLine);
        }
        if (!(sstream >> connection.slackNode))
        {
            throw NetworkLoaderError("Invalid slack node index", curLine);
        }
        if (!(sstream >> connection.pqGrid))
        {
            throw NetworkLoaderError("Invalid PQ grid index", curLine); // Byt namn till LOAD/MIDDLE??
        }
        if (!(sstream >> connection.pqNode))
        {
            throw NetworkLoaderError("Invalid PQ node index", curLine);
        }

        //Check if the middle grid exists
        if (network.grids.size() < connection.pqGrid)
        {
            throw NetworkLoaderError("The middle grid " + std::to_string(connection.pqGrid) +
                " doesn't exist", curLine);
        }
        //Check if the slack node exists
        if (network.grids.at(connection.pqGrid).nodes.size() < connection.slackNode)
        {
            throw NetworkLoaderError("The slack node " + std::to_string(connection.slackNode) +
                "in the middle grid doesn't exist", curLine);
        }
        //Check if the slack grid exists
        if (network.grids.size() < connection.slackGrid)
        {
            throw NetworkLoaderError("The middle grid " + std::to_string(connection.slackGrid) +
                " doesn't exist", curLine);
        }
        //Check if the middle node exists
        if (network.grids.at(connection.slackGrid).nodes.size() < connection.pqNode)
        {
            throw NetworkLoaderError("The slack node " + std::to_string(connection.pqNode) +
                "in the middle grid doesn't exist", curLine);
        }

        connections.push_back(connection);

        // Clear the stringstream for the next line
        sstream.str("");
        sstream.clear();
    }
    return connections;
}

bool NetworkLoader::getNextLine(std::string& line)
{
    while (std::getline(file, line))
    {
        ++curLine;
        if (!line.empty() && line.at(0) != '#')
        {
            return true;
        }
    }
    return false;
}
