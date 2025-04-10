#include "mex.hpp"
#include "mexAdapter.hpp"
#include <unordered_map>
#include "powerflow/network.hpp"
#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/NetworkLoader.hpp"
#include <fstream>
//to make c++ understand matlabstring
#include "MatlabDataArray.hpp"
//for ostringstream
#include <sstream>
#include <memory>



//using matlab::mex::ArgumentList;
using namespace matlab::data;

class MexFunction : public matlab::mex::Function {
    //std::unordered_map<int, std::unique_ptr<PowerFlowSolver>> solvers;
    std::unique_ptr<PowerFlowSolver> solver;

   
    // Pointer to MATLAB engine
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();

    // Factory to create MATLAB data arrays
    ArrayFactory factory;

public:
    MexFunction() {
    }

    ~MexFunction() {
    }

    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        if (inputs.size() < 1 || inputs[0].getType() != matlab::data::ArrayType::MATLAB_STRING) {
            displayError("Missing first argument string (command)");
            return;
        }
        std::string command = inputs[0][0];

        if (command == "load") {
            if (inputs.size() < 2 || inputs[1].getType() != matlab::data::ArrayType::MATLAB_STRING) {
                displayError("Missing file path");
                return;
            }
            std::string filePath = inputs[1][0];
            loadNetwork(filePath);
        }
        else if (command == "solve") {
            //if (inputs.size() < 2 || inputs[1].getType() != matlab::data::Arra)
            std::vector<complex_t> S = inputs[1];
            solve(S);
        }
        else {
            displayError("Invalid command");
        }
    }
private:
    void loadNetwork(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file) {
            displayError("Could not open Network file");
            return;
        }
        NetworkLoader loader(file);
        std::shared_ptr<Network> net;
        try {
            net = loader.loadNetwork();
        }
        catch (...) {
            displayError("FEL FEL FEL"); // TODO
        }
        solver = std::make_unique<PowerFlowSolver>(net);
    }

    void solve(std::vector<complex_t>& S) {
        std::vector<complex_t> U = solver->solve(S);
        displayError("Seems to work...");
    }

    void displayOnMATLAB(const std::ostringstream& stream) {
        matlabPtr->feval(u"fprintf", 0,
            std::vector<Array>({ factory.createScalar(stream.str()) }));
    }

    void displayError(std::string errorMessage) {
      ArrayFactory factory;
      matlabPtr->feval(u"error", 0, std::vector<Array>({ factory.createScalar(errorMessage) }));
    }
    

  
};

