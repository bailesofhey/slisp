#include <string>
#include <cinttypes>
#include <memory>

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

const std::string Bool::ToString() const {
  return "Bool { " + std::to_string(Value) + " }";
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

std::ostream& operator<<(std::ostream& os, const Bool& value) {
  return os << value.ToString();
}
//=============================================================================

const TypeInfo Number::TypeInstance("number");

Number::Number():
  Number { 0 }
{
}

Number::Number(int64_t value):
  Literal { TypeInstance },
  Value { value }
{
}

ExpressionPtr Number::Clone() const {
  return ExpressionPtr { new Number(*this) };
}

const std::string Number::ToString() const {
  return "Number { " + std::to_string(Value) + " }";
}

bool Number::operator==(const Expression &rhs) const {
  return &rhs.Type() == &Number::TypeInstance
      && dynamic_cast<const Number&>(rhs) == *this;
}

bool Number::operator==(const Number &rhs) const {
  return Value == rhs.Value;
}

bool Number::operator!=(const Number &rhs) const {
  return !(*this == rhs);
}

bool Number::operator<(const Number &rhs) const {
  return Value < rhs.Value;
}

bool Number::operator>=(const Number &rhs) const {
  return !(*this < rhs);
}

Number& Number::operator=(Number rhs) {
  Swap(rhs);
  return *this;
}

void Number::Swap(Number &rhs) {
  Value = rhs.Value;
}

std::ostream& operator<<(std::ostream& os, const Number& value) {
  return os << value.ToString();
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

const std::string String::ToString() const {
  return "String { \"" + Value + "\" }";
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

std::ostream& operator<<(std::ostream& os, const String& value) {
  return os << value.ToString();
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

const std::string Quote::ToString() const {
  return "Quote { " + Value->ToString() + " }";
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

std::ostream& operator<<(std::ostream& os, const Quote& value) {
  return os << value.ToString();
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

const std::string Symbol::ToString() const {
  return "Symbol { " + Value + " }";
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

std::ostream& operator<<(std::ostream& os, const Symbol& value) {
  return os << value.ToString();
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

const std::string Sexp::ToString() const {
  return "Sexp { }";
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

std::ostream& operator<<(std::ostream& os, const Sexp& value) {
  return os << value.ToString();
}