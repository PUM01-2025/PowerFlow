#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <sstream>
#include <mutex>

enum class LogLevel{
    DEBUG,
    WARNING,
    EXTRA
};

class Logger
{
public:
    template <typename T>
    Logger &operator<<(T && output);
    Logger &operator<<(std::ostream &(*manip)(std::ostream &));
    Logger() = default;
    Logger(LogLevel log_level) : level{ log_level } {};
    
protected:
    LogLevel const level{};
    virtual void flush() = 0;
    std::stringstream ss{};
    std::mutex ss_lock{};

};

// Operator<< is thread safe for each call to operator<<
template <typename T>
Logger &Logger::operator<<(T && output)
{
    std::lock_guard<std::mutex> lock{ss_lock};
    ss << std::forward<T>(output);
    return *this;
}


#endif