#include <string>
#include <sstream>
#include <initializer_list>
#include <vector>
#include <memory>
#include <functional>

#include "Expression.h"
#include "FunctionDef.h"

//=============================================================================

ArgDef::~ArgDef() {
}

bool ArgDef::Validate(ExpressionEvaluator evaluator, ExpressionPtr &expr, std::string &error) const {
  if (auto sexp = dynamic_cast<Sexp*>(expr.get())) {
    auto &args = sexp->Args;
    if (!args.empty()) {
      ExpressionPtr fnExpr = std::move(args.front());
      args.pop_front();
      auto fn = dynamic_cast<Function*>(fnExpr.get());
      if (fn) {
        ValidateArgs(evaluator, args, error);
      }
      else
        throw std::exception("first argument in sexp must be a function");
    }
    else
      throw std::exception("sexp requires at least one argument");
  }
  else
    error = "Expected: Sexp. Actual: " + expr->Type().TypeName;

  return error.empty();
}

//TODO: Need to account for inheritance in TypeInfo itself
bool ArgDef::TypeMatches(const TypeInfo &expected, const TypeInfo &actual) {
  if (&expected == &Literal::TypeInstance)
    return IsLiteral(actual);
  else if (&expected == &Sexp::TypeInstance)
    return true;
  else
    return &expected == &actual;
}

bool ArgDef::IsFunction(const TypeInfo &type) {
  return &type == &Function::TypeInstance 
      || &type == &CompiledFunction::TypeInstance 
      || &type == &InterpretedFunction::TypeInstance
      ;
}

bool ArgDef::IsLiteral(const TypeInfo &type) {
  return &type == &Literal::TypeInstance
      || &type == &Bool::TypeInstance
      || &type == &Number::TypeInstance 
      || &type == &String::TypeInstance
      || IsFunction(type)
      || &type == &Quote::TypeInstance
      ;
}

bool ArgDef::CheckArgCount(int expected, ArgList &args, std::string &error) const {
  int actualArgCount = args.size();
  int expectedArgCount = expected;
  if (actualArgCount == expectedArgCount)
    return true;
  else {
    error = "Expected " + std::to_string(expectedArgCount) + " args, got " + std::to_string(actualArgCount);
    return false;
  }
}

bool ArgDef::CheckArg(ExpressionEvaluator evaluator, ExpressionPtr &arg, const TypeInfo &expectedType, int argNum, std::string &error) const {
  if (TypeMatches(expectedType, arg->Type()) || evaluator(arg)) {
    if (!TypeMatches(expectedType, arg->Type())) {
      error = "Argument " + std::to_string(argNum) + ": Expected " + expectedType.TypeName + ", got " + arg->Type().TypeName;
      return false;
    }
    else
      return true;
  }
  else {
    error = "Argument " + std::to_string(argNum) + ": Failed to evaluate";
    return false;
  }
}

//=============================================================================

VarArgDef::VarArgDef(const TypeInfo& type, int nargs):
  ArgDef { },
  Type { type },
  NArgs { nargs }
{
}

std::unique_ptr<ArgDef> VarArgDef::Clone() const {
  return std::unique_ptr<ArgDef> { new VarArgDef { *this } };
}

const std::string VarArgDef::ToString() const {
  if (NArgs == NO_ARGS)
    return "";
  else if (NArgs == ANY_ARGS) 
    return " " + Type.TypeName + " ..";
  else {
    std::stringstream ss;
    for (int i = 0; i < NArgs; ++i)
      ss << " " << Type.TypeName;
    return ss.str();
  }
}

bool VarArgDef::ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const {
  if (ValidateArgCount(args, error)) {
   // if (!TypeMatches(Type, Quote::TypeInstance))
      ValidateArgTypes(evaluator, args, error);
  }

  return error.empty();
}

bool VarArgDef::ValidateArgCount(ArgList &args, std::string &error) const {
  if (NArgs == ANY_ARGS)
    return true;
  else 
    return CheckArgCount(NArgs, args, error);
}

