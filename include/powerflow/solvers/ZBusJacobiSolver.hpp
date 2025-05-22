#ifndef POWERFLOW_ZBUS_JACOBI_SOLVER_H
#define POWERFLOW_ZBUS_JACOBI_SOLVER_H

#include "powerflow/solvers/GridSolver.hpp"

#include "Eigen/Core"

// GridSolver implementing the ZBus jacobi algorithm.
class ZBusJacobiSolver : public GridSolver
{
public:
    ZBusJacobiSolver(Grid* grid, Logger* const logger, int maxIter, double precision);
    int solve() override;
    void reset() override;

private:
    Eigen::MatrixXcd Z; // Impedance matrix
    Eigen::VectorXcd V;
    Eigen::VectorXcd S;
    Eigen::VectorXcd I;
    node_idx_t slackNodeIdx = -1;
    bool firstRun = true; // Used to indicate that slack power has not been calculated

    // Updates the slack node power.
    void updateSlackPower();
};

#endif
