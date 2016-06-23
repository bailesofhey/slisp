#include <iostream>
#include <sstream>
#include <fstream>
#include "Controller.h"
#include "Utils.h"
#include "Expression.h"

using namespace std;

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
      string currArg = argv[argIdx++];
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

OutputManager::OutputManager(Interpreter &interpreter, Library &lib, ConsoleInterface &cmdInterface):
  Interpreter_(interpreter),
  Lib(lib),
  CmdInterface(cmdInterface),
  Flags(0)
{
}

void OutputManager::SetFlags(uint8_t flags) {
  uint8_t flagChanges = Flags ^ flags;
  if (flagChanges & OutputManager::ShowResults)
    Lib.SetInteractiveMode(Interpreter_, (flags & OutputManager::ShowResults) != 0);
  if (flagChanges & OutputManager::ShowPrompt)
    CmdInterface.SetInteractiveMode((flags & OutputManager::ShowPrompt) != 0);
  Flags = flags;
}

uint8_t OutputManager::GetFlags() const {
  return Flags;
}

//=============================================================================

OutputSettingsScope::OutputSettingsScope(OutputManager &outManager, uint8_t flags):
  OutManager(outManager),
  OldFlags(outManager.GetFlags())
{
  OutManager.SetFlags(flags);
}

OutputSettingsScope::~OutputSettingsScope() {
  OutManager.SetFlags(OldFlags);
}

//=============================================================================

struct ImportModuleFunctor {
  Controller& Controller_;

  explicit ImportModuleFunctor(Controller &controller):
    Controller_(controller)
  {
  }

  bool operator()(EvaluationContext &ctx) {
    ExpressionPtr firstArg = move(ctx.Args.front());
    ctx.Args.pop_front();
    if (auto *sym = ctx.GetRequiredValue<Symbol>(firstArg)) {
      string fileName = sym->Value + ".slisp";
      bool result = Controller_.RunFile(fileName);
      if (result) {
        ctx.Expr = List::GetNil();
        return true;
      }
      else
        return ctx.Error("Failed to import: " + fileName);
    }
    else
      return false;
  }
};

//=============================================================================

const string Controller::HelpText = R"(
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
  Args(argc, argv),
  OutManager(Interpreter_, Lib, CmdInterface)
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
  else if (Args.Flags & ControllerArgs::RunCode) {
    Run(Args.Run);
  }
  else if (Args.Flags & ControllerArgs::RunFile) {
    bool runResult = RunFile(Args.Run);
    if (!runResult)
      CmdInterface.WriteError("Could not run: " + Args.Run);
  }

  if (Args.Flags & ControllerArgs::REPL) {
    CmdInterface.SetInput();
    REPL();
  }
}

void Controller::Run(istream &in) {
  CmdInterface.SetInput(in);
  REPL();
}

void Controller::Run(const string &code) {
  OutputSettingsScope scope(OutManager, OutputManager::ShowResults);
  stringstream in;
  in << code;
  CmdInterface.SetInput(in);
  REPL();
  CmdInterface.SetInput();
}

bool Controller::RunFile(const string &inPath) {
  OutputSettingsScope scope(OutManager, 0);
  fstream in;
  in.open(inPath, ios_base::in);
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

void Controller::SetOutput(ostream &out) {
  OutFile.reset();
  CmdInterface.SetOutput(out);
}

bool Controller::SetOutputFile(const string &outPath) {
  OutFile.reset(new fstream);
  OutFile->open(outPath, ios_base::out);
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
  OutManager.SetFlags(OutputManager::ShowPrompt | OutputManager::ShowResults);

  auto &symbols = Interpreter_.GetDynamicSymbols();
  symbols.PutSymbolFunction("import", ImportModuleFunctor(*this), FuncDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
}

void Controller::DisplayHelp() {
  CmdInterface.WriteOutputLine(HelpText);
}

void Controller::REPL() {
  while (!Interpreter_.StopRequested())
    RunSingle();
}

void Controller::RunSingle() {
  stringstream ss;
  if (Parser_.Parse()) {
    auto &exprTree = Parser_.ExpressionTree();
    if (exprTree) {
      ExpressionPtr root { exprTree.release() };
      if (!Interpreter_.Evaluate(root)) {
        auto errors = Interpreter_.GetErrors();
        for (auto &error : errors) {
          ss << error.Where << ": " << error.What << endl;
        }
      }
    }
    else
      ss << "Parse Error: No Expression Tree" << endl;
  }
  else
    ss << "Parse Error: " << Parser_.Error() << endl;

  if (!ss.str().empty()) {
    CmdInterface.WriteError(ss.str());
  }
}