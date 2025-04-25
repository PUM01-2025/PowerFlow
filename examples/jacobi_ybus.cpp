#include <unordered_map>
#include <fstream>
#include <complex>
#include <fstream>
#include <iostream>
#include <tuple>

#include "solvers/jacobi_ybus.hpp"

double const TOLERANCE = 0.000001;
using Eigen::SparseMatrix;

bool is_tolerable(double const tolerance, VectorXcd const &dP)
{

    for (int i{}; i < dP.size(); ++i)
    {
        if (dP[i].real() > tolerance || dP[i].imag() > tolerance)
            return false;
    }
    return true;
}

VectorXcd jacobi_ybus__solve(bool debug, VectorXcd U, VectorXcd const &S, SparseMatrix<complex_t> const &Ybus)
{
    auto Ydiag = Ybus.diagonal();

    int max_iter = 100000;

    if (debug)
    {
        std::cout << "Initial U: " << U << std::endl;
        std::cout << "Initial S: " << S << std::endl;
    }

    complex_t U_slack{U[0]};
    VectorXcd IR{}, deltaU{}, U_next{};
    double maxRelError{};

    for (int iter{}; iter < max_iter; iter++)
    {

        // Compute residual current: IR = S.cwiseProduct(U.conjugate()) - Ybus * U
        IR = S.cwiseQuotient(U).conjugate() - Ybus * U;
        IR[0] = 0;

        maxRelError = IR.cwiseAbs().maxCoeff();

        if (debug)
        {
            std::cout << "Ybus: " << Ybus << std::endl;
            std::cout << "U: " << U << std::endl;
            std::cout << "S: " << S << std::endl;
        }

        // Check if IR is small enough for tolerance
        if (maxRelError < TOLERANCE)
        {
            //std::cout << "Converged in " << iter + 1 << " iterations." << std::endl;
            break;
        }

        else if (debug)
        {
            std::cout << "IR max: " << maxRelError << std::endl;
        }

        deltaU = IR.cwiseQuotient(Ydiag);
        U += deltaU;
    }
    return U;
}