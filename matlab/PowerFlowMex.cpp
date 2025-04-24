#include <unordered_map>
#include <fstream>
#include <memory>
#include <cstdint>

#include "powerflow/NetworkLoader.hpp"
#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/network.hpp"
#include "MatlabLogger.hpp"
#include "mexAdapter.hpp"
#include "mex.hpp"

class MexFunction : public matlab::mex::Function
{
    std::unordered_map<std::uint64_t, std::unique_ptr<PowerFlowSolver>> solvers;
    std::uint64_t handleCounter = 0;

    // Pointer to MATLAB engine
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();

    MatlabLogger logger{getEngine(), LogLevel::DEBUG};

public:
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        if (inputs.size() < 1 || inputs[0].getType() != matlab::data::ArrayType::MATLAB_STRING)
        {
            throw std::invalid_argument("Missing first argument: command");
        }

        std::string command = inputs[0][0];

        if (command == "load")
        {
            loadNetwork(outputs, inputs);
        }
        else if (command == "solve")
        {
            solve(outputs, inputs);
        }
        else if (command == "unload")
        {
            unloadNetwork(outputs, inputs);
        }
        else
        {
            throw std::invalid_argument("Invalid command");
        }
    }

private:
    void loadNetwork(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        if (inputs.size() < 2 || inputs[1].getType() != matlab::data::ArrayType::MATLAB_STRING)
        {
            throw std::invalid_argument("Missing file path");
        }

        std::string filePath = inputs[1][0];
        std::ifstream file(filePath);

        if (!file)
        {
            throw std::runtime_error("Could not open Network file");
        }

        NetworkLoader loader(file);
        std::shared_ptr<Network> net = loader.loadNetwork();

        solvers.insert({ handleCounter, std::make_unique<PowerFlowSolver>(net, &logger) });

        std::uint64_t handle = handleCounter;
        ++handleCounter;
        matlab::data::ArrayFactory factory;
        outputs[0] = factory.createScalar<std::uint64_t>(handle);
    }

    void solve(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        int maxIter = 1000;

        if (inputs.size() < 2 || inputs[1].getType() != matlab::data::ArrayType::UINT64 || inputs[1].getNumberOfElements() != 1)
        {
            throw std::invalid_argument("Invalid or missing Network handle");
        }
        if (inputs.size() < 3 || inputs[2].getType() != matlab::data::ArrayType::COMPLEX_DOUBLE)
        {
            throw std::invalid_argument("Missing or invalid S vector");
        }
        if (inputs.size() < 4 || inputs[3].getType() != matlab::data::ArrayType::COMPLEX_DOUBLE && !inputs[3].isEmpty())
        {
            throw std::invalid_argument("Missing or invalid V vector");
        }
        if (inputs.size() == 5 && inputs[4].getType() != matlab::data::ArrayType::DOUBLE)
        {
            throw std::invalid_argument("Max iterations must be a postive integer");
        }
        if (inputs.size() == 5 && inputs[4].getType() == matlab::data::ArrayType::DOUBLE)
        {
            matlab::data::TypedArray<double> maxIterArray = inputs[3]; //TODO
            maxIter = static_cast<int>(maxIterArray[0]);
        }

        std::uint64_t handle = inputs[1][0];
        std::unique_ptr<PowerFlowSolver>& solver = solvers.at(handle);

        matlab::data::ArrayFactory factory;
        matlab::data::TypedArray<complex_t> matlabS = inputs[2];
        matlab::data::TypedArray<complex_t> matlabV = factory.createArray<complex_t>({ 0,0 });

        if (!inputs[3].isEmpty())
        {
            matlabV = inputs[3];
        }

        std::vector<complex_t> S(matlabS.begin(), matlabS.end());
        std::vector<complex_t> V(matlabV.begin(), matlabV.end());

        std::tuple< std::vector<complex_t>, int> Vres_iter = solver->solve(S, V, maxIter);
        std::vector<complex_t> Vres = std::get<0>(Vres_iter);
        int iter = std::get<1>(Vres_iter);

        std::ostringstream oss;

        if (iter == maxIter)
        {
            oss << "\nReached max iterations (" + std::to_string(iter) + " iterations) without converging consider rerunning with \n higher number of iterations";
            printToMatlab(oss);
        }
        else
        {
            oss << "\nConverged after (" + std::to_string(iter) + " iterations)";
            printToMatlab(oss);
        }

        outputs[0] = factory.createArray({ 1, Vres.size() }, Vres.begin(), Vres.end());
    }

    void unloadNetwork(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        if (inputs.size() < 2 || inputs[1].getType() != matlab::data::ArrayType::UINT64 || inputs[1].getNumberOfElements() != 1)
        {
            throw std::invalid_argument("Invalid or missing Network handle");
        }

        std::uint64_t handle = inputs[1][0];

        if (solvers.count(handle) == 0)
        {
            throw std::runtime_error("Network not loaded");
        }
        else
        {
            //solvers.clear();
            solvers.erase(handle);
        }
    }

    void printToMatlab(const std::ostringstream& message)
    {
        matlab::data::ArrayFactory factory;
        matlabPtr->feval(u"fprintf", 0, std::vector<matlab::data::Array>({ factory.createScalar(message.str()) }));
    }
};
