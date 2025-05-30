#ifndef POWERFLOW_GRID_SOLVER_H
#define POWERFLOW_GRID_SOLVER_H

#include "powerflow/network.hpp"
#include "powerflow/logger/Logger.hpp"

// Base class for grid solvers (BFS, Gauss-Seidel etc.).
// WARNING: A GridSolver may cache information about the provided grid!
class GridSolver
{
public:
    // grid - The grid to solve. Must outlive this object!
    // logger - Logger object.
    // maxIter - Max number of iterations allowed.
    // precision - Solver precision.
    GridSolver(Grid* grid, Logger* const logger, int maxIter, double precision) 
        : grid{ grid }, logger{ logger }, maxIterations { maxIter }, precision { precision } {}
    virtual ~GridSolver() {};

    // Runs the GridSolver algorithm. Returns number of iterations.
    // The solve method MUST return 0 iterations if the solution has already
    // converged!
    virtual int solve() = 0;

    // Resets the solver.
    virtual void reset();
protected:
    Grid* grid{nullptr};
    Logger* const logger{nullptr};
    int maxIterations = 0;
    double precision = 0;
};

#endif
