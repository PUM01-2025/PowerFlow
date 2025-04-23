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
/*

Loger base class
This class is used to create a logger that can be used to log messages to different
outputs (console, file, etc.). The logger is thread safe and can be used in a multi-threaded
environment. The logger uses a stringstream to store the messages and a mutex to protect
the stringstream from concurrent access. The logger uses the operator<< to append messages
to the stringstream. The logger also uses the flush() function to write the messages to the 
chosen buffer or way in the derived class.

*/
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