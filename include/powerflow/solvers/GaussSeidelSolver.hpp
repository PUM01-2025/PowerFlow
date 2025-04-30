#ifndef POWERFLOW_GAUSS_SEIDEL_SOLVER_H
#define POWERFLOW_GAUSS_SEIDEL_SOLVER_H

#include "powerflow/solvers/GridSolver.hpp"

// GridSolver implementing the Gauss-Seidel algorithm.
class GaussSeidelSolver : public GridSolver
{
public:
    GaussSeidelSolver(Grid* grid, Logger* const logger, int maxIter, double precision);
    int solve();

private:
    std::vector<complex_t> y{};    // Admittances
    std::vector<complex_t> ySum{}; // "Self-admittances" (diagonal elements in admittance matrix)
};

#endif
