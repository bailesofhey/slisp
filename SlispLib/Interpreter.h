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
#include "ExpressionFactory.h"

class Interpreter;

class StackFrame {
  public:
    explicit StackFrame(Interpreter &interp, InterpretedFunction &&func);
    explicit StackFrame(Interpreter &interp, InterpretedFunction &func);
    ~StackFrame();
    void PutSymbol(const std::string &symbolName, ExpressionPtr &value);

    // these needed?
    void PutLocalSymbol(const std::string &symbolName, ExpressionPtr &&value);
    void PutLocalSymbol(const std::string &symbolName, ExpressionPtr &value);
    void PutDynamicSymbol(const std::string &symbolName, ExpressionPtr &value);
    void PutDynamicSymbol(const std::string &symbolName, ExpressionPtr &&value);

    bool GetSymbol(const std::string &symbolName, ExpressionPtr &valueCopy);
    bool GetSymbol(const std::string &symbolName, Expression *&value);
    void DeleteSymbol(const std::string &symbolName);
    SymbolTable& GetLocalSymbols();
    InterpretedFunction& GetFunction();

  private:
    Interpreter     &Interp;
    InterpretedFunction  &Func;
    SymbolTable     Dynamics;
    SymbolTableType LocalStore;
    SymbolTable     Locals;
    SymbolTable     Closure;
    Scope           DynamicScope;
};

class EvaluationContext {
  public:
    Interpreter       &Interp;
    Symbol            CurrentFunction;
    ExpressionPtr     &Expr_;
    ArgList           &Args;
    SourceContext     SourceContext_;
    ExpressionFactory Factory;

    explicit EvaluationContext(Interpreter &interpreter, CompiledFunction &compiledFunction, Symbol &currentFunction, ExpressionPtr &expr, ArgList &args);

    const SourceContext& GetSourceContext() const;

    bool Evaluate(ExpressionPtr &expr, int argNum);
    bool Evaluate(ExpressionPtr &expr, const std::string &argName);
    bool EvaluateNoError(ExpressionPtr &expr);

    template<typename T, typename... Args>
    T* Alloc(Args&&... args) {
      return Factory.Alloc<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    ExpressionContainer<T> New(Args&&... args) {
      if (auto expr = Factory.New<T>(std::forward<Args>(args)...))
        return expr;
      else {
        AllocationError();
        return expr;
      }
    }

    template<typename T, typename... Args>
    bool ReturnNew(Args&&... args) {
      Expr_.reset(Alloc<T>(std::forward<Args>(args)...));
      if (Expr_)
        return true;
      else {
        AllocationError();
        return false;
      }
    }

    bool Return(ExpressionPtr &expr) {
      Expr_.swap(expr);
      if (Expr_)
        return true;
      else
        return false;
    }

    bool Return(Expression *expr) {
      Expr_.reset(expr);
      if (Expr_)
        return true;
      else
        return false;
    }

    bool ReturnNil() {
      ExpressionPtr nil = List::GetNil(SourceContext_);
      if (nil)
        return Return(nil);
      else {
        AllocationError();
        return false;
      }
    }

    template<class T>
    T* GetRequiredValue(ExpressionPtr &expr) {
      if (auto value = TypeHelper::GetValue<T>(expr)) 
        return value;
      TypeError<T>(expr);
      return nullptr;
    }

    template<class T>
    T* GetRequiredValue(ExpressionPtr &expr, const std::string &expectedType) {
      if (auto value = TypeHelper::GetValue<T>(expr)) 
        return value;
      TypeError(expectedType, expr);
      return nullptr;
    }

    bool GetSymbol(const std::string &symName, ExpressionPtr &valueCopy);
    bool GetSymbol(const std::string &symName, Expression *&value);

    bool IsQuoteAList(Quote &quote);
    Sexp* GetList(ExpressionPtr &expr);
    Sexp* GetRequiredListValue(ExpressionPtr &expr);
    const std::string GetThisFunctionName();

    bool Error(const std::string &what);
    bool EvaluateError(int argNum);
    bool EvaluateError(const std::string &argName);
    bool UnknownSymbolError(const std::string &symName);

    template<class T>
    bool TypeError(const ExpressionPtr &actual) {
      return TypeError(T::TypeInstance, actual);
    }

    bool TypeError(const TypeInfo &expected, const ExpressionPtr &actual);
    bool TypeError(const std::string &expectedName, const ExpressionPtr &actual);
    bool ArgumentExpectedError();
    bool AllocationError();
};

class Interpreter {
  public:    
    using SymbolFunctor = std::function<void(const std::string&, ExpressionPtr&)>;

