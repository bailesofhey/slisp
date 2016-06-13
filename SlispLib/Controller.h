#pragma once

#include <iostream>
#include <fstream>
#include <memory>

#include "Interpreter.h"
#include "ConsoleInterface.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "StdLib\StdLib.h"

class Controller {
  public:
    explicit Controller();
    explicit Controller(int argc, char **argv);
    void Run();
    void Run(std::istream &in);
    void Run(const std::string &code);
    bool RunFile(const std::string &inPath);

    void SetOutput();
    void SetOutput(std::ostream &out);
    bool SetOutputFile(const std::string &outPath);

  private:
    ConsoleInterface CmdInterface;
    Interpreter Interpreter_;
    InterpreterSettings& Settings;
    Tokenizer Tokenizer_;
    Parser Parser_;
    StdLib Lib;
    std::unique_ptr<std::fstream> OutFile;

    void SetupEnvironment(int argc, char **argv);
    void REPL();
    void RunSingle();

  friend class ControllerTest;
};