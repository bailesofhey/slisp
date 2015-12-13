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

bool ArgDef::CheckArgCount(size_t expected, ArgList &args, std::string &error) const {
  return CheckArgCount(expected, expected, args, error);
}

bool ArgDef::CheckArgCount(size_t expectedMin, size_t expectedMax, ArgList &args, std::string &error) const {
  auto actualArgCount = args.size();
  if ((expectedMin == ANY_ARGS || actualArgCount >= expectedMin) &&
      (expectedMax == ANY_ARGS || actualArgCount <= expectedMax))
    return true;
  else {
    std::stringstream ss;
    ss << "Expected ";
    if (expectedMin == expectedMax)
      ss << std::to_string(expectedMin);
    else if (expectedMax == ANY_ARGS)
      ss << "at least " << std::to_string(expectedMin);
    else
      ss << "between " << std::to_string(expectedMin) << " and " << std::to_string(expectedMax);
    ss << " args, got " + std::to_string(actualArgCount);
    error = ss.str();
    return false;
  }
}

bool ArgDef::CheckArg(ExpressionEvaluator evaluator, ExpressionPtr &arg, const TypeInfo &expectedType, size_t argNum, std::string &error) const {
  if (arg && TypeHelper::TypeMatches(expectedType, arg->Type()) || evaluator(arg)) {
    if (!TypeHelper::TypeMatches(expectedType, arg->Type())) {
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

FuncDef::VarArgDef::VarArgDef(const TypeInfo &type, size_t nArgs):
  VarArgDef(type, nArgs, nArgs)
{
}

FuncDef::VarArgDef::VarArgDef(const TypeInfo &type, size_t minArgs, size_t maxArgs):
  ArgDef { },
  Type { type },
  MinArgs { minArgs },
  MaxArgs { maxArgs }
{
}

ArgDefPtr FuncDef::VarArgDef::Clone() const {
  return ArgDefPtr { new VarArgDef { *this } };
}

const std::string FuncDef::VarArgDef::ToString() const {
  if (IsAnyArgs())
    return "";
  else {
    std::stringstream ss;
    for (int i = 0; i < MinArgs; ++i)
      ss << " " << Type.TypeName;
    
    if (MaxArgs == ANY_ARGS) {
      if (ss.str().empty())
        ss << " " << Type.TypeName;
      ss << " ..";
    }
    else if (MaxArgs > MinArgs) {
      ss << " |";
      for (int i = 0; i < MaxArgs; ++i)
        ss << " " << Type.TypeName;
    }
    return ss.str();
  }
}

bool FuncDef::VarArgDef::operator==(const ArgDef &rhs) const {
  auto *varRhs = dynamic_cast<const VarArgDef*>(&rhs);
  return varRhs && *varRhs == *this;
}

bool FuncDef::VarArgDef::operator==(const VarArgDef &rhs) const {
  return &Type == &rhs.Type
      && MinArgs == rhs.MinArgs
      && MaxArgs == rhs.MaxArgs;
}

bool FuncDef::VarArgDef::ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const {
  if (ValidateArgCount(args, error)) {
    ValidateArgTypes(evaluator, args, error);
  }

  return error.empty();
}

bool FuncDef::VarArgDef::IsAnyArgs() const {
  return MinArgs == ANY_ARGS && MaxArgs == ANY_ARGS;
}

bool FuncDef::VarArgDef::ValidateArgCount(ArgList &args, std::string &error) const {
  return CheckArgCount(MinArgs, MaxArgs, args, error);
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
  return ManyArgs(Void::TypeInstance, ArgDef::NO_ARGS);
}

ArgDefPtr FuncDef::OneArg(const TypeInfo &type) {
  return ManyArgs(type, 1);
}

ArgDefPtr FuncDef::AtleastOneArg() {
  return AtleastOneArg(Sexp::TypeInstance);
}

ArgDefPtr FuncDef::AtleastOneArg(const TypeInfo &type) {
  return ManyArgs(type, 1, ArgDef::ANY_ARGS);
}

ArgDefPtr FuncDef::AnyArgs(const TypeInfo &type) {
  return ManyArgs(type, VarArgDef::ANY_ARGS);
}

ArgDefPtr FuncDef::AnyArgs() {
  return ManyArgs(Sexp::TypeInstance, ArgDef::ANY_ARGS);
}

ArgDefPtr FuncDef::ManyArgs(const TypeInfo &type, size_t nArgs) {
  return ManyArgs(type, nArgs, nArgs);
}

ArgDefPtr FuncDef::ManyArgs(const TypeInfo &type, size_t minArgs, size_t maxArgs) {
  return ArgDefPtr { new VarArgDef { type, minArgs, maxArgs } };
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
  In { val.In ? val.In->Clone().release() : nullptr },
  Out { val.Out ? val.Out->Clone().release() : nullptr }
{
}

FuncDef::FuncDef(FuncDef &&rval):
  In { std::move(rval.In) },
  Out { std::move(rval.Out) }
{
}

FuncDef FuncDef::Clone() const {
  ArgDefPtr in;
  ArgDefPtr out;
  if (In)
    in = In->Clone();
  if (Out)
    out = Out->Clone();
  return FuncDef { in, out };
}

bool FuncDef::operator==(const FuncDef &rhs) const {
  return *In == *rhs.In
      && *Out == *rhs.Out;
}

bool FuncDef::operator!=(const FuncDef &rhs) const {
  return !(*this == rhs);
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
  Def { std::move(def) },
  Symbol { }
{
}

Function::Function(const Function &rhs):
  Literal { TypeInstance },
  Def { rhs.Def },
  Symbol { }
{
  if (rhs.Symbol)
    Symbol = rhs.Symbol->Clone();
}

bool Function::operator==(const Function &rhs) const {
  return Def == rhs.Def &&
         (Symbol && rhs.Symbol && *Symbol == *rhs.Symbol);
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
  if (auto *fn = dynamic_cast<const CompiledFunction*>(&rhs))
    return *this == *fn;
  return false;
}

bool CompiledFunction::operator==(const CompiledFunction &rhs) const {
  return static_cast<const Function&>(*this) == static_cast<const Function&>(rhs);
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
  if (auto *fn = dynamic_cast<const InterpretedFunction*>(&rhs))
    return *this == *fn;
  return false;
}

bool InterpretedFunction::operator==(const InterpretedFunction &rhs) const {
  return static_cast<const Function&>(*this) == static_cast<const Function&>(rhs)
      && Code == rhs.Code
      && ArgListHelper::AreEqual(Args, rhs.Args)
      && Closure == rhs.Closure;
}

bool InterpretedFunction::operator!=(const InterpretedFunction &rhs) const {
  return !(rhs == *this);
}

//=============================================================================

//TODO: Need to account for inheritance in TypeInfo itself
bool TypeHelper::TypeMatches(const TypeInfo &expected, const TypeInfo &actual) {
  if (&expected == &Literal::TypeInstance)
    return IsLiteral(actual);
  else if (&expected == &Sexp::TypeInstance)
    return true;
  else if (&expected == &Number::TypeInstance)
    return IsConvertableToNumber(actual);
  else
    return &expected == &actual;
}

bool TypeHelper::IsFunction(const TypeInfo &type) {
  return &type == &Function::TypeInstance 
      || &type == &CompiledFunction::TypeInstance 
      || &type == &InterpretedFunction::TypeInstance
      ;
}

bool TypeHelper::IsAtom(const TypeInfo &type) {
  return &type == &Bool::TypeInstance
      || &type == &Number::TypeInstance 
      || &type == &String::TypeInstance
      || IsFunction(type)
      ;
}

bool TypeHelper::IsLiteral(const TypeInfo &type) {
  return IsAtom(type)
      || &type == &Literal::TypeInstance
      || &type == &Quote::TypeInstance
      ;
}

// implicit conversion from bool to number - a good thing?
bool TypeHelper::IsConvertableToNumber(const TypeInfo &type) {
  //return &type == &Number::TypeInstance
  //    || &type == &Bool::TypeInstance
  //    ;
  return &type == &Number::TypeInstance;
}

ExpressionPtr TypeHelper::GetNumber(ExpressionPtr &expr) {
  //if (expr) {
  //  if (&expr->Type() == &Number::TypeInstance)
  //    return expr->Clone();
  //  else if (&expr->Type() == &Bool::TypeInstance)
  //    return ExpressionPtr { new Number(static_cast<Bool&>(*expr).Value) };
  //}
  //return ExpressionPtr {};

  if (expr) {
    if (&expr->Type() == &Number::TypeInstance)
      return expr->Clone();
  }
  return ExpressionPtr {};
}

ExpressionPtr TypeHelper::GetBool(ExpressionPtr &expr) {
  return expr->Clone();
}