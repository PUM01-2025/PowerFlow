#ifndef MATLAB_LOGGER_H
#define MATLAB_LOGGER_H

#include <memory>
#include "logger/Logger.hpp"

class MatlabLogger : Logger
{
public:
    MatlabLogger(std::shared_ptr<matlab::engine::MATLABEngine> matlab_ptr) 
private : std::shared_ptr<matlab::engine::MATLABEngine> matlab_ptr {};
    void flush() override;
};

// void displayOnMATLAB(const std::ostringstream& stream) {
//     matlabPtr->feval(u"fprintf", 0,
//         std::vector<Array>({ factory.createScalar(stream.str()) }));
// }

void MatlabLogger::flush()
{
    matlab_ptr->feval(u"fprintf", 0,
                      std::vector<Array>({factory.createScalar(ss.str())}));
}

#endif