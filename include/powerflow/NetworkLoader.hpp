#ifndef NETWORK_LOADER_H
#define NEWTORK_LOADER_H

#include "powerflow/network.hpp"
#include <memory>

struct NetworkLoaderError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class NetworkLoader
{
public:
    NetworkLoader(std::istream &file);
    std::unique_ptr<Network> loadNetwork();

private:
    std::istream &file;
    int currentLine = 0;

    Grid loadGrid();
    std::vector<GridConnection> loadConnections();
    bool getNextLine(std::string &line);
};

#endif