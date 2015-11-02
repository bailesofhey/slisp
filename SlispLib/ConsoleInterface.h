#pragma once

#include <iostream>

#include "CommandInterface.h"

class ConsoleInterface: public CommandInterface {
  public:
    explicit ConsoleInterface();
    explicit ConsoleInterface(std::istream &in, std::ostream &out);
    ConsoleInterface(const ConsoleInterface&)             = delete;
    ConsoleInterface(ConsoleInterface &&)                 = delete;
    ConsoleInterface& operator=(const ConsoleInterface &) = delete;
    ConsoleInterface& operator=(ConsoleInterface &&)      = delete;

    virtual bool ReadInputLine(std::string &input);
    virtual bool ReadContinuedInputLine(std::string &input);
    virtual bool WriteOutputLine(const std::string &output);
    virtual bool WriteError(const std::string &error);

  private:
    std::istream &In;
    std::ostream &Out;

    bool ReadLine(const std::string &prefix, std::string &input);
};