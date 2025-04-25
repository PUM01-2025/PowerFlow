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
#include "powerflow/logger/CppLogger.hpp"

class PowerFlow
{
public:
    PowerFlow(const std::string &filePath)
    {
        file = std::make_shared<std::ifstream>(filePath);
        if (!file->is_open())
        {
            throw std::runtime_error("Could not open file: " + filePath);
        }

        loader = std::make_unique<NetworkLoader>(*file);
        network = loader->loadNetwork();
        solver = std::make_unique<PowerFlowSolver>(network, &cpp_logger);
    }

    std::vector<std::complex<double>> solve(std::vector<std::complex<double>> &S, std::vector<std::complex<double>> &V)
    {
        return std::get<0>(solver->solve(S, V));
    }
    std::vector<std::complex<double>> getSlackNodeCurrents() const
    {
        return {}; // Ska implenteras om tid finns (Ej prio)
    }

    std::vector<double> getSlackNodePowers() const
    {
        return {}; // Ska implenteras om tid finns (Ej prio)
    }

private:
    std::shared_ptr<std::ifstream> file;
    std::unique_ptr<NetworkLoader> loader;
    std::shared_ptr<Network> network;
    std::unique_ptr<PowerFlowSolver> solver;
    CppLogger cpp_logger{};
};

PYBIND11_MODULE(python_wrappers, m)
{
    m.doc() = "Power Flow binding for Python interface";

    pybind11::class_<PowerFlow>(m, "PowerFlow")
        .def(pybind11::init<const std::string &>(), pybind11::arg("filepath"))
        .def("solve", &PowerFlow::solve, pybind11::arg("P"), pybind11::arg("V"), "Solve the power flow problem")
        .def("get_slack_node_currents", &PowerFlow::getSlackNodeCurrents, "Get the slack node currents")
        .def("get_slack_node_powers", &PowerFlow::getSlackNodePowers, "Get the slack node powers");
}
