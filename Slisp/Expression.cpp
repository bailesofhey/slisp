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

bool Bool::operator==(const Bool &rhs) const {
  return Value == rhs.Value;
}

bool Bool::operator<(const Bool &rhs) const {
  return Value < rhs.Value;
}

Bool& Bool::operator=(Bool rhs) {
  Swap(rhs);
  return *this;
}

void Bool::Swap(Bool &rhs) {
  Value = rhs.Value;
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

bool Number::operator==(const Number &rhs) const {
  return Value == rhs.Value;
}

bool Number::operator<(const Number &rhs) const {
  return Value < rhs.Value;
}

Number& Number::operator=(Number rhs) {
  Swap(rhs);
  return *this;
}

void Number::Swap(Number &rhs) {
  Value = rhs.Value;
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

bool String::operator==(const String &rhs) const {
  return Value == rhs.Value;
}

bool String::operator<(const String &rhs) const {
  return Value < rhs.Value;
}

String& String::operator=(String rhs) {
  Swap(rhs);
  return *this;
}

void String::Swap(String &rhs) {
  Value = rhs.Value;
}

//=============================================================================

const TypeInfo Quote::TypeInstance("quote");

Quote::Quote(ExpressionPtr &&expr):
  Literal { TypeInstance },
  Value { std::move(expr) }
{
}

Quote& Quote::operator=(Quote rhs) {
  Swap(rhs);
  return *this;
}

void Quote::Swap(Quote &rhs) {
  Value = std::move(rhs.Value);
}

ExpressionPtr Quote::Clone() const {
  return ExpressionPtr { new Quote(Value->Clone()) };
}

const std::string Quote::ToString() const {
  return "Quote { " + Value->ToString() + " }";
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

bool Symbol::operator==(const Symbol& rhs) const {
  return Value == rhs.Value;
}

bool Symbol::operator<(const Symbol &rhs) const {
  return Value < rhs.Value;
}

Symbol& Symbol::operator=(Symbol rhs) {
  Swap(rhs);
  return *this;
}

void Symbol::Swap(Symbol &rhs) {
  Value = rhs.Value;
}

//=============================================================================

const TypeInfo Sexp::TypeInstance("sexp");

Sexp::Sexp():
  Expression { TypeInstance }
{
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

