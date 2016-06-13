#include <iostream>
#include <sstream>
#include <fstream>
#include "Controller.h"

Controller::Controller(int argc, char **argv):
  CmdInterface(),
  Interpreter_(CmdInterface),
  Settings(Interpreter_.GetSettings()),
  Tokenizer_(),
  Parser_(CmdInterface, Tokenizer_, Settings),
  Lib()
{
  SetupEnvironment(argc, argv);
  Lib.Load(Interpreter_);
}

Controller::Controller():
  Controller(0, nullptr)
{
}

void Controller::Run() {
  CmdInterface.SetInput();
  REPL();
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

void Controller::SetupEnvironment(int argc, char **argv) {
  std::vector<std::string> args;
  for (int i = 0; i < argc; ++i)
    args.push_back(argv[i]);
  Interpreter_.GetEnvironment().SetArgs(args, args);
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