    explicit Interpreter(CommandInterface &commandInterface);
    ~Interpreter();
    
    bool Evaluate(ExpressionPtr &&expr);
    bool Evaluate(ExpressionPtr &expr);
    bool EvaluatePartial(ExpressionPtr &expr);

    InterpreterSettings& GetSettings();

    bool PushError(const EvalError &error);
    std::list<EvalError> GetErrors() const;
    void ClearErrors();
    std::vector<std::string> GetErrorStackTrace() const;

    void Stop();
    bool StopRequested() const;

    int GetExitCode() const;
    void SetExitCode(int exitCode);

    SymbolTable GetDynamicSymbols(const SourceContext &sourceContext);

    StackFrame& GetCurrentStackFrame();
    void PushStackFrame(StackFrame &stackFrame);
    void PopStackFrame();

    CommandInterface& GetCommandInterface();

    Environment& GetEnvironment();

    ModuleInfo* CreateModule(const std::string &moduleName, const std::string &filePath);

  private:
    using TypeReducer      = std::function<bool(ExpressionPtr &expr)>;
    using TypeReducersType = std::map<const TypeInfo*, TypeReducer>;
    
    CommandInterface                   &CmdInterface;
    std::map<std::string, ModuleInfo*> Modules;  
    SourceContext                      SourceContext_;
    SymbolTableType                    DynamicSymbolStore;
    SymbolTable                        DynamicSymbols;
    InterpreterSettings                Settings;
    std::vector<StackFrame*>           StackFrames;
    InterpretedFunction                MainFunc;
    StackFrame                         MainFrame;
    TypeReducersType                   TypeReducers;
    std::list<EvalError>               Errors;
    std::string                        ErrorWhere;
    std::vector<std::string>           ErrorStackTrace;
    Sexp                               *Current;
    bool                               StopRequested_;
    int                                ExitCode;
    Environment                        Environment_;

    template<class T>          bool InterpretLiteral(T *expr, char *wrapper = nullptr);
    template<class S, class V> bool GetLiteral(const std::string &symbolName, V &value);

    bool GetSpecialFunction(const std::string &name, FunctionPtr &func);
    bool GetCurrFrameSymbol(const std::string &symbolName, ExpressionPtr &value);
    void RegisterReducers();
    bool ReduceBool(ExpressionPtr &expr);
    bool ReduceInt(ExpressionPtr &expr);
    bool ReduceFloat(ExpressionPtr &expr);
    bool ReduceStr(ExpressionPtr &expr);
    bool ReduceSymbol(ExpressionPtr &expr);
    bool ReduceFunction(ExpressionPtr &expr);
    bool ReduceSexp(ExpressionPtr &expr);
    bool ReduceSexpFunction(ExpressionPtr &expr, Function &function);
    bool ReduceSexpCompiledFunction(ExpressionPtr &expr, CompiledFunction &function, ArgList &args);
    bool ReduceSexpInterpretedFunction(ExpressionPtr &expr, InterpretedFunction &function, ArgList &args);
    bool ReduceSexpList(ExpressionPtr &expr, ArgList &args);
    bool ReduceQuote(ExpressionPtr &expr);
    bool ReduceRef(ExpressionPtr &expr);

    bool EvaluateArgs(ArgList &args);
    bool BuildListSexp(Sexp &wrappedSexp, ArgList &args);
};

