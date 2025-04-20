#ifndef CPP_LOGGER_H
#define CPP_LOGGER_H

#include "logger/Logger.hpp"

class CppLogger : public Logger
{
public:
    explicit CppLogger(std::ostream& os = std::cout); 
    ~CppLogger() = default;

private:
    std::ostream& os;
    void flush() override;
};

// Det här kan vara 
void CppLogger::flush()
{
    os << ss.str() << std::flush;
    ss.str("");
    ss.clear();
}

#endif