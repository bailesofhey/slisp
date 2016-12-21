#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <algorithm>
#include <iterator>

#include "Interpreter.h"
#include "Expression.h"
#include "FunctionDef.h"

using namespace std;
using namespace std::placeholders;

//=============================================================================
StackFrame::StackFrame(Interpreter &interp, Function &func):
  StackFrame(interp, move(func))
{
}

StackFrame::StackFrame(Interpreter &interp, Function &&func):
  Interp { interp },
  Func { func },
  LocalStore { },
  Locals { LocalStore, func.GetSourceContext() },
  Dynamics { interp.GetDynamicSymbols(func.GetSourceContext()) },
  DynamicScope { Dynamics, func.GetSourceContext() }
{
  Interp.PushStackFrame(*this);
}

StackFrame::~StackFrame() {
  Interp.PopStackFrame();
}

void StackFrame::PutLocalSymbol(const string &symbolName, ExpressionPtr &&value) {
  Locals.PutSymbol(symbolName, value);
}

void StackFrame::PutLocalSymbol(const string &symbolName, ExpressionPtr &value) {
  PutLocalSymbol(symbolName, move(value));
}

void StackFrame::PutDynamicSymbol(const string &symbolName, ExpressionPtr &&value) {
  DynamicScope.PutSymbol(symbolName, value);
}

void StackFrame::PutDynamicSymbol(const string &symbolName, ExpressionPtr &value) {
  PutDynamicSymbol(symbolName, move(value));
}

bool StackFrame::GetSymbol(const string &symbolName, ExpressionPtr &valueCopy) {
  if (Locals.GetSymbol(symbolName, valueCopy))
    return true;
  else
    return Dynamics.GetSymbol(symbolName, valueCopy);
}

bool StackFrame::GetSymbol(const string &symbolName, Expression *&value) {
  if (Locals.GetSymbol(symbolName, value))
    return true;
  else
    return Dynamics.GetSymbol(symbolName, value);
}

void StackFrame::DeleteSymbol(const string &symbolName) {
  ExpressionPtr value;
  if (Locals.GetSymbol(symbolName, value))
    Locals.DeleteSymbol(symbolName);
  else 
    Dynamics.DeleteSymbol(symbolName);
}

SymbolTable& StackFrame::GetLocalSymbols() {
  return Locals;
}

Function& StackFrame::GetFunction() {
  return Func;
}
//=============================================================================

EvaluationContext::EvaluationContext(Interpreter &interpreter, CompiledFunction &compiledFunction, Symbol &currentFunction, ExpressionPtr &expr, ArgList &args):
  Interp(interpreter),
  CurrentFunction(currentFunction),
  Expr_(expr),
  Args(args),
  SourceContext_(compiledFunction.GetSourceContext()),
  Factory(SourceContext_)
{
}

const SourceContext& EvaluationContext::GetSourceContext() const {
  return SourceContext_;
}

bool EvaluationContext::EvaluateNoError(ExpressionPtr &expr) {
  return Interp.EvaluatePartial(expr);
}

bool EvaluationContext::Evaluate(ExpressionPtr &expr, int argNum) {
  return EvaluateNoError(expr) ? true : EvaluateError(argNum);
}

bool EvaluationContext::Evaluate(ExpressionPtr &expr, const string &argName) {
  return EvaluateNoError(expr) ? true : EvaluateError(argName);
}

bool EvaluationContext::GetSymbol(const string &symName, ExpressionPtr &valueCopy) {
  return Interp.GetCurrentStackFrame().GetSymbol(symName, valueCopy);
}

bool EvaluationContext::GetSymbol(const string &symName, Expression *&value) {
  return Interp.GetCurrentStackFrame().GetSymbol(symName, value);
}

bool EvaluationContext::IsQuoteAList(Quote &quote) {
  auto &quoteValue = quote.Value;
  if (quoteValue) {
    if (auto sexp = TypeHelper::GetValue<Sexp>(quoteValue))
      return true; 
    else
      return false;
  }
  else
    return false;
}

Sexp* EvaluationContext::GetRequiredListValue(ExpressionPtr &expr) {
  Sexp* result = GetList(expr);
  if (!result) {
    if (!Evaluate(expr, "list"))
      return nullptr;
    result = GetList(expr);
  }

  if (!result)
    TypeError("list", expr);
  return result;
}

Sexp* EvaluationContext::GetList(ExpressionPtr &expr) {
  Sexp* result = nullptr;
  if (auto quote = TypeHelper::GetValue<Quote>(expr)) {
    if (auto sexp = TypeHelper::GetValue<Sexp>(quote->Value))
      result = sexp;
  }
  return result;
}

