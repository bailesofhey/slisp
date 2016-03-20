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

void SymbolTable::PutSymbolStr(const std::string &symbolName, const std::string &value) {
  PutSymbol(symbolName, ExpressionPtr { new Str { value } });
}

void SymbolTable::PutSymbolInt(const std::string &symbolName, int64_t value) {
  PutSymbol(symbolName, ExpressionPtr { new Int { value } });
}

void SymbolTable::PutSymbolFloat(const std::string &symbolName, double value) {
  PutSymbol(symbolName, ExpressionPtr { new Float { value } });
}

void SymbolTable::PutSymbolQuote(const std::string &symbolName, ExpressionPtr &&value) {
  PutSymbol(symbolName, ExpressionPtr { new Quote { std::move(value) } });
}

void SymbolTable::PutSymbolFunction(const std::string &symbolName, Function &&func) {
  func.Symbol = ExpressionPtr { new Symbol(symbolName) };
  PutSymbol(symbolName, func.Clone());
}

void SymbolTable::PutSymbolFunction(const std::string &symbolName, SlipFunction fn, FuncDef &&def) {
  ExpressionPtr funcExpr { new CompiledFunction { std::move(def), fn } };
  static_cast<CompiledFunction*>(funcExpr.get())->Symbol = ExpressionPtr { new Symbol(symbolName) };
  PutSymbol(symbolName, std::move(funcExpr));
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
  auto beg = begin(InfixSymbolNames);
  auto en = end(InfixSymbolNames);
  auto target = std::find(beg, en, symbolName);
  if (target != en)
    InfixSymbolNames.erase(target);
}

int InterpreterSettings::GetInfixSymbolPrecedence(const std::string &symbolName) const {
  for (int i = 0; i < InfixSymbolNames.size(); ++i) {
    if (InfixSymbolNames[i] == symbolName)
      return i;
  }
  return NO_PRECEDENCE;
}

bool InterpreterSettings::IsSymbolFunction(const std::string &symbolName) const {
  ExpressionPtr value { };
  return DynamicSymbols.GetSymbol(symbolName, value) &&
         TypeHelper::GetValue<Function>(value);
}

bool InterpreterSettings::GetSpecialFunction(const std::string &name, FunctionPtr &func) const {
  ExpressionPtr symbol;
  if (DynamicSymbols.GetSymbol(name, symbol)) {
    if (auto sym = TypeHelper::GetValue<Function>(symbol)) {
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

