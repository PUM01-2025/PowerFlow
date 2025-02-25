#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <unordered_map>
#include <fstream>
#include <complex>
#include <fstream>
#include <iostream>
#include <tuple>

double const TOLERANCE = 0.000001;

// using Eigen::VectorXcd;
using Eigen::SparseMatrix;

enum class NODE_T
{
    GENERATOR,
    JUNCTION,
    LOAD
};

template <typename info_T>
using node_info = std::unordered_map<unsigned int, info_T>;
using val_t = std::complex<double>;
using VectorXcd = Eigen::Matrix<val_t, Eigen::Dynamic, 1>;

SparseMatrix<val_t> load_grid_from_file(std::string filename, int &size)
{
    std::ifstream gridfile{filename};

    std::vector<std::tuple<int, int, val_t>> edges{};
    std::unordered_map<int, val_t> diagonal_terms{};

    unsigned int x{}, y{};
    val_t z_c{};

    gridfile >> x >> y >> z_c;
    unsigned int max{};
    do
    {
        val_t y_c{static_cast<val_t>(1.0) / z_c};
        // For Y-bus
        edges.push_back(std::make_tuple(y, x, y_c));

        diagonal_terms[x] += y_c;
        diagonal_terms[y] += y_c;

        if (x > max)
            max = x;
        if (y > max)
            max = y;

        gridfile >> x >> y >> z_c;
    } while (gridfile);

    size = max + 1;

    SparseMatrix<val_t> m{size, size};

    for (auto [row, col, y_c] : edges)
    {
        m.insert(row, col) = -y_c;
        m.insert(col, row) = -y_c;
    }
    // For y-bus
    for (auto [i, sum_y] : diagonal_terms)
        m.insert(i, i) = sum_y;

    return m;
}

std::tuple<VectorXcd, VectorXcd, node_info<NODE_T>> load_data_from_file(std::string filename, int const &size)
{
    std::ifstream datafile{filename};

    int node_idx{};
    val_t v{};
    val_t s{};
    std::string type{};

    VectorXcd voltages{size};
    VectorXcd powers{size};
    node_info<NODE_T> types{};

    datafile >> node_idx >> v >> s; //>> type;

    do
    {

        voltages[node_idx] = v;
        powers[node_idx] = -s; // OBS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // This is for when we have types if we need em.
        /*
        if (type == "junction")
            types[node_idx] = NODE_T::JUNCTION;
        else if (type == "load" )
            types[node_idx] = NODE_T::LOAD;
        else
            types[node_idx] = NODE_T::GENERATOR;
        */

        datafile >> node_idx >> v >> s; // >> type;
    } while (datafile);

    return {voltages, powers, types};
}

bool is_tolerable(double const tolerance, VectorXcd const &dP)
{

    for (int i{}; i < dP.size(); ++i)
    {
        if (dP[i].real() > tolerance || dP[i].imag() > tolerance)
            return false;
    }
    return true;
}

void save_data_to_file(std::string const &filename, VectorXcd const &U)
{
    std::ofstream datafile{filename};

    for (val_t node : U)
        datafile << node.real() << "," << node.imag() << std::endl;
}

VectorXcd ybus_jacobi_solver(bool debug, VectorXcd U, VectorXcd const &S, SparseMatrix<val_t> const &Ybus)
{
    auto Ydiag = Ybus.diagonal();

    int max_iter = 100000;

    if (debug)
    {
        std::cout << "Initial U: " << U << std::endl;
        std::cout << "Initial S: " << S << std::endl;
    }

    val_t U_slack{U[0]};
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
            std::cout << "Converged in " << iter + 1 << " iterations." << std::endl;
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

int main()
{
    int size{};
    bool const debug{false};
    SparseMatrix<val_t> Ybus{load_grid_from_file("../grid_edges_linghem.txt", size)};
    auto [U, S, types] = load_data_from_file("../node_data_linghem.txt", size);

    VectorXcd U_solved{ybus_jacobi_solver(debug, U, S, Ybus)};

    save_data_to_file("result.txt", U_solved);
}