const string EvaluationContext::GetThisFunctionName() {
  if (auto thisSexp = TypeHelper::GetValue<Sexp>(Expr_)) {
    if (auto thisFn = TypeHelper::GetValue<Function>(thisSexp->Args.front())) {
      if (thisFn->Symbol) {
        if (auto thisFnSym = TypeHelper::GetValue<Symbol>(thisFn->Symbol)) {
          return thisFnSym->Value;
        }
      }
    }
  }
  Error("Failed to get current function name");
  return "";
}

bool EvaluationContext::Error(const string &what) {
  return Interp.PushError(EvalError { CurrentFunction.Value, what });
}

bool EvaluationContext::EvaluateError(int argNum) {
  return EvaluateError(to_string(argNum));
}

bool EvaluationContext::EvaluateError(const string &argName) {
  return Error("Failed to evaluate arg: " + argName);
}

bool EvaluationContext::UnknownSymbolError(const string &symName) {
  return Error("Unknown symbol: " + symName);
}

bool EvaluationContext::TypeError(const TypeInfo &expected, const ExpressionPtr &actual) {
  return TypeError(expected.Name(), actual);
}

bool EvaluationContext::TypeError(const string &expectedName, const ExpressionPtr &actual) {
  return Error("Expecting: " + expectedName + ". Got: " + actual->Type().Name());
}

bool EvaluationContext::ArgumentExpectedError() {
  return Error("Argument expected");
}

bool EvaluationContext::AllocationError() {
  return Error("Allocation failed");
}

//=============================================================================

Interpreter::Interpreter(CommandInterface &cmdInterface):
  CmdInterface { cmdInterface },
  Modules { },
  SourceContext_ { CreateModule("Internal", ""), 0 },
  DynamicSymbolStore { },
  DynamicSymbols { DynamicSymbolStore, SourceContext_ },
  Settings { DynamicSymbols },
  StackFrames { },
  MainFunc { SourceContext_ },
  MainFrame { *this, MainFunc },
  TypeReducers { },
  Errors { },
  ErrorWhere { "Interpreter" },
  ErrorStackTrace { },
  StopRequested_ { false },
  ExitCode { 0 },
  Environment_ { }
{
  MainFunc.Symbol.reset(new Symbol(SourceContext_, "__main__"));
  RegisterReducers();
}

Interpreter::~Interpreter() {
  for (auto *mod : Modules)
    delete mod;
}

InterpreterSettings& Interpreter::GetSettings() {
  return Settings;
}

list<EvalError> Interpreter::GetErrors() const {
  return Errors;
}

vector<string> Interpreter::GetErrorStackTrace() const {
  vector<string> errStackTrace;
  copy(begin(ErrorStackTrace), end(ErrorStackTrace), back_inserter(errStackTrace));
  return errStackTrace;
}

bool Interpreter::PushError(const EvalError &error) {
  if (Errors.empty()) {
    Errors.push_back(error);
    ErrorStackTrace.clear();
    for (auto curr = crbegin(StackFrames); curr != crend(StackFrames); ++curr)
      ErrorStackTrace.push_back((*curr)->GetFunction().SymbolName());
  }
  return false;
}

void Interpreter::ClearErrors() {
  Errors.clear();
  ErrorStackTrace.clear();
}

void Interpreter::Stop() {
  StopRequested_ = true;
}

bool Interpreter::StopRequested() const {
  return StopRequested_ || !CmdInterface.HasMore();
}

int Interpreter::GetExitCode() const {
  return ExitCode;
}

void Interpreter::SetExitCode(int exitCode) {
  ExitCode = exitCode;
}

SymbolTable Interpreter::GetDynamicSymbols(const SourceContext &sourceContext) {
  return SymbolTable(DynamicSymbolStore, sourceContext);
}

StackFrame& Interpreter::GetCurrentStackFrame() {
  return *(StackFrames.back());
}

void Interpreter::PushStackFrame(StackFrame &stackFrame) {
  StackFrames.push_back(&stackFrame);
}

void Interpreter::PopStackFrame() {
  StackFrames.pop_back();
}

bool Interpreter::EvaluatePartial(ExpressionPtr &expr) {
  const TypeInfo *type = &(expr->Type());
  auto search = TypeReducers.find(type);
  if (search != TypeReducers.end())
    return search->second(expr);
  else
    throw runtime_error("unknown type: ");
}

bool Interpreter::Evaluate(ExpressionPtr &expr) {
  return Evaluate(move(expr));
}

