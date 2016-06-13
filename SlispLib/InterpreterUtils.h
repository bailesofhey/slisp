#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>

#include "Expression.h"
#include "FunctionDef.h"

struct EvalError {
  std::string Where; //TODO: Function
  std::string What;

  explicit EvalError(const std::string &where, const std::string &what);
};

class SymbolTable {
  public:
    void PutSymbol(const std::string &symbolName, ExpressionPtr &value);
    void PutSymbolBool(const std::string &symbolName, bool value);
    void PutSymbolInt(const std::string &symbolName, int64_t value);
    void PutSymbolFloat(const std::string &symbolName, double value);
    void PutSymbolStr(const std::string &symbolName, const std::string &value);
    void PutSymbolFunction(const std::string &symbolName, Function &&func);
    void PutSymbolFunction(const std::string &symbolName, SlipFunction fn, FuncDef &&def);
    void PutSymbolQuote(const std::string &symbolName, ExpressionPtr &&value);
    bool GetSymbol(const std::string &symbolName, ExpressionPtr &valueCopy);
    bool GetSymbol(const std::string &symbolName, Expression *&value);
    void DeleteSymbol(const std::string &symbolName);
    void ForEach(std::function<void(const std::string &, ExpressionPtr &)>);
    size_t GetCount() const;

  private:
    SymbolTableType Symbols;
};

class Scope {
  public:
    explicit Scope(SymbolTable &symbols);
    ~Scope();
    void PutSymbol(const std::string &symbolName, ExpressionPtr &value);
    bool IsScopedSymbol(const std::string &symbolName) const;

  private:
    SymbolTable                  &Symbols;
    SymbolTable                  ShadowedSymbols;
    std::vector<std::string>     ScopedSymbols;
};

class InterpreterSettings {
  public:
    static const int NO_PRECEDENCE = -1;

    explicit InterpreterSettings(SymbolTable &dynamicSymbols);

    const std::string GetDefaultSexp() const;
    const std::string GetListSexp() const;

    bool GetDefaultFunction(FunctionPtr &func) const;
    bool GetListFunction(FunctionPtr &func) const;

    void PutDefaultFunction(Function &&func);
    void PutListFunction(Function &&func);

    void RegisterInfixSymbol(const std::string &symbolName);
    void UnregisterInfixSymbol(const std::string &symbolName);
    int GetInfixSymbolPrecedence(const std::string &symbolName) const;

    bool IsSymbolFunction(const std::string &symbolName) const;

  private:
    SymbolTable& DynamicSymbols;
    std::vector<std::string> InfixSymbolNames;
    std::string DefaultSexp;
    std::string ListSexp;

    bool GetSpecialFunction(const std::string &name, FunctionPtr &func) const;
};

struct SlispVersion {
  const int Major;
  const int Minor;
  const int SubMinor;
  const int Build;
  explicit SlispVersion(const int major, const int minor, const int subMinor, const int build);
};

class Environment {
public:
  explicit Environment();
  void SetArgs(const std::vector<std::string> &processArgs, const std::vector<std::string> &slispArgs);
  const SlispVersion& Version() const;
  const std::vector<std::string>& ProcessArgs() const;
  const std::vector<std::string>& SlispArgs() const;
private:
  const SlispVersion Version_;
  std::vector<std::string> ProcessArgs_;
  std::vector<std::string> SlispArgs_; 
};