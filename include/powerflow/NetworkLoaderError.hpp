#ifndef POWERFLOW_NETWORK_LOADER_ERROR_H
#define POWERFLOW_NETWORK_LOADER_ERROR_H

#include <exception>
#include <string>

// Exception thrown if NetworkLoader encounters errors.
class NetworkLoaderError : public std::exception
{
public:
    NetworkLoaderError(const std::string& msg, int line);
    const char* what() const noexcept override;

private:
    std::string message;
};

#endif