bool VarArgDef::ValidateArgTypes(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const {
  int argNum = 0;
  for (auto &arg : args) {
    ++argNum;
    if (!CheckArg(evaluator, arg, Type, argNum, error))
        return false;
  }
  return true;
}

//=============================================================================

ListArgDef::ListArgDef(std::initializer_list<const TypeInfo*> types):
  ArgDef {},
  Types { types }
{
}

std::unique_ptr<ArgDef> ListArgDef::Clone() const {
  return std::unique_ptr<ArgDef> { new ListArgDef { *this } };
}

const std::string ListArgDef::ToString() const {
  std::stringstream ss;
  for (auto type : Types) {
    ss << " " << type->TypeName;
  }
  return ss.str();
}

bool ListArgDef::ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const {
  if (CheckArgCount(Types.size(), args, error)) {
    int argNum = 0;
    auto currExpectedArg = Types.begin();
    auto currActualArg = args.begin();

    while (currExpectedArg != Types.end() && currActualArg != args.end()) {
      ++argNum;
      if (!CheckArg(evaluator, *currActualArg, **currExpectedArg, argNum, error))
        return false;

      ++currExpectedArg;
      ++currActualArg;
    }

    return true;
  }
  return false;
}
  
//=============================================================================

std::unique_ptr<ArgDef> FuncDef::NoArgs() {
  return ManyArgs(Void::TypeInstance, VarArgDef::NO_ARGS);
}

std::unique_ptr<ArgDef> FuncDef::OneArg(const TypeInfo& type) {
  return ManyArgs(type, 1);
}

std::unique_ptr<ArgDef> FuncDef::AnyArgs(const TypeInfo& type) {
  return ManyArgs(type, VarArgDef::ANY_ARGS);
}

std::unique_ptr<ArgDef> FuncDef::AnyArgs() {
  return ManyArgs(Sexp::TypeInstance, VarArgDef::ANY_ARGS);
}

std::unique_ptr<ArgDef> FuncDef::ManyArgs(const TypeInfo& type, int nargs) {
  return std::unique_ptr<ArgDef> { new VarArgDef { type, nargs } };
}

std::unique_ptr<ArgDef> FuncDef::Args(std::initializer_list<const TypeInfo*> args) {
  return std::unique_ptr<ArgDef> { new ListArgDef { args } };
}

FuncDef::FuncDef( std::unique_ptr<ArgDef> &in, std::unique_ptr<ArgDef> &out):
  In { std::move(in) },
  Out { std::move(out) }
{
}

FuncDef::FuncDef(const FuncDef &val):
  In { val.In->Clone().release() },
  Out { val.Out->Clone().release() }
{
}

FuncDef::FuncDef(FuncDef &&rval):
  In { std::move(rval.In) },
  Out { std::move(rval.Out) }
{
}

FuncDef& FuncDef::operator=(FuncDef func) {
  Swap(func);
  return *this;
}

void FuncDef::Swap(FuncDef &func) {
  using std::swap;
  swap(In, func.In);
  swap(Out, func.Out);
}

bool FuncDef::ValidateArgs(ExpressionEvaluator evaluator, ExpressionPtr &expr, std::string &error) {
  std::stringstream ss;
  std::string tmpError;
  if (!In->Validate(evaluator, expr, tmpError)) {
    ss << "Wrong input args: " << tmpError;
    error = ss.str();
    return false;
  }
  return true;
}

const std::string FuncDef::ToString() const {
  std::stringstream ss;
  ss << In->ToString() << ") -> " << Out->ToString();
  return ss.str();
}

//=============================================================================

const TypeInfo Function::TypeInstance("function");

Function::Function():
  Function {
    FuncDef { std::unique_ptr<ArgDef> { }, std::unique_ptr<ArgDef> { } } 
  }
{
}

Function::Function(FuncDef &&def):
  Literal { TypeInstance },
  Def { std::move(def) }
{
}

//=============================================================================

const TypeInfo CompiledFunction::TypeInstance("compiledfunction");

CompiledFunction::CompiledFunction(FuncDef &&def, SlipFunction fn):
  Function { std::move(def) },
  Fn { fn }
{
}

CompiledFunction::CompiledFunction():
  Function {},
  Fn { nullptr }
{
}

ExpressionPtr CompiledFunction::Clone() const {
  return ExpressionPtr { new CompiledFunction(*this) };
}

const std::string CompiledFunction::ToString() const {
  return "CompiledFunction { }";
}

CompiledFunction& CompiledFunction::operator=(CompiledFunction rhs) {
  Swap(rhs);
  return *this;
}

void CompiledFunction::Swap(CompiledFunction &rhs) {
  Def = rhs.Def;
  Fn = rhs.Fn;
}

//=============================================================================

const TypeInfo InterpretedFunction::TypeInstance("interpretedfunction");

InterpretedFunction::InterpretedFunction(FuncDef &&def, ExpressionPtr &&code, ArgList &&args):
  Function { std::move(def) },
  Code { std::move(code) },
  Args { std::move(args) },
  Closure { }
{
}

ExpressionPtr InterpretedFunction::Clone() const {
  ArgList argsCopy;
  for (auto &arg : Args) {
    argsCopy.push_back(arg->Clone());
  }
  ExpressionPtr copy { new InterpretedFunction { FuncDef { this->Def }, Code.Value->Clone(), std::move(argsCopy) } };
  auto interpFunc = static_cast<InterpretedFunction*>(copy.get());
  for (auto &kv : Closure) {
    interpFunc->Closure.emplace(kv.first, kv.second->Clone());
  }
  return copy;
}

const std::string InterpretedFunction::ToString() const {
  return "InterpretedFunction { }";
}

