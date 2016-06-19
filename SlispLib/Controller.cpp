#include <iostream>
#include <sstream>
#include <fstream>
#include "Controller.h"
#include "Utils.h"

ControllerArgs::ControllerArgs(int argc, const char * const *argv):
  ScriptArgs(),
  Flags(0)
{
  ParseArgs(argc, argv);
}

void ControllerArgs::ParseArgs(int argc, const char * const *argv) {
  if (argc > 0) {
    int argIdx = 0;
    ProgramName.assign(argv[argIdx++]);

    if (argc == 1)
      Flags |= OptionFlags::REPL;
    else {
      std::string currArg = argv[argIdx++];
      if (currArg == "-h"  || currArg == "-help"  || currArg == "-?" ||
          currArg == "--h" || currArg == "--help" || currArg == "--?" ||
          currArg == "/h"  || currArg == "/help"  || currArg == "/?")
      {
        Flags |= OptionFlags::Help;
        return;
      }
      else if (currArg == "-i") {
        Flags |= OptionFlags::REPL;
        if (argc > 2)
          currArg = argv[argIdx++];
      }
      else if (currArg[0] == '-') {
        Flags |= OptionFlags::Error;
        return;
      }

      if (Utils::EndsWith(currArg, ".slisp"))
        Flags |= OptionFlags::RunFile;
      else
        Flags |= OptionFlags::RunCode;
      Run = currArg;

      for (; argIdx < argc; ++argIdx)
        ScriptArgs.push_back(argv[argIdx]);
    }
  }
  else
    Flags |= OptionFlags::REPL;
}

//=============================================================================

const std::string Controller::HelpText = R"(
usage: slisp [option] [code | file] [arg] ...
Options and arguments:
-h   : help
-i   : run REPL after running code or file
file : program read from script file (e.g. script.slisp)
code : program passed in as string
)";

Controller::Controller(int argc, const char * const * argv):
  CmdInterface(),
  Interpreter_(CmdInterface),
  Settings(Interpreter_.GetSettings()),
  Tokenizer_(),
  Parser_(CmdInterface, Tokenizer_, Settings),
  Lib(),
  Args(argc, argv)
{
  SetupEnvironment();
  SetupModules();
}

Controller::Controller():
  Controller(0, nullptr)
{
}

void Controller::Run() {
  if (Args.Flags & ControllerArgs::Error) {
    CmdInterface.WriteError("Failed to parse args");
    DisplayHelp();
  }
  else if (Args.Flags & ControllerArgs::Help)
    DisplayHelp();
  else if (Args.Flags & ControllerArgs::RunCode)
    Run(Args.Run);
  else if (Args.Flags & ControllerArgs::RunFile) {
    if (!RunFile(Args.Run))
      CmdInterface.WriteError("Could not run: " + Args.Run);
  }

  if (Args.Flags & ControllerArgs::REPL) {
    CmdInterface.SetInput();
    REPL();
  }
}

void Controller::Run(std::istream &in) {
  CmdInterface.SetInput(in);
  REPL();
}

void Controller::Run(const std::string &code) {
  std::stringstream in;
  in << code;
  CmdInterface.SetInput(in);
  REPL();
  CmdInterface.SetInput();
}

bool Controller::RunFile(const std::string &inPath) {
  std::fstream in;
  in.open(inPath, std::ios_base::in);
  if (in.is_open()) {
    CmdInterface.SetInput(in);
    REPL();
    CmdInterface.SetInput();
    return true;
  }
  return false;
}

void Controller::SetOutput() {
  OutFile.reset();
  CmdInterface.SetOutput();
}

void Controller::SetOutput(std::ostream &out) {
  OutFile.reset();
  CmdInterface.SetOutput(out);
}

bool Controller::SetOutputFile(const std::string &outPath) {
  OutFile.reset(new std::fstream);
  OutFile->open(outPath, std::ios_base::out);
  if (OutFile->is_open()) {
    CmdInterface.SetOutput(*OutFile);
    return true;
  }
  return false;
}

void Controller::SetupEnvironment() {
  auto &env = Interpreter_.GetEnvironment();
  env.Program = Args.ProgramName;
  if (Args.Flags & ControllerArgs::RunFile)
    env.Script = Args.Run;
  env.Args = Args.ScriptArgs;
}

void Controller::SetupModules() {
  Lib.Load(Interpreter_);
}

void Controller::DisplayHelp() {
  CmdInterface.WriteOutputLine(HelpText);
}

void Controller::REPL() {
  while (!Interpreter_.StopRequested())
    RunSingle();
}

void Controller::RunSingle() {
  std::stringstream ss;
  if (Parser_.Parse()) {
    auto &exprTree = Parser_.ExpressionTree();
    if (exprTree) {
      ExpressionPtr root { exprTree.release() };
      if (!Interpreter_.Evaluate(root)) {
        auto errors = Interpreter_.GetErrors();
        for (auto &error : errors) {
          ss << error.Where << ": " << error.What << std::endl;
        }
      }
    }
    else
      ss << "Parse Error: No Expression Tree" << std::endl;
  }
  else
    ss << "Parse Error: " << Parser_.Error() << std::endl;

  if (!ss.str().empty()) {
    CmdInterface.WriteError(ss.str());
  }
}