bool Interpreter::Evaluate(ExpressionPtr &&expr) {
  ClearErrors();
  return EvaluatePartial(expr);
}

CommandInterface& Interpreter::GetCommandInterface() {
  return CmdInterface;
}

Environment& Interpreter::GetEnvironment() {
  return Environment_;
}

ModuleInfo* Interpreter::CreateModule(const string& name, const string &filePath) {
  unique_ptr<ModuleInfo> newModulePtr { new ModuleInfo { name, filePath } };
  if (newModulePtr) {
    ModuleInfo* newMod = newModulePtr.get();
    Modules.push_back(newModulePtr.release());
    return newMod;
  }
  return nullptr;
}

bool Interpreter::GetCurrFrameSymbol(const string &symbolName, ExpressionPtr &value) {
  return GetCurrentStackFrame().GetSymbol(symbolName, value);
}

void Interpreter::RegisterReducers() {
  TypeReducers[&Int::TypeInstance]      = bind(&Interpreter::ReduceInt,       this, _1);
  TypeReducers[&Float::TypeInstance]    = bind(&Interpreter::ReduceFloat,     this, _1);
  TypeReducers[&Str::TypeInstance]      = bind(&Interpreter::ReduceStr,       this, _1);
  TypeReducers[&Bool::TypeInstance]     = bind(&Interpreter::ReduceBool,      this, _1);
  TypeReducers[&Symbol::TypeInstance]   = bind(&Interpreter::ReduceSymbol,    this, _1);
  TypeReducers[&Function::TypeInstance] = bind(&Interpreter::ReduceFunction,  this, _1);
  TypeReducers[&Sexp::TypeInstance]     = bind(&Interpreter::ReduceSexp,      this, _1);
  TypeReducers[&Quote::TypeInstance]    = bind(&Interpreter::ReduceQuote,     this, _1);
  TypeReducers[&Ref::TypeInstance]      = bind(&Interpreter::ReduceRef,       this, _1);
}

bool Interpreter::ReduceBool(ExpressionPtr &expr) {
  return true;
}

bool Interpreter::ReduceInt(ExpressionPtr &expr) {
  return true;
}

bool Interpreter::ReduceFloat(ExpressionPtr &expr) {
  return true;
}

bool Interpreter::ReduceStr(ExpressionPtr &expr) {
  return true;
}

bool Interpreter::ReduceFunction(ExpressionPtr &expr) {
  return true;
}

bool Interpreter::ReduceQuote(ExpressionPtr &expr) {
  return true;
}

bool Interpreter::ReduceRef(ExpressionPtr &expr) {
  return true;
}

bool Interpreter::ReduceSymbol(ExpressionPtr &expr) {
  auto symbol = static_cast<Symbol*>(expr.get());
  ExpressionPtr symCopy = symbol->Clone();
  ExpressionPtr value;
  if (GetCurrFrameSymbol(symbol->Value, value) && value) {
    if (EvaluatePartial(value) && value) {
      if (auto copy = value->Clone()) {
        if (copy) {
          expr = move(copy);
          if (auto fn = TypeHelper::GetValue<Function>(expr))
            fn->Symbol = move(symCopy);
          return true;
        }
        else
          throw runtime_error("Clone() didn't return an Expression");
      }
      else
        return PushError(EvalError { ErrorWhere, "Copying symbol value failed: " + value->ToString() });
    }
    else
      return PushError(EvalError { ErrorWhere, "Evaluation failed: " + value->ToString() });
  }
  else
    return PushError(EvalError { ErrorWhere, "Unknown symbol: " + symbol->Value });
}

bool Interpreter::ReduceSexp(ExpressionPtr &expr) {
  auto sexp = static_cast<Sexp*>(expr.get());
  auto &args = sexp->Args;
  int argNum = 0;
  if (!args.empty()) {
    ExpressionPtr firstArg = move(args.front());
    args.pop_front();

    if (EvaluatePartial(firstArg)) {
      args.push_front(move(firstArg));
      auto &funcExpr = args.front();
      if (auto func = TypeHelper::GetValue<Function>(funcExpr))
        return ReduceSexpFunction(expr, *func);
      else if (TypeHelper::TypeMatches(Literal::TypeInstance, funcExpr.get()->Type())) 
        return ReduceSexpList(expr, args);
      else
        return PushError(EvalError { ErrorWhere, "Expecting function: " + funcExpr->ToString() });
    }
    else
      return PushError(EvalError { ErrorWhere, "Evaluation failed: " + firstArg->ToString() });
  }
  else
    return ReduceSexpList(expr, args);
}

