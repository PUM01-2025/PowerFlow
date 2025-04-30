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
        std::ifstream file(filePath);

        if (!file)
        {
            throw std::runtime_error("Could not open Network file: " + filePath);
        }

        NetworkLoader loader(file);
        std::unique_ptr<Network> network = loader.loadNetwork();
        PowerFlowSolverSettings settings;
        solver = std::make_unique<PowerFlowSolver>(std::move(network), settings, &cpp_logger);
    }

    void solve(std::vector<std::complex<double>> &S, std::vector<std::complex<double>> &V)
    {
        solver->solve(S, V);
    }

    std::vector<complex_t> getLoadVoltages() const
    {
        return solver->getLoadVoltages();
    }

    std::vector<complex_t> getAllVoltages() const
    {
        return solver->getAllVoltages();
    }

    std::vector<complex_t> getCurrents() const
    {
        return solver->getCurrents();
    }

    std::vector<complex_t> getSlackPowers() const
    {
        return solver->getSlackPowers();
    }

private:
    std::unique_ptr<PowerFlowSolver> solver;
    CppLogger cpp_logger{};
};

PYBIND11_MODULE(PowerFlowPython, m)
{
    pybind11::class_<PowerFlow>(m, "PowerFlow")
        .def(pybind11::init<const std::string&>(), pybind11::arg("filepath"))
        .def("solve", &PowerFlow::solve, pybind11::arg("P"), pybind11::arg("V"), "Solve the power flow problem")
        .def("getLoadVoltages", &PowerFlow::getLoadVoltages, "Get the LOAD node voltages")
        .def("getAllVoltages", &PowerFlow::getAllVoltages, "Get all node voltages")
        .def("getCurrents", &PowerFlow::getCurrents, "Get currents")
        .def("getSlackPowers", &PowerFlow::getSlackPowers, "Get SLACK/SLACK_EXTERNAL powers");
}
