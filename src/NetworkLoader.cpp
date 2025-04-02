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

    // Get number of nodes in the grid
    try
    {
        getNextLine(line);
        grid.nodes.resize(std::stoi(line));
    }
    catch (...)
    {
        throw NetworkLoaderError("Invalid node count");
    }

    // Get node info
    while (getNextLine(line))
    {
        if (line == "%") // End of node list
            break;
        std::stringstream sstream(line);
        int nodeIdx = 0;
        if (!(sstream >> nodeIdx) || nodeIdx < 0 || nodeIdx >= grid.nodes.size())
            throw NetworkLoaderError("Invalid node index");
        GridNode &node = grid.nodes.at(nodeIdx);
        std::string type;
        sstream >> type;
        if (type == "s")
        {
            node.type = SLACK;
            if (!(sstream >> node.v))
                throw NetworkLoaderError("Invalid slack node voltage");
        }
        else if (type == "l")
        {
            node.type = LOAD;
            if (!(sstream >> node.s))
                throw NetworkLoaderError("Invalid load node power");
            node.s = -node.s; // OBS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        }
        else
            throw NetworkLoaderError("Invalid node type");
    }
    if (line != "%")
        throw NetworkLoaderError("Missing end-of-list indicator");

    // Get edges
    while (getNextLine(line))
    {
        if (line == "%") // End of edges list
            break;
        std::stringstream sstream(line);
        GridEdge edge;
        if (!(sstream >> edge.parent) || edge.parent < 0 || edge.parent >= grid.nodes.size())
            throw NetworkLoaderError("Invalid edge parent index");
        if (!(sstream >> edge.child) || edge.child < 0 || edge.child >= grid.nodes.size() || edge.child == edge.parent)
            throw NetworkLoaderError("Invalid edge child index");
        if (!(sstream >> edge.z_c)) // Check for zero impedance??
            throw NetworkLoaderError("Invalid edge impedance");
        int edgeIdx = grid.edges.size();
        grid.nodes.at(edge.parent).edges.push_back(edgeIdx);
        grid.nodes.at(edge.child).edges.push_back(edgeIdx);
        grid.edges.push_back(edge);
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
        if (!(sstream >> connection.slack_grid))
            throw NetworkLoaderError("Invalid slack grid index");
        if (!(sstream >> connection.slack_node))
            throw NetworkLoaderError("Invalid slack node index");
        if (!(sstream >> connection.pq_grid))
            throw NetworkLoaderError("Invalid PQ grid index");
        if (!(sstream >> connection.pq_node))
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
