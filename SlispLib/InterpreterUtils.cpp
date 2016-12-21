#include <algorithm>
#include <vector>
#include <sstream>

#include "InterpreterUtils.h"

using namespace std;

//=============================================================================

EvalError::EvalError(const string &where, const string &what):
  Where { where },
  What { what }
{
}

//=============================================================================
SymbolTable::SymbolTable(SymbolTableType& symbols, const SourceContext &sourceContext):
  Symbols(symbols),
  SourceContext_(sourceContext)
{
}

void SymbolTable::PutSymbol(const string &symbolName, ExpressionPtr &value) {
  PutSymbol(symbolName, move(value));
}

void SymbolTable::PutSymbol(const string &symbolName, ExpressionPtr &&value) {
  auto search = Symbols.find(symbolName);
  if (search != Symbols.end()) {
    if (auto *ref = dynamic_cast<Ref*>(search->second.get())) {
      if (auto *refValue = dynamic_cast<Ref*>(value.get()))
        ref->Value = refValue->Clone();
      else
        ref->Value = move(value);
    }
    else
      search->second = move(value);
  }
  else
    Symbols.emplace(symbolName, move(value));
}

void SymbolTable::PutSymbolBool(const string &symbolName, bool value) {
  PutSymbol(symbolName, ExpressionPtr { new Bool { SourceContext_, value } });
}

void SymbolTable::PutSymbolStr(const string &symbolName, const string &value) {
  PutSymbol(symbolName, ExpressionPtr { new Str { SourceContext_, value } });
}

void SymbolTable::PutSymbolInt(const string &symbolName, int64_t value) {
  PutSymbol(symbolName, ExpressionPtr { new Int { SourceContext_, value } });
}

void SymbolTable::PutSymbolFloat(const string &symbolName, double value) {
  PutSymbol(symbolName, ExpressionPtr { new Float { SourceContext_, value } });
}

void SymbolTable::PutSymbolQuote(const string &symbolName, ExpressionPtr &&value) {
  PutSymbol(symbolName, ExpressionPtr { new Quote { SourceContext_, move(value) } });
}

void SymbolTable::PutSymbolFunction(const string &symbolName, Function &&func) {
  func.Symbol = ExpressionPtr { new Symbol(SourceContext_, symbolName) };
  PutSymbol(symbolName, func.Clone());
}

void SymbolTable::PutSymbolFunction(const string &symbolName, initializer_list<string> signatures, const string &doc, initializer_list<ExampleDef> examples, SlipFunction fn, FuncDef &&def) {
  ExpressionPtr funcExpr { new CompiledFunction { SourceContext_, move(def), fn } };
  if (funcExpr) {
    auto *func = static_cast<CompiledFunction*>(funcExpr.get());
    func->Symbol = ExpressionPtr { new Symbol(SourceContext_, symbolName) };
    func->Signatures = signatures;
    func->Doc = doc;
    func->Examples = examples;
    PutSymbol(symbolName, move(funcExpr));
  }
}

bool SymbolTable::GetSymbol(const string &symbolName, ExpressionPtr &valueCopy) {
  auto it = Symbols.find(symbolName);
  if (it != Symbols.end()) {
    if (it->second)
      valueCopy = ExpressionPtr { it->second->Clone() };
    else
      valueCopy = ExpressionPtr { };
    return true;
  }
  else
    return false;
}

bool SymbolTable::GetSymbol(const string &symbolName, Expression *&value) {
  auto it = Symbols.find(symbolName);
  if (it != Symbols.end()) {
    if (it->second)
      value = it->second.get();
    else
      value = nullptr;
    return true;
  }
  else
    return false;

}

void SymbolTable::DeleteSymbol(const string &symbolName) {
  Symbols.erase(symbolName);
}

void SymbolTable::ForEach(function<void(const string &, ExpressionPtr &)> fn) {
  for (auto &sym : Symbols)
    fn(sym.first, sym.second);
}

size_t SymbolTable::GetCount() const {
  return Symbols.size();
}

//=============================================================================

Scope::Scope(SymbolTable &symbols, const SourceContext &sourceContext):
  Symbols { symbols },
  ShadowedSymbolStore {},
  ShadowedSymbols { ShadowedSymbolStore, sourceContext },
  ScopedSymbols {}
{
}

