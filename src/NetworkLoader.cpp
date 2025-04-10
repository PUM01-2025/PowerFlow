#include "powerflow/NetworkLoader.hpp"

NetworkLoader::NetworkLoader(std::istream &file) : file{file} {}

std::unique_ptr<Network> NetworkLoader::loadNetwork()
{
    try
    {
        std::unique_ptr<Network> network = std::make_unique<Network>();

        std::string line;
        while (getNextLine(line))
        {
            if (line == "grid")
                network->grids.push_back(loadGrid());
            else if (line == "connections")
                network->connections = loadConnections();
            else
                throw NetworkLoaderError("Invalid command");
        }
        return network;
    }
    catch (NetworkLoaderError &e)
    {
        throw NetworkLoaderError("Error on line " + std::to_string(currentLine) + ": " + e.what());
    }
}

Grid NetworkLoader::loadGrid()
{
    Grid grid;
    std::string line;
    int nodeCount = 0; // Number of nodes in the grid

    // Get edges.
    while (getNextLine(line))
    {
        if (line == "%") // End of edges list
            break;
        std::stringstream sstream(line);
        GridEdge edge;
        if (!(sstream >> edge.parent) || edge.parent < 0)
            throw NetworkLoaderError("Invalid edge parent index");
        if (!(sstream >> edge.child) || edge.child < 0 || edge.child == edge.parent)
            throw NetworkLoaderError("Invalid edge child index");
        if (!(sstream >> edge.z_c) || edge.z_c == (complex_t)0)
            throw NetworkLoaderError("Invalid edge impedance");
        
        grid.edges.push_back(edge);
        nodeCount = std::max(nodeCount, std::max(edge.parent + 1, edge.child + 1));
    }
    if (line != "%")
        throw NetworkLoaderError("Missing end-of-list indicator");
    if (nodeCount == 0)
        throw NetworkLoaderError("Empty grid");

    grid.nodes.resize(nodeCount);
    grid.nodes.at(0).type = NodeType::SLACK;

    for (size_t edgeIdx = 0; edgeIdx < grid.edges.size(); ++edgeIdx) {
        GridEdge& edge = grid.edges[edgeIdx];

        grid.nodes.at(edge.parent).edges.push_back(edgeIdx);
        grid.nodes.at(edge.child).edges.push_back(edgeIdx);
    }

    // Get load nodes
    while (getNextLine(line))
    {
        if (line == "%") // End of node list
            break;
        std::stringstream sstream(line);
        int nodeIdx = 0;
        if (!(sstream >> nodeIdx) || nodeIdx < 0 || nodeIdx >= grid.nodes.size())
            throw NetworkLoaderError("Invalid node index");
        grid.nodes.at(nodeIdx).type = NodeType::LOAD;
    }
    if (line != "%")
        throw NetworkLoaderError("Missing end-of-list indicator");
    
    return grid;
}

std::vector<GridConnection> NetworkLoader::loadConnections()
{
    std::vector<GridConnection> connections;
    std::string line;
    while (getNextLine(line))
    {
        if (line == "%") // End of connections list
            break;
        GridConnection connection;
        std::stringstream sstream(line);
        if (!(sstream >> connection.slackGrid))
            throw NetworkLoaderError("Invalid slack grid index");
        if (!(sstream >> connection.slackNode))
            throw NetworkLoaderError("Invalid slack node index");
        if (!(sstream >> connection.pqGrid))
            throw NetworkLoaderError("Invalid PQ grid index");
        if (!(sstream >> connection.pqNode))
            throw NetworkLoaderError("Invalid PQ node index");
        connections.push_back(connection);
    }
    if (line != "%")
        throw NetworkLoaderError("Missing end-of-list indicator");
    return connections;
}

bool NetworkLoader::getNextLine(std::string &line)
{
    while (std::getline(file, line))
    {
        ++currentLine;
        if (!line.empty() && line.at(0) != '#')
            return true;
    }
    return false;
}
