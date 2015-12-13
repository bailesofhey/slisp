#include <algorithm>
#include <vector>

#include "InterpreterUtils.h"

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
  if (!IsScopedSymbol(symbolName)) {
    if (Symbols.GetSymbol(symbolName, oldValue)) {
      ShadowedSymbols.PutSymbol(symbolName, std::move(oldValue));
    }
  }
  Symbols.PutSymbol(symbolName, std::move(value));
  ScopedSymbols.push_back(symbolName);
}

bool Scope::IsScopedSymbol(const std::string &symbolName) const {
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

const std::string InterpreterSettings::GetDefaultSexp() const {
  return DefaultSexp;
}

const std::string InterpreterSettings::GetListSexp() const {
  return ListSexp;
}

bool InterpreterSettings::GetDefaultFunction(FunctionPtr &func) const {
  return GetSpecialFunction(DefaultSexp, func);
}

bool InterpreterSettings::GetListFunction(FunctionPtr &func) const {
  return GetSpecialFunction(ListSexp, func);
}

void InterpreterSettings::PutDefaultFunction(Function &&func) {
  DynamicSymbols.PutSymbolFunction(DefaultSexp, std::move(func));
}

void InterpreterSettings::PutListFunction(Function &&func) {
  DynamicSymbols.PutSymbolFunction(ListSexp, std::move(func));
}

void InterpreterSettings::RegisterInfixSymbol(const std::string &symbolName) {
  InfixSymbolNames.push_back(symbolName);
}

void InterpreterSettings::UnregisterInfixSymbol(const std::string &symbolName) {
  std::remove(begin(InfixSymbolNames), end(InfixSymbolNames), symbolName);
}

bool InterpreterSettings::IsInfixSymbol(const std::string &symbolName) const {
  auto endName = end(InfixSymbolNames);
  return std::find(begin(InfixSymbolNames), endName, symbolName) != endName;
}

bool InterpreterSettings::IsSymbolFunction(const std::string &symbolName) const {
  ExpressionPtr value { };
  return DynamicSymbols.GetSymbol(symbolName, value) &&
         dynamic_cast<Function*>(value.get());
}

bool InterpreterSettings::GetSpecialFunction(const std::string &name, FunctionPtr &func) const {
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

//=============================================================================
