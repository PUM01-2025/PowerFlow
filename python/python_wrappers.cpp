#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>

#include <memory>
#include <complex>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

#include "powerflow/network.hpp"
#include "powerflow/NetworkLoader.hpp"
#include "powerflow/PowerFlowSolver.hpp"

class PowerFlowWrapper
{
public:
    PowerFlowWrapper(const std::string &filePath)
    {
        file = std::make_shared<std::ifstream>(filePath);
        if (!file->is_open())
        {
            throw std::runtime_error("Could not open file: " + filePath);
        }

        loader = std::make_unique<NetworkLoader>(*file);
        network = loader->loadNetwork();
        solver = std::make_unique<PowerFlowSolver>(network);
    }

    std::vector<std::complex<double>> solve(std::vector<std::complex<double>> &P)
    {
        return solver->solve(P);
    }

private:
    std::shared_ptr<std::ifstream> file;
    std::unique_ptr<NetworkLoader> loader;
    std::shared_ptr<Network> network;
    std::unique_ptr<PowerFlowSolver> solver;
};

PYBIND11_MODULE(python_wrappers, m)
{
    m.doc() = "Power Flow binding for Python interface";

    pybind11::class_<PowerFlowWrapper>(m, "PowerFlowWrapper")
        .def(pybind11::init<const std::string &>(), pybind11::arg("filepath"))
        .def("solve", &PowerFlowWrapper::solve, pybind11::arg("P"), "Solve the power flow problem");
}
