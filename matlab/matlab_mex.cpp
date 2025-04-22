#include "mex.hpp"
#include "mexAdapter.hpp"
#include <unordered_map>
#include "powerflow/network.hpp"
#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/NetworkLoader.hpp"
#include <fstream>
#include <memory>

class MexFunction : public matlab::mex::Function
{
    // std::unordered_map<int, std::unique_ptr<PowerFlowSolver>> solvers;
    std::unique_ptr<PowerFlowSolver> solver;

    // Pointer to MATLAB engine
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();

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
                throw std::invalid_argument("Missing P vector");

            matlab::data::TypedArray<complex_t> matlabS = inputs[1];
            std::vector<complex_t> P(matlabS.begin(), matlabS.end());

            std::vector<complex_t> U = solver->solve(P);
            matlab::data::ArrayFactory factory;
            outputs[0] = factory.createArray({1, U.size()}, U.begin(), U.end());
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
        solver = std::make_unique<PowerFlowSolver>(net);
    }

    // void displayOnMATLAB(const std::ostringstream& stream) {
    //     matlabPtr->feval(u"fprintf", 0,
    //         std::vector<Array>({ factory.createScalar(stream.str()) }));
    // }
};
