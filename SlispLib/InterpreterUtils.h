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
  SourceContext SourceContext_;

  explicit EvalError(const std::string &where, const std::string &what);
  explicit EvalError(const SourceContext &sourceContext, const std::string &where, const std::string &what);
};

class SymbolTable {
  public:
    explicit SymbolTable(SymbolTableType& symbols, const SourceContext &sourceContext);
    void PutSymbol(const std::string &symbolName, ExpressionPtr &value);
    void PutSymbol(const std::string &symbolName, ExpressionPtr &&value);
    void PutSymbolBool(const std::string &symbolName, bool value);
    void PutSymbolInt(const std::string &symbolName, int64_t value);
    void PutSymbolFloat(const std::string &symbolName, double value);
    void PutSymbolStr(const std::string &symbolName, const std::string &value);
    void PutSymbolFunction(const std::string &symbolName, Function &&func);
    void PutSymbolFunction(const std::string &symbolName, std::initializer_list<std::string> signatures, const std::string &doc, std::initializer_list<ExampleDef> examples, SlipFunction fn, FuncDef &&def);
    void PutSymbolQuote(const std::string &symbolName, ExpressionPtr &&value);
    bool GetSymbol(const std::string &symbolName, ExpressionPtr &valueCopy);
    bool GetSymbol(const std::string &symbolName, Expression *&value);
    //bool GetSymbolRef(const std::string &symbolName, ExpressionPtr &ref);
    void DeleteSymbol(const std::string &symbolName);
    void ForEach(std::function<void(const std::string &, ExpressionPtr &)>);
    size_t GetCount() const;

  private:
    SymbolTableType& Symbols;
    SourceContext SourceContext_;
};

class Scope {
  public:
    explicit Scope(SymbolTable &symbols, const SourceContext &sourceContext);
    ~Scope();
    void PutSymbol(const std::string &symbolName, ExpressionPtr &value);
    void PutSymbol(const std::string &symbolName, ExpressionPtr &&value);
    bool IsScopedSymbol(const std::string &symbolName) const;

  private:
    SymbolTable                  &Symbols;
    SymbolTableType              ShadowedSymbolStore;
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
  const std::string ToString() const;
};

struct Environment {
  const SlispVersion Version;
  std::string Program;
  std::string Script;
  std::vector<std::string> Args;
  explicit Environment();
};
