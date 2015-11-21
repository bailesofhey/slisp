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
  if (expr) {
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
  }
  else
    throw std::exception("ExpressionPtr is empty");

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

FuncDef::VarArgDef::VarArgDef(const TypeInfo& type, int nargs):
  ArgDef { },
  Type { type },
  NArgs { nargs }
{
}

ArgDefPtr FuncDef::VarArgDef::Clone() const {
  return ArgDefPtr { new VarArgDef { *this } };
}

const std::string FuncDef::VarArgDef::ToString() const {
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

bool FuncDef::VarArgDef::operator==(const ArgDef &rhs) const {
  auto *varRhs = dynamic_cast<const VarArgDef*>(&rhs);
  return varRhs && *varRhs == *this;
}

bool FuncDef::VarArgDef::operator==(const VarArgDef &rhs) const {
  return &Type == &rhs.Type
      && NArgs == rhs.NArgs;
}

bool FuncDef::VarArgDef::ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const {
  if (ValidateArgCount(args, error)) {
   // if (!TypeMatches(Type, Quote::TypeInstance))
      ValidateArgTypes(evaluator, args, error);
  }

  return error.empty();
}

bool FuncDef::VarArgDef::ValidateArgCount(ArgList &args, std::string &error) const {
  if (NArgs == ANY_ARGS)
    return true;
  else 
    return CheckArgCount(NArgs, args, error);
}

bool FuncDef::VarArgDef::ValidateArgTypes(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const {
  int argNum = 0;
  for (auto &arg : args) {
    ++argNum;
    if (!CheckArg(evaluator, arg, Type, argNum, error))
        return false;
  }
  return true;
}

//=============================================================================

FuncDef::ListArgDef::ListArgDef(std::initializer_list<const TypeInfo*> &&types):
  ArgDef {},
  Types { std::move(types) }
{
}

ArgDefPtr FuncDef::ListArgDef::Clone() const {
  return ArgDefPtr { new ListArgDef { *this } };
}

const std::string FuncDef::ListArgDef::ToString() const {
  std::stringstream ss;
  for (auto type : Types) {
    ss << " " << type->TypeName;
  }
  return ss.str();
}

bool FuncDef::ListArgDef::operator==(const ArgDef &rhs) const {
  auto *lstRhs = dynamic_cast<const ListArgDef*>(&rhs);
  return lstRhs && *lstRhs == *this;
}

bool FuncDef::ListArgDef::operator==(const ListArgDef &rhs) const {
  return Types == rhs.Types;
}

bool FuncDef::ListArgDef::ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const {
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

ArgDefPtr FuncDef::NoArgs() {
  return ManyArgs(Void::TypeInstance, VarArgDef::NO_ARGS);
}

ArgDefPtr FuncDef::OneArg(const TypeInfo& type) {
  return ManyArgs(type, 1);
}

ArgDefPtr FuncDef::AnyArgs(const TypeInfo& type) {
  return ManyArgs(type, VarArgDef::ANY_ARGS);
}

ArgDefPtr FuncDef::AnyArgs() {
  return ManyArgs(Sexp::TypeInstance, VarArgDef::ANY_ARGS);
}

ArgDefPtr FuncDef::ManyArgs(const TypeInfo& type, int nargs) {
  return ArgDefPtr { new VarArgDef { type, nargs } };
}

ArgDefPtr FuncDef::Args(std::initializer_list<const TypeInfo*> &&args) {
  return ArgDefPtr { new ListArgDef { std::move(args) } };
}

FuncDef::FuncDef(ArgDefPtr &in, ArgDefPtr &out):
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

bool FuncDef::operator==(const FuncDef &rhs) const {
  return In == rhs.In
      && Out == rhs.Out;
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
    FuncDef { ArgDefPtr {}, ArgDefPtr {} }
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

bool CompiledFunction::operator==(const Expression &rhs) const {
  return &rhs.Type() == &CompiledFunction::TypeInstance
      && dynamic_cast<const CompiledFunction&>(rhs) == *this;
}

bool CompiledFunction::operator==(const CompiledFunction &rhs) const {
  return Def == rhs.Def
      && false; //eventually support comparing functions
}

bool CompiledFunction::operator!=(const CompiledFunction &rhs) const {
  return !(rhs == *this);
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

bool InterpretedFunction::operator==(const Expression &rhs) const {
  return &rhs.Type() == &InterpretedFunction::TypeInstance
      && dynamic_cast<const InterpretedFunction&>(rhs) == *this;
}

bool InterpretedFunction::operator==(const InterpretedFunction &rhs) const {
  return Code == rhs.Code
      && ArgListHelper::AreEqual(Args, rhs.Args)
      && Closure == rhs.Closure;
}

bool InterpretedFunction::operator!=(const InterpretedFunction &rhs) const {
  return !(rhs == *this);
}
