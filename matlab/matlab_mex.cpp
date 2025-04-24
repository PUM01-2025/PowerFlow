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
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();

    MatlabLogger logger{getEngine(), LogLevel::DEBUG};

public:
    MexFunction()
    {
    }

    ~MexFunction()
    {
    }
    /*Takes input from matlab and executes the powerflow simulation with it*/

    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        matlab::data::ArrayFactory factory;

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
        else if (command == "solve") {
            int maxIter = 1000;
            if (inputs.size() < 2 || inputs[1].getType() != matlab::data::ArrayType::COMPLEX_DOUBLE)
                throw std::invalid_argument("Missing S vector");
            if (inputs.size() < 3 || inputs[2].getType() != matlab::data::ArrayType::COMPLEX_DOUBLE && !inputs[2].isEmpty())
                throw std::invalid_argument("Missing V vector");
            if (inputs.size() == 4 && inputs[3].getType() != matlab::data::ArrayType::DOUBLE)
                throw std::invalid_argument("Max iterations must be a postive integer");
            if (inputs.size() == 4 && inputs[3].getType() == matlab::data::ArrayType::DOUBLE) {
                matlab::data::TypedArray<double> maxIterArray = inputs[3]; //TODO
                maxIter = static_cast<int>(maxIterArray[0]);

            }

         
            matlab::data::TypedArray<complex_t> matlabS = inputs[1];
            matlab::data::TypedArray<complex_t> matlabV = factory.createArray<complex_t>({ 0,0 });
            if (!inputs[2].isEmpty()) { 
                matlabV = inputs[2];
            }
          
            std::vector<complex_t> S(matlabS.begin(), matlabS.end());
            std::vector<complex_t> V(matlabV.begin(), matlabV.end());

            std::tuple< std::vector<complex_t>, int> Vres_iter = solver->solve(S, V, maxIter);
            std::vector<complex_t> Vres = std::get<0>(Vres_iter);
            int iter = std::get<1>(Vres_iter);

            std::ostringstream oss;
            if (iter == maxIter) {
                oss << "\nReached max iterations (" + std::to_string(iter) + " iterations) without converging consider rerunning with \n higher number of iterations";
                printToMatlab(oss);
            }
            else {
                oss << "\nConverged after (" + std::to_string(iter) + " iterations)";
                printToMatlab(oss);
            }

            outputs[0] = factory.createArray({ 1, Vres.size() }, Vres.begin(), Vres.end());
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

    void printToMatlab(const std::ostringstream& message) {
        matlab::data::ArrayFactory factory;
        matlabPtr->feval(u"fprintf", 0, std::vector<matlab::data::Array>({ factory.createScalar(message.str()) }));
    }

    

};
