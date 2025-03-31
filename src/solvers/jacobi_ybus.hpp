#ifndef JACOBI_YBUS_H
#define JACOBI_YBUS_H

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include "network.hpp"

using VectorXcd = Eigen::Matrix<complex_t, Eigen::Dynamic, 1>;

VectorXcd jacobi_ybus__solve(bool debug, VectorXcd U, VectorXcd const &S, Eigen::SparseMatrix<complex_t> const &Ybus);

#endif