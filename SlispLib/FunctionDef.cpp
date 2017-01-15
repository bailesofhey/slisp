#include <string>
#include <sstream>
#include <initializer_list>
#include <vector>
#include <memory>
#include <functional>

#include "Expression.h"
#include "FunctionDef.h"

using namespace std;

//=============================================================================

ArgDef::~ArgDef() {
}

bool ArgDef::Validate(ExpressionEvaluator evaluator, ExpressionPtr &expr, string &error) const {
  if (expr) {
    if (auto sexp = TypeHelper::GetValue<Sexp>(expr)) {
      auto &args = sexp->Args;
      if (!args.empty()) {
        ExpressionPtr fnExpr = move(args.front());
        args.pop_front();
        if (auto fn = TypeHelper::GetValue<Function>(fnExpr)) {
          bool result = ValidateArgs(evaluator, args, error);
          args.push_front(move(fnExpr));
          return result;
        }
        else
          throw invalid_argument("first argument in sexp must be a function");
      }
      else
        throw invalid_argument("sexp requires at least one argument");
    }
    else
      error = "Expected: Sexp. Actual: " + expr->Type().Name();
  }
  else
    throw invalid_argument("ExpressionPtr is empty");

  return error.empty();
}

bool ArgDef::CheckArgCount(size_t expected, ArgList &args, string &error) const {
  return CheckArgCount(expected, expected, args, error);
}

bool ArgDef::CheckArgCount(size_t expectedMin, size_t expectedMax, ArgList &args, string &error) const {
  auto actualArgCount = args.size();
  if ((expectedMin == ANY_ARGS || actualArgCount >= expectedMin) &&
      (expectedMax == ANY_ARGS || actualArgCount <= expectedMax))
    return true;
  else {
    stringstream ss;
    ss << "Expected ";
    if (expectedMin == expectedMax)
      ss << to_string(expectedMin);
    else if (expectedMax == ANY_ARGS)
      ss << "at least " << to_string(expectedMin);
    else
      ss << "between " << to_string(expectedMin) << " and " << to_string(expectedMax);
    ss << " args, got " + to_string(actualArgCount);
    error = ss.str();
    return false;
  }
}

