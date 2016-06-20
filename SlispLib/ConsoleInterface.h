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

    virtual bool HasMore() const override;
    virtual void Reset() override;

    virtual bool ReadLine(const std::string &prefix, std::string &input) override;
    virtual bool WriteOutputLine(const std::string &output) override;
    virtual bool WriteError(const std::string &error) override;
    virtual void SetInteractiveMode(bool enabled) override;
    virtual void GetInteractiveMode(bool &enabled) override;

    void SetInput();
    void SetInput(std::istream &in);
    void SetOutput();
    void SetOutput(std::ostream &out);

  private:
    std::istream *In;
    std::ostream *Out;
    bool HasMore_;
    bool InteractiveMode_;

    void Init();
};
