#ifndef POWERFLOW_CPP_LOGGER_H
#define POWERFLOW_CPP_LOGGER_H

#include "powerflow/logger/Logger.hpp"
#include <iostream>
#include <sstream>

class CppLogger : public Logger
{
public:
    explicit CppLogger(std::ostream &os = std::cout);
    ~CppLogger() = default;

private:
    std::ostream &os;
    void flush() override;
};

CppLogger::CppLogger(std::ostream &os) : os{os} {}

// Det här kan vara
void CppLogger::flush()
{
    os << ss.str() << std::flush;
    ss.str("");
    ss.clear();
}

#endif
