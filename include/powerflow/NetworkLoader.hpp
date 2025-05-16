#ifndef POWERFLOW_NETWORK_LOADER_H
#define POWERFLOW_NETWORK_LOADER_H

#include <memory>
#include <istream>
#include <string>
#include <tuple>
#include "powerflow/network.hpp"
#include "powerflow/NetworkLoaderError.hpp"

// Class responsible for loading network files.
class NetworkLoader
{
public:
    // file - Input stream providing the network file.
    NetworkLoader(std::istream &file);

    // Reads the network file and creates a Network struct.
    // Throws a NetworkLoaderError if the Network could not be loaded.
    std::unique_ptr<Network> loadNetwork();

private:
    std::istream &file;
    int curLine = 0;

    // Loads a single grid from the network file.
    Grid loadGrid();

    // Loads grid base from the network file into grid.
    void getGridBase(Grid &grid);

    // Loads the list of connections from the network file.
    std::vector<GridConnection> loadConnections();

    // Returns the next non-comment and non-empty line in the network file.
    bool getNextLine(std::string &line);
};

#endif
