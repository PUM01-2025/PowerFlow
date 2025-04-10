#include "mex.hpp"
#include "mexAdapter.hpp"
#include <unordered_map>
#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/NetworkLoader.hpp"
#include <fstream>
//to make c++ understand matlabstring
#include "MatlabDataArray.hpp"
//for ostringstream
#include <sstream>



using matlab::mex::ArgumentList;
using namespace matlab::data;

class MexFunction : public matlab::mex::Function {
    std::unordered_map<int, std::unique_ptr<PowerFlowSolver>> solvers;

    std::vector<std::vector<int>> vec;

    // Pointer to MATLAB engine
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();

    // Factory to create MATLAB data arrays
    ArrayFactory factory;

public:
    MexFunction() {
    }

    ~MexFunction() {
    }

    void operator()(ArgumentList outputs, ArgumentList inputs) {
        //checkArguments(outputs, inputs);
        // N�r fil l�ses in:
        // 1. Anv�nd NetworkLoader f�r att l�sa in n�tverket.
        // 2. Skapa sedan en PowerFlowSolver med det inl�sta n�tverket och spara undan i solvers.
        // 3. Returnera n�gon slags pekare
        //std::ifstream file{inputs[0]};
        
        //NetworkLoader(file);
        //!inputs[0].isType<MATLABString>() //inputs[0].getType() != matlab::data::MATLABString
        // Check if the input is a valid file name 
        if (inputs.size() < 1 || inputs[0].getType() != matlab::data::ArrayType::MATLAB_STRING) {
            std::ostringstream stream;
            stream << "Not valid file"<<std::endl;
            displayOnMATLAB(stream);
            throw std::invalid_argument("Input must be a valid file name.");
        }

        std::string fileName = inputs[0][0];
        std::ifstream file{fileName};
        
        NetworkLoader loader(file);
        std::shared_ptr<Network> net = loader.loadNetwork();

        // 2. Skapa en PowerFlowSolver med det inl�sta n�tverket och spara undan i solvers.

        // Convert unique_ptr to shared_ptr (network returns unique Powerflow takes shared)
       // std::shared_ptr<Network> sharedNet = std::move(net);

        // create a shared pointer to PowerFlowSolver intitialized with net 

        //FRÅGA MALTE
        //std::shared_ptr<PowerFlowSolver> solver = std::<PowerFlowSolver>(net);
        std::unique_ptr<PowerFlowSolver> pfs = std::make_unique<PowerFlowSolver>(net);
      


        // 3. Returnera n�gon slags pekare

        std::ostringstream stream;
        stream << "No of vectors: " << vec.size() << std::endl;
        displayOnMATLAB(stream);
    }

    void displayOnMATLAB(const std::ostringstream& stream) {
        matlabPtr->feval(u"fprintf", 0,
            std::vector<Array>({ factory.createScalar(stream.str()) }));
    }

  
};

