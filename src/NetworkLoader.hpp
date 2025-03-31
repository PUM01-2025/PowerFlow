#ifndef NETWORKLOADER_H
#define NETWORKLOADER_H

#include "network.hpp"

struct NetworkLoaderError: std::runtime_error {
    using std::runtime_error::runtime_error;
};

class NetworkLoader {
public:
    NetworkLoader(std::istream& file);
    Network loadNetwork();

private:
    std::istream& file;
    int currentLine = 0;

    Grid loadGrid();
    std::vector<GridConnection> loadConnections();
    bool getNextLine(std::string& line);
};

#endif /* NETWORKLOADER_H */
