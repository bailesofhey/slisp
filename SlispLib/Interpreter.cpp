#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <algorithm>

#include "Interpreter.h"
#include "Expression.h"
#include "FunctionDef.h"

using std::cout;
using std::endl;
using namespace std::placeholders;

//=============================================================================

StackFrame::StackFrame(Interpreter &interp, Function &func):
  Interp { interp },
  Func { func },
  Locals {},
  Dynamics { interp.GetDynamicSymbols() },
  DynamicScope { Dynamics }
{
  Interp.PushStackFrame(*this);
}

StackFrame::~StackFrame() {
  Interp.PopStackFrame();
}

void StackFrame::PutLocalSymbol(const std::string &symbolName, ExpressionPtr &value) {
  Locals.PutSymbol(symbolName, value);
}

void StackFrame::PutDynamicSymbol(const std::string &symbolName, ExpressionPtr &value) {
  DynamicScope.PutSymbol(symbolName, value);
}

bool StackFrame::GetSymbol(const std::string &symbolName, ExpressionPtr &value) {
  if (Locals.GetSymbol(symbolName, value))
    return true;
  else
    return Dynamics.GetSymbol(symbolName, value);
}

void StackFrame::DeleteSymbol(const std::string &symbolName) {
  ExpressionPtr value;
  if (Locals.GetSymbol(symbolName, value))
    Locals.DeleteSymbol(symbolName);
  else 
    Dynamics.DeleteSymbol(symbolName);
}

SymbolTable& StackFrame::GetLocalSymbols() {
  return Locals;
}

//=============================================================================

EvaluationContext::EvaluationContext(Interpreter &interpreter, Symbol &currentFunction, ExpressionPtr &expr, ArgList &args):
  Interp(interpreter),
  CurrentFunction(currentFunction),
  Expr(expr),
  Args(args)
{
}

bool EvaluationContext::EvaluateNoError(ExpressionPtr &expr) {
  return Interp.EvaluatePartial(expr);
}

bool EvaluationContext::Evaluate(ExpressionPtr &expr, int argNum) {
  return EvaluateNoError(expr) ? true : EvaluateError(argNum);
}

bool EvaluationContext::Evaluate(ExpressionPtr &expr, const std::string &argName) {
  return EvaluateNoError(expr) ? true : EvaluateError(argName);
}

const std::string EvaluationContext::GetThisFunctionName() {
  if (auto *thisSexp = dynamic_cast<Sexp*>(Expr.get())) {
    if (auto *thisFn = dynamic_cast<Function*>(thisSexp->Args.front().get())) {
      if (thisFn->Symbol) {
        if (auto *thisFnSym = dynamic_cast<Symbol*>(thisFn->Symbol.get())) {
          return thisFnSym->Value;
        }
      }
    }
  }
  Error("Failed to get current function name");
  return "";
}

bool EvaluationContext::Error(const std::string &what) {
  return Interp.PushError(EvalError { CurrentFunction.Value, what });
}

bool EvaluationContext::EvaluateError(int argNum) {
  return EvaluateError(std::to_string(argNum));
}

bool EvaluationContext::EvaluateError(const std::string &argName) {
  return Error("Failed to evaluate arg: " + argName);
}

bool EvaluationContext::UnknownSymbolError(const std::string &symName) {
  return Error("Unknown symbol: " + symName);
}

bool EvaluationContext::TypeError(const TypeInfo &expected, const ExpressionPtr &actual) {
  return TypeError(expected.TypeName, actual);
}

bool EvaluationContext::TypeError(const std::string &expectedName, const ExpressionPtr &actual) {
  return Error("Expecting: " + expectedName + ". Got: " + actual->Type().TypeName);
}

bool EvaluationContext::ArgumentExpectedError() {
  return Error("Argument expected");
}

//=============================================================================

Interpreter::Interpreter(CommandInterface &cmdInterface):
  CmdInterface { cmdInterface },
  DynamicSymbols { },
  Settings { DynamicSymbols },
  StackFrames { },
  MainFunc { },
  MainFrame {*this, MainFunc},
  TypeReducers { },
  Errors { },
  ErrorWhere { "Interpreter" },
  StopRequested_ { false }
{
  RegisterReducers();
}

InterpreterSettings& Interpreter::GetSettings() {
  return Settings;
}

std::list<EvalError> Interpreter::GetErrors() const {
  return Errors;
}

bool Interpreter::PushError(const EvalError &error) {
  Errors.push_back(error);
  return false;
}

void Interpreter::ClearErrors() {
  Errors.clear();
}

void Interpreter::Stop() {
  StopRequested_ = true;
}

bool Interpreter::StopRequested() const {
  return StopRequested_ || !CmdInterface.HasMore();
}

SymbolTable& Interpreter::GetDynamicSymbols() {
  return DynamicSymbols;
}

StackFrame& Interpreter::GetCurrentStackFrame() {
  return *(StackFrames.top());
}

void Interpreter::PushStackFrame(StackFrame &stackFrame) {
  StackFrames.push(&stackFrame);
}