Scope::~Scope() {
  for (auto &scopedSymbol : ScopedSymbols) {
    Symbols.DeleteSymbol(scopedSymbol);
  }

  ShadowedSymbols.ForEach([this](const string &symbolName, ExpressionPtr &value) {
    Symbols.PutSymbol(symbolName, move(value));
  });
}

void Scope::PutSymbol(const string &symbolName, ExpressionPtr &&value) {
  ExpressionPtr oldValue;
  bool isScoped = IsScopedSymbol(symbolName);
  if (!isScoped) {
    if (Symbols.GetSymbol(symbolName, oldValue))
      ShadowedSymbols.PutSymbol(symbolName, move(oldValue));
    ScopedSymbols.push_back(symbolName);
  }
  Symbols.PutSymbol(symbolName, move(value));
}

void Scope::PutSymbol(const string &symbolName, ExpressionPtr &value) {
  PutSymbol(symbolName, move(value));
}

bool Scope::IsScopedSymbol(const string &symbolName) const {
  auto scopeBegin = begin(ScopedSymbols),
       scopeEnd   = end(ScopedSymbols);
  return find(scopeBegin, scopeEnd, symbolName) != scopeEnd;
}

//=============================================================================

InterpreterSettings::InterpreterSettings(SymbolTable &dynamicSymbols):
  DynamicSymbols { dynamicSymbols },
  InfixSymbolNames { },
  DefaultSexp { "__default_sexp__" },
  ListSexp { "__list__sexp__" }
{
}

const string InterpreterSettings::GetDefaultSexp() const {
  return DefaultSexp;
}

const string InterpreterSettings::GetListSexp() const {
  return ListSexp;
}

bool InterpreterSettings::GetDefaultFunction(FunctionPtr &func) const {
  return GetSpecialFunction(DefaultSexp, func);
}

bool InterpreterSettings::GetListFunction(FunctionPtr &func) const {
  return GetSpecialFunction(ListSexp, func);
}

void InterpreterSettings::PutDefaultFunction(Function &&func) {
  DynamicSymbols.PutSymbolFunction(DefaultSexp, move(func));
}

void InterpreterSettings::PutListFunction(Function &&func) {
  DynamicSymbols.PutSymbolFunction(ListSexp, move(func));
}

void InterpreterSettings::RegisterInfixSymbol(const string &symbolName) {
  InfixSymbolNames.push_back(symbolName);
}

void InterpreterSettings::UnregisterInfixSymbol(const string &symbolName) {
  auto beg = begin(InfixSymbolNames);
  auto en = end(InfixSymbolNames);
  auto target = find(beg, en, symbolName);
  if (target != en)
    InfixSymbolNames.erase(target);
}

int InterpreterSettings::GetInfixSymbolPrecedence(const string &symbolName) const {
  for (size_t i = 0; i < InfixSymbolNames.size(); ++i) {
    if (InfixSymbolNames[i] == symbolName)
      return i;
  }
  return NO_PRECEDENCE;
}

bool InterpreterSettings::IsSymbolFunction(const string &symbolName) const {
  ExpressionPtr value { };
  return DynamicSymbols.GetSymbol(symbolName, value) &&
         TypeHelper::GetValue<Function>(value);
}

bool InterpreterSettings::GetSpecialFunction(const string &name, FunctionPtr &func) const {
  ExpressionPtr symbol;
  if (DynamicSymbols.GetSymbol(name, symbol)) {
    if (auto sym = TypeHelper::GetValue<Function>(symbol)) {
      symbol.release();
      func.reset(sym);
      return true;
    }
    else
      throw runtime_error("Special function not a function");
  }
  else
    return false;
}

//=============================================================================

SlispVersion::SlispVersion(const int major, const int minor, const int subMinor, const int build):
  Major(major),
  Minor(minor),
  SubMinor(subMinor),
  Build(build)
{
}

const std::string SlispVersion::ToString() const {
  stringstream verDisp;
  verDisp << "Slisp " << Major << "." << Minor << "." << SubMinor << "." << Build;
  return verDisp.str();
}

//=============================================================================

Environment::Environment():
  Version(0, 2, 0, 1)
{
}
