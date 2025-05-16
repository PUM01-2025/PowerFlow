#include "powerflow/logger/CppLogger.hpp"

CppLogger::CppLogger(std::ostream& os) : os{ os } {}

void CppLogger::flush()
{
    os << ss.str() << std::flush;
    ss.str("");
    ss.clear();
}
