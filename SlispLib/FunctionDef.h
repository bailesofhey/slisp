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

using SlipFunction = std::function<bool(Interpreter &, ExpressionPtr &, ArgList &)>;
using ExpressionEvaluator = std::function<bool(ExpressionPtr &)>;
using ArgDefPtr = std::unique_ptr<ArgDef>;

class ArgDef {
  public:
    static const size_t NO_ARGS = 0;
    static const size_t ANY_ARGS = -1;
    
    virtual ~ArgDef();
    virtual ArgDefPtr Clone() const = 0;
    virtual bool operator==(const ArgDef &rhs) const = 0;
    virtual const std::string ToString() const = 0;
    bool Validate(ExpressionEvaluator evaluator, ExpressionPtr &expr, std::string &error) const;

  protected:
    virtual bool ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const = 0;
    bool CheckArgCount(size_t expectedMin, size_t expectedMax, ArgList &args, std::string &error) const;
    bool CheckArgCount(size_t expected, ArgList &args, std::string &error) const;
    bool CheckArg(ExpressionEvaluator evaluator, ExpressionPtr &arg, const TypeInfo &expectedType, size_t argNum, std::string &error) const;
};

class FuncDef {
  public:
    static ArgDefPtr NoArgs();
    static ArgDefPtr OneArg(const TypeInfo &type);
    static ArgDefPtr AtleastOneArg();
    static ArgDefPtr AtleastOneArg(const TypeInfo &type);
    static ArgDefPtr AnyArgs(const TypeInfo &type);
    static ArgDefPtr AnyArgs();
    static ArgDefPtr ManyArgs(const TypeInfo &type, size_t nArgs);
    static ArgDefPtr ManyArgs(const TypeInfo &type, size_t minArgs, size_t maxArgs);
    static ArgDefPtr Args(std::initializer_list<const TypeInfo*> &&args);

    explicit FuncDef(ArgDefPtr &in, ArgDefPtr &out);
    FuncDef(const FuncDef &val);
    FuncDef(FuncDef &&rval);
    FuncDef Clone() const;
    bool operator==(const FuncDef &rhs) const;
    bool operator!=(const FuncDef &rhs) const;
    FuncDef& operator=(FuncDef);
    void Swap(FuncDef &func);
    const std::string ToString() const;
    bool ValidateArgs(ExpressionEvaluator evaluator, ExpressionPtr &expr, std::string &error);

  private:
    class VarArgDef: public ArgDef {
      public:

        explicit VarArgDef(const TypeInfo &type, size_t nArgs);
        explicit VarArgDef(const TypeInfo &type, size_t minArgs, size_t maxArgs);
        virtual ArgDefPtr Clone() const override;
        virtual const std::string ToString() const override;
        virtual bool operator==(const ArgDef &rhs) const override;
        bool operator==(const VarArgDef &rhs) const;

      private:
        const TypeInfo &Type;
        size_t MinArgs;
        size_t MaxArgs;

        virtual bool ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const override;

        bool IsAnyArgs() const;
        bool ValidateArgCount(ArgList &args, std::string &error) const;
        bool ValidateArgTypes(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const;
    };

    class ListArgDef: public ArgDef {
      public:
        explicit ListArgDef(std::initializer_list<const TypeInfo*> &&types);
        virtual ArgDefPtr Clone() const override;
        virtual const std::string ToString() const override;
        virtual bool operator==(const ArgDef &rhs) const override;
        bool operator==(const ListArgDef &rhs) const;

      private:
        std::vector<const TypeInfo*> Types;

        virtual bool ValidateArgs(ExpressionEvaluator evaluator, ArgList &args, std::string &error) const override;
    };

    ArgDefPtr In;
    ArgDefPtr Out;
};

struct Function: public Literal {
  static const TypeInfo TypeInstance;

  FuncDef Def;
  ExpressionPtr Symbol;

  explicit Function();
  explicit Function(FuncDef &&func);
  Function(const Function &rhs);
  bool operator==(const Function &rhs) const;
};
using FunctionPtr = std::unique_ptr<Function>;

struct CompiledFunction: public Function {
  static const TypeInfo TypeInstance;

  SlipFunction Fn;

  explicit CompiledFunction();
  explicit CompiledFunction(FuncDef &&def, SlipFunction fn);
  virtual ExpressionPtr Clone() const override;
  virtual const std::string ToString() const override;
  virtual bool operator==(const Expression &rhs) const override;
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
  virtual ExpressionPtr Clone() const override;
  virtual const std::string ToString() const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const InterpretedFunction &rhs) const;
  bool operator!=(const InterpretedFunction &rhs) const;
};

class TypeHelper {
  public:
    static bool TypeMatches(const TypeInfo &expected, const TypeInfo &actual);
    static bool IsAtom(const TypeInfo &type);
    static bool IsLiteral(const TypeInfo &type);
    static bool IsFunction(const TypeInfo &type);
    static bool IsConvertableToNumber(const TypeInfo &type);
    static ExpressionPtr GetNumber(ExpressionPtr &expr);
    static ExpressionPtr GetBool(ExpressionPtr &expr);
};