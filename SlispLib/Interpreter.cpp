#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <memory>

#include "Interpreter.h"
#include "Expression.h"
#include "FunctionDef.h"

using std::cout;
using std::endl;
using namespace std::placeholders;

//=============================================================================

EvalError::EvalError(const std::string &where, const std::string &what):
  Where { where },
  What { what }
{
}

//=============================================================================

void SymbolTable::PutSymbol(const std::string &symbolName, ExpressionPtr &value) {
  auto search = Symbols.find(symbolName);
  if (search != Symbols.end())
    search->second = std::move(value);
  else
    Symbols.emplace(symbolName, std::move(value));
}

void SymbolTable::PutSymbolBool(const std::string &symbolName, bool value) {
  PutSymbol(symbolName, ExpressionPtr { new Bool { value } });
}

void SymbolTable::PutSymbolString(const std::string &symbolName, const std::string &value) {
  PutSymbol(symbolName, ExpressionPtr { new String { value } });
}

void SymbolTable::PutSymbolNumber(const std::string &symbolName, int64_t value) {
  PutSymbol(symbolName, ExpressionPtr { new Number { value } });
}

void SymbolTable::PutSymbolQuote(const std::string &symbolName, ExpressionPtr &&value) {
  PutSymbol(symbolName, ExpressionPtr { new Quote { std::move(value) } });
}

void SymbolTable::PutSymbolFunction(const std::string &symbolName, Function &&func) {
  PutSymbol(symbolName, func.Clone());
}

void SymbolTable::PutSymbolFunction(const std::string &symbolName, SlipFunction fn, FuncDef &&def) {
  PutSymbol(symbolName, ExpressionPtr { new CompiledFunction { std::move(def), fn } });
}

bool SymbolTable::GetSymbol(const std::string &symbolName, ExpressionPtr &value) {
  auto it = Symbols.find(symbolName);
  if (it != Symbols.end()) {
    if (it->second)
      value = ExpressionPtr { it->second->Clone() };
    else
      value = ExpressionPtr { };
    return true;
  }
  else
    return false;
}

void SymbolTable::DeleteSymbol(const std::string &symbolName) {
  Symbols.erase(symbolName);
}

void SymbolTable::ForEach(std::function<void(const std::string &, ExpressionPtr &)> fn) {
  for (auto &sym : Symbols)
    fn(sym.first, sym.second);
}

size_t SymbolTable::GetCount() const {
  int count = 0;
  const_cast<SymbolTable*>(this)->ForEach([&count](const std::string &, ExpressionPtr &) { ++count; });
  return count;
}

//=============================================================================

Scope::Scope(SymbolTable &symbols):
  Symbols { symbols },
  ShadowedSymbols {},
  ScopedSymbols {}
{
}

Scope::~Scope() {
  for (auto &scopedSymbol : ScopedSymbols) {
    Symbols.DeleteSymbol(scopedSymbol);
  }

  ShadowedSymbols.ForEach([this](const std::string &symbolName, ExpressionPtr &value) {
    Symbols.PutSymbol(symbolName, std::move(value));
  });
}

void Scope::PutSymbol(const std::string &symbolName, ExpressionPtr &value) {
  ExpressionPtr oldValue;
  if (Symbols.GetSymbol(symbolName, oldValue)) {
    ShadowedSymbols.PutSymbol(symbolName, std::move(oldValue));
  }
  Symbols.PutSymbol(symbolName, std::move(value));
  ScopedSymbols.push_back(symbolName);
}

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

Interpreter::Interpreter(CommandInterface &cmdInterface):
  CmdInterface { cmdInterface },
  DynamicSymbols {},
  StackFrames {},
  MainFunc {},
  MainFrame {*this, MainFunc},
  TypeReducers {},
  Errors {},
  DefaultSexp { "__default_sexp__" },
  ListFuncName { "list" },
  ErrorWhere { "Interpreter" },
  StopRequested_ { false }
{
  RegisterReducers();
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
  return StopRequested_;
}

void Interpreter::PutDefaultFunction(Function &&func) {
  DynamicSymbols.PutSymbolFunction(DefaultSexp, std::move(func));
}

