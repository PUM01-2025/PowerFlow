#include "powerflow/logger/Logger.hpp"

// Special operator overloading for std::endl;
Logger &Logger::operator<<(std::ostream &(*manip)(std::ostream &))
{
    std::lock_guard<std::mutex> lock{ss_lock};
    ss << manip;
    if (manip == static_cast<std::ostream &(*)(std::ostream &)>(std::endl))
    {
        flush();
    }
    return *this;
}
