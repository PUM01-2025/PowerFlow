#include <unordered_map>
#include <fstream>
#include <memory>

#include "powerflow/NetworkLoader.hpp"
#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/network.hpp"
#include "MatlabLogger.hpp"
#include "mexAdapter.hpp"
#include "mex.hpp"

class MexFunction : public matlab::mex::Function
{
    // std::unordered_map<int, std::unique_ptr<PowerFlowSolver>> solvers;
    std::unique_ptr<PowerFlowSolver> solver;

    // Pointer to MATLAB engine
    MatlabLogger logger{getEngine(), LogLevel::DEBUG};

public:
    MexFunction()
    {
    }

    ~MexFunction()
    {
    }

    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        if (inputs.size() < 1 || inputs[0].getType() != matlab::data::ArrayType::MATLAB_STRING)
            throw std::invalid_argument("Missing first argument: command");

        std::string command = inputs[0][0];

        if (command == "load")
        {
            if (inputs.size() < 2 || inputs[1].getType() != matlab::data::ArrayType::MATLAB_STRING)
                throw std::invalid_argument("Missing file path");

            std::string filePath = inputs[1][0];
            loadNetwork(filePath);
        }
        else if (command == "solve")
        {
            if (inputs.size() < 2 || inputs[1].getType() != matlab::data::ArrayType::COMPLEX_DOUBLE)
                throw std::invalid_argument("Missing S vector");
            if (inputs.size() < 3 || inputs[2].getType() != matlab::data::ArrayType::COMPLEX_DOUBLE)
                throw std::invalid_argument("Missing V vector");

            matlab::data::TypedArray<complex_t> matlabS = inputs[1];
            matlab::data::TypedArray<complex_t> matlabV = inputs[2];
            std::vector<complex_t> S(matlabS.begin(), matlabS.end());
            std::vector<complex_t> V(matlabV.begin(), matlabV.end());

            std::vector<complex_t> Vres = solver->solve(S, V);
            matlab::data::ArrayFactory factory;
            outputs[0] = factory.createArray({1, Vres.size()}, Vres.begin(), Vres.end());
        }
        else
            throw std::invalid_argument("Invalid command");
    }

private:
    void loadNetwork(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file)
            throw std::runtime_error("Could not open Network file");

        NetworkLoader loader(file);
        std::shared_ptr<Network> net;

        net = loader.loadNetwork();
        solver = std::make_unique<PowerFlowSolver>(net, &logger);
    }
};
