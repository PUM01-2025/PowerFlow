#include "powerflow/NetworkLoaderError.hpp"

NetworkLoaderError::NetworkLoaderError(const std::string& msg, int line)
{
    message = "Error on line " + std::to_string(line) + ": " + msg;
}

const char* NetworkLoaderError::what() const noexcept
{
    return message.c_str();
}
