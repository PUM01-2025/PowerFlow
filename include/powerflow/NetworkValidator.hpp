#ifndef POWERFLOW_NETWORK_VALIDATOR_H
#define POWERFLOW_NETWORK_VALIDATOR_H

#include <memory>
#include <istream>
#include <string>
#include <tuple>
#include "powerflow/network.hpp"

// Class responsible for validating Network structs.
// Used by PowerFlowSolver.
class NetworkValidator
{
public:
    // Validates the provided network.
    // Throws if the network is not valid.
    void validateNetwork(const Network& network);
private:
    // Validates Network connections.
    void validateConnections(const Network& network);

    // Validates a single grid.
    void validateGrid(const Grid& grid, const grid_idx_t gridIdx);

    // Returns true if network has mulitple disjoint graphs.
    bool gridIsDisjoint(Grid const& grid);
};

#endif
