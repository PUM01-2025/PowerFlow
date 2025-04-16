#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <sstream>
#include <mutex>

class Logger
{
public:
    template <typename T>
    Logger &operator<<(T const &output);
    Logger &operator<<(std::ostream &(*manip)(std::ostream &));

protected:
    virtual void flush() = 0;
    std::stringstream ss{};
    std::mutex ss_lock{};
};

template <typename T>
Logger &Logger::operator<<(T const &output)
{
    std::lock_guard<std::mutex> lock{ss_lock};
    ss << output;
    return *this;
}

// Special operator overloading for std::endl;
// This could be in a cpp-file but should it?
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

#endif