bool Interpreter::ReduceSexpFunction(ExpressionPtr &expr, Function &function) {
  auto &funcDef = function.Def;
  string error;
  auto evaluator = bind(&Interpreter::EvaluatePartial, this, _1);
  ExpressionPtr funcCopy = function.Clone();
  auto funcToCall = static_cast<Function*>(funcCopy.get());
  if (funcDef.ValidateArgs(evaluator, expr, error)) {
    auto e = static_cast<Sexp*>(expr.get());
    ArgList args;
    ArgListHelper::CopyTo(e->Args, args);
    args.pop_front();
    if (auto compiledFunction = dynamic_cast<CompiledFunction*>(funcToCall))
      return ReduceSexpCompiledFunction(expr, *compiledFunction, args);
    else if (auto interpretedFunction = dynamic_cast<InterpretedFunction*>(funcToCall))
      return ReduceSexpInterpretedFunction(expr, *interpretedFunction, args);
    else
      return PushError(EvalError { ErrorWhere, "Unsupported Function Type" });
  }
  else {
    string fnName = function.SymbolName();
    PushError(EvalError { ErrorWhere, (fnName.empty() ? "" : fnName + ": ") + "Invalid arguments for function" });
    return PushError(EvalError { ErrorWhere, error });
  }
}

bool Interpreter::ReduceSexpCompiledFunction(ExpressionPtr &expr, CompiledFunction &function, ArgList &args) {
  if (function.Symbol) {
    if (auto fnSym = TypeHelper::GetValue<Symbol>(function.Symbol)) {
      EvaluationContext ctx(*this, function, *fnSym, expr, args);
      return function.Fn(ctx);
    }
  } 
  return PushError(EvalError { ErrorWhere, "No current function" });
}

bool Interpreter::ReduceSexpInterpretedFunction(ExpressionPtr &expr, InterpretedFunction &function, ArgList &args) {
  StackFrame newFrame { *this, function };
  auto currArg = begin(args);
  auto currFormal = begin(function.Args);
  auto endArg = end(args);
  auto endFormal = end(function.Args);
  while (currArg != endArg && currFormal != endFormal) {
    if (auto sym = TypeHelper::GetValue<Symbol>(*currFormal)) {
      if (EvaluatePartial(*currArg)) {
        newFrame.PutLocalSymbol(sym->Value, move(*currArg));
        ++currArg;
        ++currFormal;
      }
      else
        return PushError(EvalError { ErrorWhere, "Evaluating argument " + sym->Value + " failed" }); 
    }
    else
      return PushError(EvalError { ErrorWhere, "Current formal is not a symbol " + (*currFormal)->ToString() });
  }
  for (auto &kv : function.Closure)
    newFrame.PutLocalSymbol(kv.first, move(kv.second->Clone()));

  if (currArg != endArg)
    return PushError(EvalError { ErrorWhere, "too many args passed to function" });
  else if (currFormal != endFormal)
    return PushError(EvalError { ErrorWhere, "not enough args passed to function" });
  else {
    ExpressionPtr codeCopy = function.Code.Value->Clone();
    if (EvaluatePartial(codeCopy)) {
      expr = move(codeCopy);
      return true;
    }
    else
      return PushError(EvalError { ErrorWhere, "Failed to evaluate function" });
  }
}

// TODO: Use this from StdLib
bool Interpreter::EvaluateArgs(ArgList &args) {
  int argNum = 1;
  for (auto &arg : args) {
    if (!EvaluatePartial(arg)) 
      return PushError(EvalError { ErrorWhere, "Failed to evaluate arg " + to_string(argNum) });
    ++argNum;
  }
  return true;
}

bool Interpreter::BuildListSexp(Sexp &wrappedSexp, ArgList &args) {
  wrappedSexp.Args.push_front(ExpressionPtr { new Symbol(wrappedSexp.GetSourceContext(), Settings.GetListSexp()) });
  ArgListHelper::CopyTo(args, wrappedSexp.Args);
  return true;
}

bool Interpreter::ReduceSexpList(ExpressionPtr &expr, ArgList &args) {
  if (EvaluateArgs(args)) {
    ExpressionPtr wrappedExpr { new Sexp { expr->GetSourceContext() } };
    auto wrappedSexp = static_cast<Sexp*>(wrappedExpr.get());
    if (!BuildListSexp(*wrappedSexp, args))
      return false;

    expr = move(wrappedExpr);
    return ReduceSexp(expr);
  }
  else
    return false;
}
