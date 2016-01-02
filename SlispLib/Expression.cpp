#include <string>
#include <cinttypes>
#include <memory>
#include <sstream>
#include <iostream>

#include "Expression.h"

//=============================================================================

TypeInfo::TypeInfo(const std::string &typeName):
  TypeName { typeName }
{
}

//=============================================================================

Expression::Expression(const TypeInfo& typeInfo):
  _Type { typeInfo }
{ 
}

Expression::~Expression() {
}

const TypeInfo& Expression::Type() const {
  return _Type;
}

bool Expression::operator!=(const Expression &rhs) const {
  return !(rhs == *this);
}

bool Expression::AreEqual(const ExpressionPtr &lhs, const ExpressionPtr &rhs) {
  if (lhs && rhs)
    return *lhs == *rhs;
  else
    return static_cast<bool>(lhs) == static_cast<bool>(rhs);
}

const std::string Expression::ToString() const {
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

std::ostream& operator<<(std::ostream &out, const Expression &expr) {
  expr.Print(out);
  return out;
};

//=============================================================================

const TypeInfo Void::TypeInstance { "" };

//=============================================================================

const TypeInfo Literal::TypeInstance { "literal" };

Literal::Literal(const TypeInfo& typeInfo):
  Expression { typeInfo }
{
}

//=============================================================================

const TypeInfo Bool::TypeInstance("bool");

Bool::Bool():
  Bool { false }
{
}

Bool::Bool(bool value):
  Literal { TypeInstance },
  Value { value }
{
}

ExpressionPtr Bool::Clone() const {
  return ExpressionPtr { new Bool(*this) };
}

bool Bool::operator==(const Expression &rhs) const {
  return &rhs.Type() == &Bool::TypeInstance
      && dynamic_cast<const Bool&>(rhs) == *this;
}

bool Bool::operator==(const Bool &rhs) const {
  return Value == rhs.Value;
}

bool Bool::operator!=(const Bool &rhs) const {
  return !(*this == rhs);
}

bool Bool::operator<(const Bool &rhs) const {
  return Value < rhs.Value;
}

bool Bool::operator>=(const Bool &rhs) const {
  return !(*this < rhs);
}

Bool& Bool::operator=(Bool rhs) {
  Swap(rhs);
  return *this;
}

void Bool::Swap(Bool &rhs) {
  Value = rhs.Value;
}

void Bool::Print(std::ostream &out) const {
  if (Value)
    out << "true";
  else
    out << "false";
}
//=============================================================================

const TypeInfo Int::TypeInstance("int");

Int::Int():
  Int { 0 }
{
}

Int::Int(int64_t value):
  Literal { TypeInstance },
  Value { value }
{
}

ExpressionPtr Int::Clone() const {
  return ExpressionPtr { new Int(*this) };
}

bool Int::operator==(const Expression &rhs) const {
  return &rhs.Type() == &Int::TypeInstance
      && dynamic_cast<const Int&>(rhs) == *this;
}

bool Int::operator==(const Int &rhs) const {
  return Value == rhs.Value;
}

bool Int::operator!=(const Int &rhs) const {
  return !(*this == rhs);
}

bool Int::operator<(const Int &rhs) const {
  return Value < rhs.Value;
}

bool Int::operator>=(const Int &rhs) const {
  return !(*this < rhs);
}

Int& Int::operator=(Int rhs) {
  Swap(rhs);
  return *this;
}

void Int::Swap(Int &rhs) {
  Value = rhs.Value;
}

void Int::Print(std::ostream &out) const {
  out << Value;
}
//=============================================================================

const TypeInfo String::TypeInstance("string");

String::String():
  String { "" }
{ 
}

String::String(const std::string &value):
  Literal { TypeInstance },
  Value { value }
{
}

ExpressionPtr String::Clone() const {
  return ExpressionPtr { new String(*this) };
}

bool String::operator==(const Expression &rhs) const {
  return &rhs.Type() == &String::TypeInstance
      && dynamic_cast<const String&>(rhs) == *this;
}

bool String::operator==(const String &rhs) const {
  return Value == rhs.Value;
}

bool String::operator!=(const String &rhs) const {
  return !(*this == rhs);
}

bool String::operator<(const String &rhs) const {
  return Value < rhs.Value;
}

bool String::operator>=(const String &rhs) const {
  return !(*this < rhs);
}

String& String::operator=(String rhs) {
  Swap(rhs);
  return *this;
}

void String::Swap(String &rhs) {
  Value = rhs.Value;
}

void String::Print(std::ostream &out) const {
  out << "\"" << Value << "\"";
}

//=============================================================================

const TypeInfo Quote::TypeInstance("quote");

Quote::Quote(ExpressionPtr &&expr):
  Literal { TypeInstance },
  Value { std::move(expr) }
{
}

ExpressionPtr Quote::Clone() const {
  return ExpressionPtr { new Quote(std::move(Value->Clone())) };
}

bool Quote::operator==(const Expression &rhs) const {
  return &rhs.Type() == &Quote::TypeInstance
      && dynamic_cast<const Quote&>(rhs) == *this;
}

bool Quote::operator==(const Quote &rhs) const {
  return AreEqual(Value, rhs.Value);
}

bool Quote::operator!=(const Quote &rhs) const {
  return !(rhs == *this);
}

void Quote::Print(std::ostream &out) const {
  out << *Value;
}

//=============================================================================

const TypeInfo Symbol::TypeInstance("symbol");

Symbol::Symbol(const std::string &value):
  Expression { TypeInstance },
  Value { value }
{
}

ExpressionPtr Symbol::Clone() const {
  return ExpressionPtr { new Symbol(*this) };
}

bool Symbol::operator==(const Expression &rhs) const {
  return &rhs.Type() == &Symbol::TypeInstance
      && dynamic_cast<const Symbol&>(rhs) == *this;
}

bool Symbol::operator==(const Symbol& rhs) const {
  return Value == rhs.Value;
}

bool Symbol::operator!=(const Symbol& rhs) const {
  return !(*this == rhs);
}

bool Symbol::operator<(const Symbol &rhs) const {
  return Value < rhs.Value;
}

bool Symbol::operator>=(const Symbol &rhs) const {
  return !(*this < rhs);
}

Symbol& Symbol::operator=(Symbol rhs) {
  Swap(rhs);
  return *this;
}

void Symbol::Swap(Symbol &rhs) {
  Value = rhs.Value;
}

void Symbol::Print(std::ostream &out) const {
  out << Value;
}

//=============================================================================

bool ArgListHelper::AreEqual(const ArgList &lhs, const ArgList &rhs) {
  if (lhs.size() == rhs.size()) {
    auto lCurr = lhs.begin();
    auto rCurr = rhs.begin();
    while (lCurr != lhs.end() && rCurr != rhs.end()) {
      if (!Expression::AreEqual(*lCurr, *rCurr))
        return false;
      ++lCurr;
      ++rCurr;
    }
    return true;
  }
  return false;
}

void ArgListHelper::CopyTo(const ArgList &src, ArgList &dst) {
  for (auto &arg : src) {
    dst.push_back(arg->Clone());
  }
}

//=============================================================================

const TypeInfo Sexp::TypeInstance("sexp");

Sexp::Sexp():
  Expression { TypeInstance }
{
}

Sexp::Sexp(ArgList &&args):
  Sexp()
{
  Args = std::move(args);
}

Sexp::Sexp(std::initializer_list<ExpressionPtr> &&args):
  Sexp()
{
  if (args.size()) {
    for (auto &arg : args) {
      if (arg)
        Args.push_back(arg->Clone());
    }
  }
}

ExpressionPtr Sexp::Clone() const {
  ExpressionPtr copy { new Sexp };
  Sexp *sexpCopy = static_cast<Sexp*>(copy.get());
  for (auto &arg : Args)
    sexpCopy->Args.push_back(arg->Clone());
  return copy;
}

bool Sexp::operator==(const Expression &rhs) const {
  return &rhs.Type() == &Sexp::TypeInstance
      && dynamic_cast<const Sexp&>(rhs) == *this;
}

bool Sexp::operator==(const Sexp &rhs) const {
  return ArgListHelper::AreEqual(Args, rhs.Args);
}

bool Sexp::operator!=(const Sexp &rhs) const {
  return !(rhs == *this);
}

void Sexp::Print(std::ostream &out) const {
  out << "(";
  auto argBegin = begin(Args);
  auto argEnd = end(Args);
  auto argCurr = argBegin;
  while (argCurr != argEnd) {
    if (argCurr != argBegin)
      out << " ";
    out << **argCurr;
    ++argCurr;
  }
  out << ")";
}