void Interpreter::PopStackFrame() {
  StackFrames.pop();
}

bool Interpreter::EvaluatePartial(ExpressionPtr &expr) {
  const TypeInfo *type = &(expr->Type());
  auto search = TypeReducers.find(type);
  if (search != TypeReducers.end())
    return search->second(expr);
  else
    throw std::exception("unknown type: ");
}

bool Interpreter::Evaluate(ExpressionPtr &expr) {
  ClearErrors();
  return EvaluatePartial(expr);
}

CommandInterface& Interpreter::GetCommandInterface() {
  return CmdInterface;
}

bool Interpreter::GetCurrFrameSymbol(const std::string &symbolName, ExpressionPtr &value) {
  return GetCurrentStackFrame().GetSymbol(symbolName, value);
}

void Interpreter::RegisterReducers() {
  TypeReducers[&Int::TypeInstance]      = std::bind(&Interpreter::ReduceInt,       this, _1);
  TypeReducers[&Float::TypeInstance]    = std::bind(&Interpreter::ReduceFloat,     this, _1);
  TypeReducers[&Str::TypeInstance]      = std::bind(&Interpreter::ReduceStr,       this, _1);
  TypeReducers[&Bool::TypeInstance]     = std::bind(&Interpreter::ReduceBool,      this, _1);
  TypeReducers[&Symbol::TypeInstance]   = std::bind(&Interpreter::ReduceSymbol,    this, _1);
  TypeReducers[&Function::TypeInstance] = std::bind(&Interpreter::ReduceFunction,  this, _1);
  TypeReducers[&Sexp::TypeInstance]     = std::bind(&Interpreter::ReduceSexp,      this, _1);
  TypeReducers[&Quote::TypeInstance]    = std::bind(&Interpreter::ReduceQuote,     this, _1);
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

bool Interpreter::ReduceSymbol(ExpressionPtr &expr) {
  auto symbol = static_cast<Symbol*>(expr.get());
  ExpressionPtr symCopy = symbol->Clone();
  ExpressionPtr value;
  if (GetCurrFrameSymbol(symbol->Value, value) && value) {
    if (EvaluatePartial(value) && value) {
      if (auto copy = value->Clone()) {
        if (copy) {
          expr = std::move(copy);
          if (auto *fn = dynamic_cast<Function*>(expr.get()))
            fn->Symbol = std::move(symCopy);
          return true;
        }
        else
          throw std::exception("Clone() didn't return an Expression");
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
  if (!args.empty()) {
    ExpressionPtr firstArg = std::move(args.front());
    args.pop_front();

    if (EvaluatePartial(firstArg)) {
      args.push_front(std::move(firstArg));
      auto &funcExpr = args.front();
      if (auto *func = dynamic_cast<Function*>(funcExpr.get()))
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
  std::string error;
  auto evaluator = std::bind(&Interpreter::EvaluatePartial, this, _1);
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
    PushError(EvalError { ErrorWhere, "Invalid arguments for function" });
    return PushError(EvalError { ErrorWhere, error });
  }
}

bool Interpreter::ReduceSexpCompiledFunction(ExpressionPtr &expr, CompiledFunction &function, ArgList &args) {
  if (function.Symbol) {
    if (auto *fnSym = dynamic_cast<Symbol*>(function.Symbol.get())) {
      return function.Fn(EvaluationContext { *this, *fnSym, expr, args });
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
    auto sym = dynamic_cast<Symbol*>((*currFormal).get());
    if (sym) {
      if (EvaluatePartial(*currArg)) {
        newFrame.PutLocalSymbol(sym->Value, std::move(*currArg));
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
    newFrame.PutLocalSymbol(kv.first, std::move(kv.second->Clone()));

  if (currArg != endArg)
    return PushError(EvalError { ErrorWhere, "too many args passed to function" });
  else if (currFormal != endFormal)
    return PushError(EvalError { ErrorWhere, "not enough args passed to function" });
  else {
    ExpressionPtr codeCopy = function.Code.Value->Clone();
    if (EvaluatePartial(codeCopy)) {
      expr = std::move(codeCopy);
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
      return PushError(EvalError { ErrorWhere, "Failed to evaluate arg " + std::to_string(argNum) });
    ++argNum;
  }
  return true;
}

bool Interpreter::BuildListSexp(Sexp &wrappedSexp, ArgList &args) {
  wrappedSexp.Args.push_front(ExpressionPtr { new Symbol(Settings.GetListSexp()) });
  ArgListHelper::CopyTo(args, wrappedSexp.Args);
  return true;
}

bool Interpreter::ReduceSexpList(ExpressionPtr &expr, ArgList &args) {
  if (EvaluateArgs(args)) {
    ExpressionPtr wrappedExpr { new Sexp {} };
    auto wrappedSexp = static_cast<Sexp*>(wrappedExpr.get());
    if (!BuildListSexp(*wrappedSexp, args))
      return false;

    expr = std::move(wrappedExpr);
    return ReduceSexp(expr);
  }
  else
    return false;
}
