#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <stack>

#include "Expression.h"
#include "FunctionDef.h"
#include "CommandInterface.h"

struct EvalError {
  std::string Where; //TODO: Function
  std::string What;

  explicit EvalError(const std::string &where, const std::string &what);
};

class SymbolTable {
  public:
    void PutSymbol(const std::string &symbolName, ExpressionPtr &value);
    void PutSymbolBool(const std::string &symbolName, bool value);
    void PutSymbolNumber(const std::string &symbolName, int64_t value);
    void PutSymbolString(const std::string &symbolName, const std::string &value);
    void PutSymbolFunction(const std::string &symbolName, Function &&func);
    void PutSymbolFunction(const std::string &symbolName, SlipFunction fn, FuncDef &&def);
    void PutSymbolQuote(const std::string &symbolName, ExpressionPtr &&value);
    bool GetSymbol(const std::string &symbolName, ExpressionPtr &value);
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

  private:
    SymbolTable                  &Symbols;
    SymbolTable                  ShadowedSymbols;
    std::vector<std::string>     ScopedSymbols;
};

class Interpreter;

class StackFrame {
  public:
    explicit StackFrame(Interpreter &interp, Function &func);
    ~StackFrame();
    void PutLocalSymbol(const std::string &symbolName, ExpressionPtr &value);
    void PutDynamicSymbol(const std::string &symbolName, ExpressionPtr &value);
    bool GetSymbol(const std::string &symbolName, ExpressionPtr &value);
    void DeleteSymbol(const std::string &symbolName);
    SymbolTable& GetLocalSymbols();

  private:
    Interpreter &Interp;
    Function    &Func;
    SymbolTable &Dynamics;
    SymbolTable Locals;
    Scope       DynamicScope;
};

class Interpreter {
  public:    
    using SymbolFunctor = std::function<void(const std::string&, ExpressionPtr&)>;

    explicit Interpreter(CommandInterface &commandInterface);
    
    bool Evaluate(ExpressionPtr &expr);
    bool EvaluatePartial(ExpressionPtr &expr);

    bool PushError(const EvalError &error);
    std::list<EvalError> GetErrors() const;
    void ClearErrors();

    void Stop();
    bool StopRequested() const;

    SymbolTable& GetDynamicSymbols();

    void PutDefaultFunction(Function &&func);
    bool GetDefaultFunction(FunctionPtr &func);
    const std::string GetDefaultSexp() const;

    StackFrame& GetCurrentStackFrame();
    void PushStackFrame(StackFrame &stackFrame);
    void PopStackFrame();

    CommandInterface& GetCommandInterface();

  private:
    using TypeReducer      = std::function<bool(ExpressionPtr &expr)>;
    using TypeReducersType = std::map<const TypeInfo*, TypeReducer>;
    
    CommandInterface        &CmdInterface;
    SymbolTable             DynamicSymbols;
    std::stack<StackFrame*> StackFrames;
    CompiledFunction        MainFunc;
    StackFrame              MainFrame;
    TypeReducersType        TypeReducers;
    std::list<EvalError>    Errors;
    std::string             DefaultSexp;
    std::string             ErrorWhere;
    Sexp                    *Current;
    bool                    StopRequested_;

    template<class T>          bool InterpretLiteral(T *expr, char *wrapper = nullptr);
    template<class S, class V> bool GetLiteral(const std::string &symbolName, V &value);

    bool GetCurrFrameSymbol(const std::string &symbolName, ExpressionPtr &value);
    void RegisterReducers();
    bool ReduceBool(ExpressionPtr &expr);
    bool ReduceNumber(ExpressionPtr &expr);
    bool ReduceString(ExpressionPtr &expr);
    bool ReduceSymbol(ExpressionPtr &expr);
    bool ReduceFunction(ExpressionPtr &expr);
    bool ReduceSexp(ExpressionPtr &expr);
    bool ReduceSexpFunction(ExpressionPtr &expr, Function &function);
    bool ReduceSexpCompiledFunction(ExpressionPtr &expr, CompiledFunction &function, ArgList &args);
    bool ReduceSexpInterpretedFunction(ExpressionPtr &expr, InterpretedFunction &function, ArgList &args);
    bool ReduceSexpList(ExpressionPtr &expr, ArgList &args);
    bool ReduceQuote(ExpressionPtr &expr);
};
