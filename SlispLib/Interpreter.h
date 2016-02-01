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
#include "InterpreterUtils.h"

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

class EvaluationContext {
  public:
    Interpreter   &Interp;
    Symbol        CurrentFunction;
    ExpressionPtr &Expr;
    ArgList       &Args;

    explicit EvaluationContext(Interpreter &interpreter, Symbol &currentFunction, ExpressionPtr &expr, ArgList &args);
    bool Evaluate(ExpressionPtr &expr, int argNum);
    bool Evaluate(ExpressionPtr &expr, const std::string &argName);
    bool EvaluateNoError(ExpressionPtr &expr);

    bool Error(const std::string &what);
    bool EvaluateError(int argNum);
    bool EvaluateError(const std::string &argName);
    bool UnknownSymbolError(const std::string &symName);
    bool TypeError(const TypeInfo &expected, const ExpressionPtr &actual);
    bool TypeError(const std::string &expectedName, const ExpressionPtr &actual);
    bool ArgumentExpectedError();
};

class Interpreter {
  public:    
    using SymbolFunctor = std::function<void(const std::string&, ExpressionPtr&)>;

    explicit Interpreter(CommandInterface &commandInterface);
    
    bool Evaluate(ExpressionPtr &expr);
    bool EvaluatePartial(ExpressionPtr &expr);

    InterpreterSettings& GetSettings();

    bool PushError(const EvalError &error);
    std::list<EvalError> GetErrors() const;
    void ClearErrors();

    void Stop();
    bool StopRequested() const;

    SymbolTable& GetDynamicSymbols();

    StackFrame& GetCurrentStackFrame();
    void PushStackFrame(StackFrame &stackFrame);
    void PopStackFrame();

    CommandInterface& GetCommandInterface();

  private:
    using TypeReducer      = std::function<bool(ExpressionPtr &expr)>;
    using TypeReducersType = std::map<const TypeInfo*, TypeReducer>;
    
    CommandInterface        &CmdInterface;
    SymbolTable             DynamicSymbols;
    InterpreterSettings     Settings;
    std::stack<StackFrame*> StackFrames;
    CompiledFunction        MainFunc;
    StackFrame              MainFrame;
    TypeReducersType        TypeReducers;
    std::list<EvalError>    Errors;
    std::string             ErrorWhere;
    Sexp                    *Current;
    bool                    StopRequested_;

    template<class T>          bool InterpretLiteral(T *expr, char *wrapper = nullptr);
    template<class S, class V> bool GetLiteral(const std::string &symbolName, V &value);

    bool GetSpecialFunction(const std::string &name, FunctionPtr &func);
    bool GetCurrFrameSymbol(const std::string &symbolName, ExpressionPtr &value);
    void RegisterReducers();
    bool ReduceBool(ExpressionPtr &expr);
    bool ReduceInt(ExpressionPtr &expr);
    bool ReduceFloat(ExpressionPtr &expr);
    bool ReduceString(ExpressionPtr &expr);
    bool ReduceSymbol(ExpressionPtr &expr);
    bool ReduceFunction(ExpressionPtr &expr);
    bool ReduceSexp(ExpressionPtr &expr);
    bool ReduceSexpFunction(ExpressionPtr &expr, Function &function);
    bool ReduceSexpCompiledFunction(ExpressionPtr &expr, CompiledFunction &function, ArgList &args);
    bool ReduceSexpInterpretedFunction(ExpressionPtr &expr, InterpretedFunction &function, ArgList &args);
    bool ReduceSexpList(ExpressionPtr &expr, ArgList &args);
    bool ReduceQuote(ExpressionPtr &expr);

    bool EvaluateArgs(ArgList &args);
    bool BuildListSexp(Sexp &wrappedSexp, ArgList &args);
};