bool ArgDef::CheckArg(ExpressionEvaluator evaluator, ExpressionPtr &arg, const TypeInfo &expectedType, size_t argNum, string &error) const {
  if (arg && TypeHelper::TypeMatches(expectedType, arg->Type()) || evaluator(arg)) {
    if (&expectedType == &Literal::TypeInstance && &arg->Type() == &Quote::TypeInstance) {
      return true;
    }

    if (!TypeHelper::TypeMatches(expectedType, arg)) {
      error = "Argument " + to_string(argNum) + ": Expected " + expectedType.Name() + ", got " + arg->Type().Name();
      return false;
    }
    else
      return true;
  }
  else {
    error = "Argument " + to_string(argNum) + ": Failed to evaluate";
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

const string FuncDef::VarArgDef::ToString() const {
  if (IsAnyArgs())
    return "";
  else {
    stringstream ss;
    for (size_t i = 0; i < MinArgs; ++i)
      ss << " " << Type.Name();
    
    if (MaxArgs == ANY_ARGS) {
      if (ss.str().empty())
        ss << " " << Type.Name();
      ss << " ..";
    }
    else if (MaxArgs > MinArgs) {
      ss << " |";
      for (size_t i = 0; i < MaxArgs; ++i)
        ss << " " << Type.Name();
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

bool FuncDef::VarArgDef::ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, string &error) const {
  if (ValidateArgCount(args, error)) {
    ValidateArgTypes(evaluator, args, error);
  }

  return error.empty();
}

bool FuncDef::VarArgDef::IsAnyArgs() const {
  return MinArgs == ANY_ARGS && MaxArgs == ANY_ARGS;
}

bool FuncDef::VarArgDef::ValidateArgCount(ArgList &args, string &error) const {
  return CheckArgCount(MinArgs, MaxArgs, args, error);
}

bool FuncDef::VarArgDef::ValidateArgTypes(ExpressionEvaluator evaluator, ArgList &args, string &error) const {
  int argNum = 0;
  for (auto &arg : args) {
    ++argNum;
    if (!CheckArg(evaluator, arg, Type, argNum, error))
        return false;
  }
  return true;
}

//=============================================================================

FuncDef::ListArgDef::ListArgDef(initializer_list<const TypeInfo*> &&types):
  ArgDef {},
  Types { move(types) }
{
}

ArgDefPtr FuncDef::ListArgDef::Clone() const {
  return ArgDefPtr { new ListArgDef { *this } };
}

const string FuncDef::ListArgDef::ToString() const {
  stringstream ss;
  for (auto type : Types) {
    ss << " " << type->Name();
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

bool FuncDef::ListArgDef::ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, string &error) const {
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

ArgDefPtr FuncDef::Args(initializer_list<const TypeInfo*> &&args) {
  return ArgDefPtr { new ListArgDef { move(args) } };
}

FuncDef::FuncDef(ArgDefPtr in, ArgDefPtr out):
  In { move(in) },
  Out { move(out) }
{
}

FuncDef::FuncDef(const FuncDef &val):
  In { val.In ? val.In->Clone().release() : nullptr },
  Out { val.Out ? val.Out->Clone().release() : nullptr }
{
}

FuncDef::FuncDef(FuncDef &&rval):
  In { move(rval.In) },
  Out { move(rval.Out) }
{
}

FuncDef FuncDef::Clone() const {
  ArgDefPtr in;
  ArgDefPtr out;
  if (In)
    in = In->Clone();
  if (Out)
    out = Out->Clone();
  return FuncDef { move(in), move(out) };
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
  swap(In, func.In);
  swap(Out, func.Out);
}

bool FuncDef::ValidateArgs(ExpressionEvaluator evaluator, ExpressionPtr &expr, string &error) {
  stringstream ss;
  string tmpError;
  if (!In->Validate(evaluator, expr, tmpError)) {
    if (!Name.empty())
      ss << Name << ": ";
    ss <<"Wrong input args: " << tmpError;
    error = ss.str();
    return false;
  }
  return true;
}

const string FuncDef::ToString() const {
  stringstream ss;
  ss << In->ToString() << ") -> " << Out->ToString();
  return ss.str();
}

//=============================================================================

const TypeInfo Function::TypeInstance("fn", TypeInfo::NewUndefined);

Function::Function(const SourceContext &sourceContext, const TypeInfo &typeInfo):
  Function {
    sourceContext,
    Function::TypeInstance,
    FuncDef { ArgDefPtr {}, ArgDefPtr {} }
  }
{
}

Function::Function(const SourceContext &sourceContext, const TypeInfo &typeInfo, FuncDef &&def):
  Function(sourceContext, typeInfo, move(def), ExpressionPtr {})
{
}

Function::Function(const SourceContext &sourceContext, const TypeInfo &typeInfo, FuncDef &&def, ExpressionPtr &&sym):
  Literal { sourceContext, typeInfo },
  Def { move(def) },
  Symbol { move(sym) }
{
  Def.Name = SymbolName();
}

Function::Function(const SourceContext &sourceContext, const TypeInfo &typeInfo, FuncDef &&def, ExpressionPtr &sym):
  Function(sourceContext, typeInfo, move(def), move(sym))
{
}

Function::Function(const Function &rhs):
  Literal { rhs.GetSourceContext(), rhs.Type() },
  Def { rhs.Def },
  Symbol { },
  Signatures { rhs.Signatures },
  Doc { rhs.Doc },
  Examples { rhs.Examples }
{
  if (rhs.Symbol)
    Symbol = rhs.Symbol->Clone();
  Def.Name = SymbolName();
}

void Function::Display(ostream &out) const {
  out << "<Function";
  if (Symbol) {
    out << ":";
    if (auto *sym = dynamic_cast<::Symbol*>(Symbol.get()))
      out << *sym;
  }
  out << ">";
}

bool Function::operator==(const Function &rhs) const {
  return Def == rhs.Def &&
         (Symbol && rhs.Symbol && *Symbol == *rhs.Symbol) &&
         Signatures == rhs.Signatures &&
         Doc == rhs.Doc;
}

std::string Function::SymbolName() const {
  if (!Def.Name.empty())
    return Def.Name;
  if (Symbol) {
    if (auto *sym = dynamic_cast<::Symbol*>(Symbol.get()))
      return sym->Value;
  }
  return "";
}

//=============================================================================

const TypeInfo CompiledFunction::TypeInstance("compiled-fn", TypeInfo::NewUndefined);
const CompiledFunction CompiledFunction::Null(NullSourceContext);

CompiledFunction::CompiledFunction(const SourceContext &sourceContext):
  Function { sourceContext, TypeInstance },
  Fn { nullptr }
{
}

CompiledFunction::CompiledFunction(const CompiledFunction &rhs):
  Function(rhs),
  Fn { rhs.Fn }
{
}

CompiledFunction::CompiledFunction(const SourceContext &sourceContext, FuncDef &&def, SlipFunction fn):
  Function { sourceContext, TypeInstance, move(def) },
  Fn { fn }
{
}

ExpressionPtr CompiledFunction::Clone() const {
  return ExpressionPtr { new CompiledFunction(*this) };
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

const TypeInfo InterpretedFunction::TypeInstance("interpreted-fn", TypeInfo::NewUndefined);
const InterpretedFunction InterpretedFunction::Null(NullSourceContext);

InterpretedFunction::InterpretedFunction(const InterpretedFunction &rhs):
  Function(rhs),
  Code { rhs.GetSourceContext(), ExpressionPtr { } },
  Args { },
  Closure { }
{
  ArgListHelper::CopyTo(rhs.Args, Args);
  if (rhs.Code.Value)
    Code.Value = move(rhs.Code.Value->Clone());
  for (auto &kv : rhs.Closure) 
    Closure.emplace(kv.first, kv.second->Clone());
}

InterpretedFunction::InterpretedFunction(const SourceContext &sourceContext, FuncDef &&def, ExpressionPtr &&code, ArgList &&args):
  Function { sourceContext, TypeInstance, move(def) },
  Code { sourceContext, move(code) },
  Args { move(args) },
  Closure { }
{
}

InterpretedFunction::InterpretedFunction(const SourceContext &sourceContext):
  Function { sourceContext, TypeInstance },
  Code { sourceContext, ExpressionPtr {} },
  Args {},
  Closure {}
{
}

ExpressionPtr InterpretedFunction::Clone() const {
  return ExpressionPtr { new InterpretedFunction(*this) };
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
const ExpressionPtr TypeHelper::Null;

bool TypeHelper::IsQuoteAList(const Quote &quote) {
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

std::string TypeHelper::TypeName(const ExpressionPtr &expr) {
  const TypeInfo *type = nullptr;
  if (auto quote = TypeHelper::GetValue<Quote>(expr)) {
    if (TypeHelper::IsQuoteAList(*quote)) 
      type = &(List::TypeInstance);
    else
      type = &(quote->Type());
  }
  else if (auto ref = dynamic_cast<Ref*>(expr.get()))
    type = &(ref->Value->Type());
  else
    type = &(expr->Type());

  if (TypeHelper::SimpleIsA<Function>(*type))
    return Function::TypeInstance.Name();
  else
    return type->Name();
}

//TODO: Need to account for inheritance in TypeInfo itself
bool TypeHelper::TypeMatches(const TypeInfo &expected, const TypeInfo &actual) {
  if (SimpleIsA<Literal>(expected))
    return IsA<Literal>(actual);
  else if (SimpleIsA<Sexp>(expected))
    return true;
  else
    return &expected == &actual;
}

bool TypeHelper::TypeMatches(const TypeInfo &expected, const ExpressionPtr &actualExpr) {
  if (SimpleIsA<Literal>(expected))
    return IsA<Literal>(actualExpr);
  else if (SimpleIsA<Sexp>(expected))
    return true;
  else if (SimpleIsA<Function>(expected))
    return IsA<Function>(actualExpr); 
  else if (auto ref = dynamic_cast<Ref*>(actualExpr.get()))
    return TypeMatches(expected, ref->Value);
  else
    return &expected == &actualExpr->Type();
}

bool TypeHelper::IsAtom(const ExpressionPtr &expr) {
  return IsAtom(expr->Type());
}

bool TypeHelper::IsAtom(const TypeInfo &type) {
  return IsA<Bool>(type)
      || IsA<Int>(type)
      || IsA<Float>(type)
      || IsA<Str>(type)
      || IsA<Function>(type)
      ;
}

