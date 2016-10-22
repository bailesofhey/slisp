#include <string>
#include <cinttypes>
#include <memory>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "Expression.h"

using namespace std;

//=============================================================================

TypeInfo::TypeInfo(const string &typeName, const ExpressionNewFn newFn):
  TypeName { typeName },
  NewFn { newFn }
{
}

const string& TypeInfo::Name() const {
  return TypeName;
}

ExpressionPtr TypeInfo::New() const {
  return (*NewFn)();
}

ExpressionPtr TypeInfo::NewUndefined() {
  return ExpressionPtr { };
}

//=============================================================================

Expression::Expression(const TypeInfo& typeInfo):
  Type_ { typeInfo }
{ 
}

Expression::~Expression() {
}

const TypeInfo& Expression::Type() const {
  return Type_;
}

ExpressionPtr Expression::New() const {
  return Type().New();
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

const string Expression::ToString() const {
  stringstream ss;
  ss << *this;
  return ss.str();
}

void Expression::Print(ostream& out) const {
  Display(out);
}

ostream& operator<<(ostream &out, const Expression &expr) {
  expr.Display(out);
  return out;
};

//=============================================================================

IIterator::~IIterator() {
}

//=============================================================================

const TypeInfo Void::TypeInstance { "void", TypeInfo::NewUndefined };

//=============================================================================

const TypeInfo Literal::TypeInstance { "literal", TypeInfo::NewUndefined };

Literal::Literal(const TypeInfo& typeInfo):
  Expression { typeInfo }
{
}

//=============================================================================

const TypeInfo Bool::TypeInstance("bool", Bool::NewInstance);

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

void Bool::Display(ostream &out) const {
  if (Value)
    out << "true";
  else
    out << "false";
}

ExpressionPtr Bool::NewInstance() {
  return ExpressionPtr { new Bool() };
}

//=============================================================================

const TypeInfo Int::TypeInstance("int", Int::NewInstance);

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

void Int::Display(ostream &out) const {
  out << Value;
}

ExpressionPtr Int::NewInstance() {
  return ExpressionPtr { new Int() };
}

//=============================================================================

const TypeInfo Float::TypeInstance("float", Float::NewInstance);

Float::Float():
  Float { 0 }
{
}

Float::Float(double value):
  Literal { TypeInstance },
  Value { value }
{
}

ExpressionPtr Float::Clone() const {
  return ExpressionPtr { new Float(*this) };
}

bool Float::operator==(const Expression &rhs) const {
  return &rhs.Type() == &Float::TypeInstance
      && dynamic_cast<const Float&>(rhs) == *this;
}

bool Float::operator==(const Float &rhs) const {
  return Value == rhs.Value;
}

bool Float::operator!=(const Float &rhs) const {
  return !(*this == rhs);
}

bool Float::operator<(const Float &rhs) const {
  return Value < rhs.Value;
}

bool Float::operator>=(const Float &rhs) const {
  return !(*this < rhs);
}

Float& Float::operator=(Float rhs) {
  Swap(rhs);
  return *this;
}

void Float::Swap(Float &rhs) {
  Value = rhs.Value;
}

void Float::Display(ostream &out) const {
  out << setprecision(17) << Value;
}

ExpressionPtr Float::NewInstance() {
  return ExpressionPtr { new Float() };
}


//=============================================================================

const TypeInfo Str::TypeInstance("str", Str::NewInstance);

Str::Str():
  Str { "" }
{ 
}

Str::Str(const string &value):
  Literal { TypeInstance },
  Value { value }
{
}

ExpressionPtr Str::Clone() const {
  return ExpressionPtr { new Str(*this) };
}

IteratorPtr Str::GetIterator() {
  return IteratorPtr { new StrIterator(*this) };
}

bool Str::operator==(const Expression &rhs) const {
  return &rhs.Type() == &Str::TypeInstance
      && dynamic_cast<const Str&>(rhs) == *this;
}

bool Str::operator==(const Str &rhs) const {
  return Value == rhs.Value;
}

bool Str::operator!=(const Str &rhs) const {
  return !(*this == rhs);
}

bool Str::operator<(const Str &rhs) const {
  return Value < rhs.Value;
}

bool Str::operator>=(const Str &rhs) const {
  return !(*this < rhs);
}

Str& Str::operator=(Str rhs) {
  Swap(rhs);
  return *this;
}

void Str::Swap(Str &rhs) {
  Value = rhs.Value;
}

void Str::Display(ostream &out) const {
  out << "\"" << Value << "\"";
}

void Str::Print(ostream &out) const {
  out << Value;
}

ExpressionPtr Str::NewInstance() {
  return ExpressionPtr { new Str() };
}


//=============================================================================

StrIterator::StrIterator(Str &str):
  Value(str),
  Index(0)
{
}

// TODO: really should be returning a CharRef (see #98)
ExpressionPtr& StrIterator::Next() {
  if (Index < Value.Value.length()) {
    Curr = ExpressionPtr { new Str(string(1, Value.Value[Index++]))};
    return Curr;
  }
  else
    return Null;
}

int64_t StrIterator::GetLength() {
  return Value.Value.length();
}

//=============================================================================

const TypeInfo Quote::TypeInstance("quote", Quote::NewInstance);

Quote::Quote(ExpressionPtr &&expr):
  Literal { TypeInstance },
  Value { move(expr) }
{
}

ExpressionPtr Quote::Clone() const {
  return ExpressionPtr { new Quote(move(Value->Clone())) };
}

IteratorPtr Quote::GetIterator() {
  if (Value) {
    if (auto *sexp = dynamic_cast<Sexp*>(Value.get()))
      return sexp->GetIterator();
  }
  return IteratorPtr { };
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

void Quote::Display(ostream &out) const {
  out << *Value;
}

void Quote::Print(ostream &out) const {
  Value->Print(out);
}

ExpressionPtr Quote::NewInstance() {
  return ExpressionPtr { new Quote(ExpressionPtr {}) };
}

//=============================================================================

const TypeInfo Symbol::TypeInstance("symbol", TypeInfo::NewUndefined);

Symbol::Symbol(const string &value):
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

void Symbol::Display(ostream &out) const {
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
    if (auto ref = dynamic_cast<Ref*>(arg.get()))
      dst.push_back(ref->NewRef());
    else
      dst.push_back(arg->Clone());
  }
}

//=============================================================================

const TypeInfo Sexp::TypeInstance("sexp", Sexp::NewInstance);

Sexp::Sexp():
  Expression { TypeInstance }
{
}

Sexp::Sexp(ArgList &&args):
  Sexp()
{
  Args = move(args);
}

Sexp::Sexp(initializer_list<ExpressionPtr> &&args):
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

IteratorPtr Sexp::GetIterator() {
  return IteratorPtr { new SexpIterator(*this) };
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

void Sexp::Display(ostream &out) const {
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

ExpressionPtr Sexp::NewInstance() {
  return ExpressionPtr { new Sexp() };
}

//=============================================================================

SexpIterator::SexpIterator(Sexp &sexp):
  Curr(sexp.Args.begin()),
  End(sexp.Args.end()),
  Length(sexp.Args.size())
{
}

ExpressionPtr& SexpIterator::Next() {
  if (Curr != End)
    return *(Curr++); 
  else
    return Null; 
}

int64_t SexpIterator::GetLength() {
  return Length;
}

//=============================================================================

const TypeInfo Ref::TypeInstance { "ref", TypeInfo::NewUndefined };

Ref::Ref(ExpressionPtr &value):
  Expression(TypeInstance),
  Value(value)
{
}

ExpressionPtr Ref::Clone() const {
  return Value->Clone();
}

ExpressionPtr Ref::NewRef() const {
  return ExpressionPtr { new Ref(Value) }; 
}

void Ref::Display(ostream& out) const {
}

IteratorPtr Ref::GetIterator() {
  if (auto *iterable = dynamic_cast<IIterable*>(Value.get()))
    return iterable->GetIterator();
  else
    return IteratorPtr {};
}

bool Ref::operator==(const Expression &rhs) const {
  return &rhs.Type() == &Ref::TypeInstance
      && dynamic_cast<const Ref&>(rhs) == *this;
}

bool Ref::operator==(const Ref &rhs) const {
  return Value.get() == rhs.Value.get();
}

bool Ref::operator!=(const Ref &rhs) const {
  return !(rhs == *this);
}

//=============================================================================

ExpressionPtr List::GetNil() {
  return ExpressionPtr { new Quote { ExpressionPtr { new Sexp {} } } };
}