bool Interpreter::GetSpecialFunction(const std::string &name, FunctionPtr &func) {
  ExpressionPtr symbol;
  if (DynamicSymbols.GetSymbol(name, symbol)) {
    if (auto sym = dynamic_cast<Function*>(symbol.get())) {
      symbol.release();
      func.reset(sym);
      return true;
    }
    else
      throw std::exception("Special function not a function");
  }
  else
    return false;
}

bool Interpreter::GetDefaultFunction(FunctionPtr &func) {
  return GetSpecialFunction(DefaultSexp, func);
}

const std::string Interpreter::GetDefaultSexp() const {
  return DefaultSexp;
}

void Interpreter::PutListFunction(Function &&func) {
  DynamicSymbols.PutSymbolFunction(ListFuncName, std::move(func));
}

bool Interpreter::GetListFunction(FunctionPtr &func) {
  return GetSpecialFunction(ListFuncName, func);
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
  TypeReducers[&Number::TypeInstance]   = std::bind(&Interpreter::ReduceNumber,    this, _1);
  TypeReducers[&String::TypeInstance]   = std::bind(&Interpreter::ReduceString,    this, _1);
  TypeReducers[&Bool::TypeInstance]     = std::bind(&Interpreter::ReduceBool,      this, _1);
  TypeReducers[&Symbol::TypeInstance]   = std::bind(&Interpreter::ReduceSymbol,    this, _1);
  TypeReducers[&Function::TypeInstance] = std::bind(&Interpreter::ReduceFunction,  this, _1);
  TypeReducers[&Sexp::TypeInstance]     = std::bind(&Interpreter::ReduceSexp,      this, _1);
  TypeReducers[&Quote::TypeInstance]    = std::bind(&Interpreter::ReduceQuote,     this, _1);
}

bool Interpreter::ReduceBool(ExpressionPtr &expr) {
  return true;
}

bool Interpreter::ReduceNumber(ExpressionPtr &expr) {
  return true;
}

bool Interpreter::ReduceString(ExpressionPtr &expr) {
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
  ExpressionPtr value;
  if (GetCurrFrameSymbol(symbol->Value, value) && value) {
    if (EvaluatePartial(value) && value) {
      if (auto copy = value->Clone()) {
        if (copy) {
          expr = std::move(copy);
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
      else if (ArgDef::TypeMatches(Literal::TypeInstance, funcExpr.get()->Type())) 
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
    auto &args = e->Args;
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
  return function.Fn(*this, expr, args);
}

bool Interpreter::ReduceSexpInterpretedFunction(ExpressionPtr &expr, InterpretedFunction &function, ArgList &args) {
  StackFrame newFrame { *this, function };
  auto currArg = begin(args);
  auto currFormal = begin(function.Args);
  auto endArg = end(args);
  auto endFormal = end(function.Args);
  while (currArg != endArg && currFormal != endFormal) {
    auto sym = static_cast<Symbol*>((*currFormal).get());
    if (EvaluatePartial(*currArg)) {
      newFrame.PutLocalSymbol(sym->Value, std::move(*currArg));
      ++currArg;
      ++currFormal;
    }
    else
      return PushError(EvalError { ErrorWhere, "Evaluating argument " + sym->Value + " failed" }); 
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

bool Interpreter::ReduceSexpList(ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr listSym;
  if (GetCurrFrameSymbol(ListFuncName, listSym)) { 
    auto listFn = dynamic_cast<Function*>(listSym.get());
    if (listFn) {
      ExpressionPtr wrappedExpr { new Sexp {} };
      auto wrappedSexp = static_cast<Sexp*>(wrappedExpr.get());
      wrappedSexp->Args.push_front(listFn->Clone());
      while (!args.empty()) {
        wrappedSexp->Args.push_back(std::move(args.front()));
        args.pop_front();
      }

      expr = std::move(wrappedExpr);
      return ReduceSexp(expr);
    }
    else
      return PushError(EvalError { ErrorWhere, "list is not a function" });
  }
  else
    return PushError(EvalError { ErrorWhere, "no list function found" });
}
