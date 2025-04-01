#include "mex.hpp"
#include "mexAdapter.hpp"

using matlab::mex::ArgumentList;
using namespace matlab::data;

class MexFunction : public matlab::mex::Function {

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
        std::vector<int> newVec(10000000);
        vec.push_back(newVec);

        std::ostringstream stream;
        stream << "No of vectors: " << vec.size() << std::endl;
        displayOnMATLAB(stream);
    }

    void displayOnMATLAB(const std::ostringstream& stream) {
        matlabPtr->feval(u"fprintf", 0,
            std::vector<Array>({ factory.createScalar(stream.str()) }));
    }
};