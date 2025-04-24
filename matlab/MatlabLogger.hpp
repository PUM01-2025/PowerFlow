#ifndef POWERFLOW_MATLAB_LOGGER_H
#define POWERFLOW_MATLAB_LOGGER_H

#include <memory>

#include "mex.hpp"
#include "mexAdapter.hpp"
#include "powerflow/logger/Logger.hpp"

// Logger that prints to the Matlab console.
class MatlabLogger : public Logger
{
public:
    MatlabLogger(std::shared_ptr<matlab::engine::MATLABEngine> matlab_ptr, LogLevel log_level);
private: 
    std::shared_ptr<matlab::engine::MATLABEngine> matlab_ptr {};
    void flush() override;
};

MatlabLogger::MatlabLogger(std::shared_ptr<matlab::engine::MATLABEngine> matlab_ptr, LogLevel log_level) : Logger{ log_level }, matlab_ptr{ matlab_ptr } {}

void MatlabLogger::flush()
{   
    matlab::data::ArrayFactory factory;
    matlab_ptr->feval(u"fprintf", 0,
                      std::vector<matlab::data::Array>({factory.createScalar(ss.str())}));
    ss.str("");
    ss.clear();
}

#endif
