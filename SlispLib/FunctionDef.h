#pragma once

#include <string>
#include <initializer_list>
#include <vector>
#include <memory>
#include <functional>
#include <list>

#include "Expression.h"

class Interpreter;
class ArgDef;

using SlipFunction = std::function<bool(Interpreter &, ExpressionPtr&, ArgList&)>;
using ExpressionEvaluator = std::function<bool(ExpressionPtr&)>;
using ArgDefPtr = std::unique_ptr<ArgDef>;

class ArgDef {
  public:
    virtual ~ArgDef();
    virtual ArgDefPtr Clone() const = 0;
    virtual bool operator==(const ArgDef &rhs) const = 0;
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

class FuncDef {
  public:
    static ArgDefPtr NoArgs();
    static ArgDefPtr OneArg(const TypeInfo& type);
    static ArgDefPtr AnyArgs(const TypeInfo& type);
    static ArgDefPtr AnyArgs();
    static ArgDefPtr ManyArgs(const TypeInfo& type, int nargs);
    static ArgDefPtr Args(std::initializer_list<const TypeInfo*> &&args);

    explicit FuncDef(ArgDefPtr &in, ArgDefPtr &out);
    FuncDef(const FuncDef &val);
    FuncDef(FuncDef &&rval);
    FuncDef Clone() const;
    bool operator==(const FuncDef &rhs) const;
    FuncDef& operator=(FuncDef);
    void Swap(FuncDef &func);
    const std::string ToString() const;
    bool ValidateArgs(ExpressionEvaluator evaluator, ExpressionPtr &expr, std::string &error);

  private:
    class VarArgDef: public ArgDef {
      public:
        static const int NO_ARGS = 0;
        static const int ANY_ARGS = -1;

        explicit VarArgDef(const TypeInfo& type, int nargs);
        virtual ArgDefPtr Clone() const;
        virtual const std::string ToString() const;
        virtual bool operator==(const ArgDef &rhs) const;
        bool operator==(const VarArgDef &rhs) const;

      private:
        const TypeInfo& Type;
        int NArgs;

        virtual bool ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const;

        bool ValidateArgCount(ArgList &args, std::string &error) const;
        bool ValidateArgTypes(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const;
    };

    class ListArgDef: public ArgDef {
      public:
        explicit ListArgDef(std::initializer_list<const TypeInfo*> &&types);
        virtual ArgDefPtr Clone() const;
        virtual const std::string ToString() const;
        virtual bool operator==(const ArgDef &rhs) const;
        bool operator==(const ListArgDef &rhs) const;

      private:
        std::vector<const TypeInfo*> Types;

        virtual bool ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const;
    };

    ArgDefPtr In;
    ArgDefPtr Out;
};

struct Function: public Literal {
  static const TypeInfo TypeInstance;

  FuncDef Def;

  explicit Function();
  explicit Function(FuncDef &&func);
};
using FunctionPtr = std::unique_ptr<Function>;

struct CompiledFunction: public Function {
  static const TypeInfo TypeInstance;

  SlipFunction Fn;

  explicit CompiledFunction();
  explicit CompiledFunction(FuncDef &&def, SlipFunction fn);
  ExpressionPtr Clone() const;
  const std::string ToString() const;
  virtual bool operator==(const Expression &rhs) const;
  bool operator==(const CompiledFunction &rhs) const;
  bool operator!=(const CompiledFunction &rhs) const;
  CompiledFunction& operator=(CompiledFunction rhs);
  void Swap(CompiledFunction &rhs);
};

struct InterpretedFunction: public Function {
  static const TypeInfo TypeInstance;

  Quote           Code;
  ArgList         Args;
  SymbolTableType Closure;

  explicit InterpretedFunction(FuncDef &&def, ExpressionPtr &&code, ArgList &&args);
  ExpressionPtr Clone() const;
  const std::string ToString() const;
  virtual bool operator==(const Expression &rhs) const;
  bool operator==(const InterpretedFunction &rhs) const;
  bool operator!=(const InterpretedFunction &rhs) const;
};
