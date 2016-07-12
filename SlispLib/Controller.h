#pragma once

#include <iostream>
#include <fstream>
#include <memory>

#include "Interpreter.h"
#include "ConsoleInterface.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "StdLib\StdLib.h"

class ControllerArgs {
public:
  enum OptionFlags {
    Error   = 1 << 1,
    Help    = 1 << 2,
    RunCode = 1 << 3,
    RunFile = 1 << 4,
    REPL    = 1 << 5,
  };

  std::vector<std::string> ScriptArgs;
  std::string ProgramName;
  std::string Run;
  int Flags;

  explicit ControllerArgs(int argc, const char * const *argv);

private:
  void ParseArgs(int argc, const char * const * argv);
};

class OutputManager {
public:
  enum OutputFlags {
    ShowPrompt = 1 << 1,
    ShowResults = 1 << 2,
  };

  explicit OutputManager(Interpreter &interpreter, Library &lib, ConsoleInterface &cmdInterface);
  void SetFlags(uint8_t flags);
  uint8_t GetFlags() const;

private:
  Interpreter &Interpreter_;
  Library &Lib;
  ConsoleInterface &CmdInterface;
  uint8_t Flags; 
};

class OutputSettingsScope {
public:
  explicit OutputSettingsScope(OutputManager &outManager, uint8_t flags);
  ~OutputSettingsScope();
private:
  OutputManager &OutManager;
  uint8_t OldFlags;
};

class Controller {
  public:
    explicit Controller();
    explicit Controller(int argc, const char * const *argv);
    void Run();
    void Run(std::istream &in);
    void Run(const std::string &code);
    bool RunFile(const std::string &inPath);

    void SetOutput();
    void SetOutput(std::ostream &out);
    bool SetOutputFile(const std::string &outPath);

  private:
    static const std::string HelpText;
    ConsoleInterface CmdInterface;
    Interpreter Interpreter_;
    InterpreterSettings& Settings;
    Tokenizer Tokenizer_;
    Parser Parser_;
    StdLib Lib;
    ControllerArgs Args;
    OutputManager OutManager;
    std::unique_ptr<std::fstream> OutFile;

    void SetupEnvironment();
    void SetupModules();
    void DisplayHelp();
    void StartInteractiveREPL();
    void REPL();
    void RunSingle();

  friend class ControllerTest;
};