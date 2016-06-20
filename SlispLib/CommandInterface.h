#pragma once

#include <string>

class CommandInterface {
  public:
    virtual ~CommandInterface();
    virtual bool HasMore() const = 0;
    virtual void Reset() = 0;
    virtual bool ReadLine(const std::string &prefix, std::string &input) = 0;
    virtual bool ReadInputLine(std::string &input);
    virtual bool ReadContinuedInputLine(std::string &input);
    virtual bool WriteOutputLine(const std::string &output) = 0;
    virtual bool WriteError(const std::string &error) = 0;
    virtual void SetInteractiveMode(bool enabled);
    virtual void GetInteractiveMode(bool &enabled);
};