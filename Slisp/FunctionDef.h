#pragma once

#include <string>
#include <initializer_list>
#include <vector>
#include <memory>
#include <functional>
#include <list>

#include "Expression.h"

using ExpressionEvaluator = std::function<bool(ExpressionPtr&)>;

class ArgDef {
  public:
    virtual ~ArgDef();
    virtual std::unique_ptr<ArgDef> Clone() const = 0;
    virtual const std::string ToString() const = 0;
    bool Validate(ExpressionEvaluator evaluator, ExpressionPtr &expr, std::string &error) const;
    static bool TypeMatches(const TypeInfo &expected, const TypeInfo& actual);
    static bool IsLiteral(const TypeInfo &type);
    static bool IsFunction(const TypeInfo &type);

  protected:
    virtual bool ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const = 0;
    bool CheckArgCount(int expected, ArgList &args, std::string &error) const;
    bool CheckArg(ExpressionEvaluator evaluator, ExpressionPtr &arg, const TypeInfo &expectedType, int argNum, std::string &error) const;
};

class VarArgDef: public ArgDef {
  public:
    static const int NO_ARGS = 0;
    static const int ANY_ARGS = -1;

    VarArgDef(const TypeInfo& type, int nargs);
    virtual std::unique_ptr<ArgDef> Clone() const;
    virtual const std::string ToString() const;

  private:
    const TypeInfo& Type;
    int NArgs;

    virtual bool ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const;

    bool ValidateArgCount(ArgList &args, std::string &error) const;
    bool ValidateArgTypes(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const;
};

class ListArgDef: public ArgDef {
  public:
    ListArgDef(std::initializer_list<const TypeInfo*> types);
    virtual std::unique_ptr<ArgDef> Clone() const;
    virtual const std::string ToString() const;

  private:
    std::vector<const TypeInfo*> Types;

    virtual bool ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const;
};

class Interpreter;

using SlipFunction = std::function<bool(Interpreter &, ExpressionPtr&, ArgList&)>;

class FuncDef {
  public:
    static std::unique_ptr<ArgDef> NoArgs();
    static std::unique_ptr<ArgDef> OneArg(const TypeInfo& type);
    static std::unique_ptr<ArgDef> AnyArgs(const TypeInfo& type);
    static std::unique_ptr<ArgDef> AnyArgs();
    static std::unique_ptr<ArgDef> ManyArgs(const TypeInfo& type, int nargs);
    static std::unique_ptr<ArgDef> Args(std::initializer_list<const TypeInfo*> args);

    FuncDef(std::unique_ptr<ArgDef> &in, std::unique_ptr<ArgDef> &out);
    FuncDef(const FuncDef &val);
    FuncDef(FuncDef &&rval);
    FuncDef& operator=(FuncDef);
    void Swap(FuncDef &func);
    const std::string ToString() const;
    bool ValidateArgs(ExpressionEvaluator evaluator, ExpressionPtr &expr, std::string &error);

  private:
    std::unique_ptr<ArgDef> In;
    std::unique_ptr<ArgDef> Out;
};

struct Function: public Literal {
  static const TypeInfo TypeInstance;

  FuncDef Def;

  Function();
  Function(FuncDef &&func);
};

struct CompiledFunction: public Function {
  static const TypeInfo TypeInstance;

  SlipFunction Fn;

  CompiledFunction();
  CompiledFunction(FuncDef &&def, SlipFunction fn);
  ExpressionPtr Clone() const;
  const std::string ToString() const;
  CompiledFunction& operator=(CompiledFunction rhs);
  void Swap(CompiledFunction &rhs);
};

struct InterpretedFunction: public Function {
  static const TypeInfo TypeInstance;

  Quote           Code;
  ArgList         Args;
  SymbolTableType Closure;

  InterpretedFunction(FuncDef &&def, ExpressionPtr &&code, ArgList &&args);
  ExpressionPtr Clone() const;
  const std::string ToString() const;
};
