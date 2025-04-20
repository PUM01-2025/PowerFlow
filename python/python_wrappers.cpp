#include <pybind11/pybind11.h>
#include "python_wrappers.hpp"
#include <unordered_map>
// #include "powerflow/network.hpp"
// #include "powerflow/PowerFlowSolver.hpp"
// #include "powerflow/NetworkLoader.hpp"
#include <fstream>
#include <memory>

float pybind11_add(float a, float b)
{
    return a + b;
}

PYBIND11_MODULE(python_wrappers, m)
{
    m.doc() = "Docstring for pybind11 module ============= ";
    m.def("pybind11_add", &pybind11_add, "Addition funtion",
          pybind11::arg("a"), pybind11::arg("b"));
}
