#ifndef GRID_SOLVER_H
#define GRID_SOLVER_H

#include "powerflow/network.hpp"
#include "powerflow/logger/Logger.hpp"

class GridSolver
{
public:
    GridSolver(Grid *grid, Logger *const logger) : grid{grid}, logger{logger} {}
    virtual ~GridSolver() {};

    virtual int solve() = 0; // Returnera antal iterationer
protected:
    Grid *grid{nullptr};
    Logger *const logger{nullptr};
};

#endif