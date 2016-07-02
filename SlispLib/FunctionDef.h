#pragma once

#include <string>
#include <initializer_list>
#include <vector>
#include <memory>
#include <functional>
#include <list>

#include "Expression.h"

class EvaluationContext;
class ArgDef;

using SlipFunction = std::function<bool(EvaluationContext&)>;
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

// Begin 0.2
struct ExampleDef {
  std::string Code;
  std::string ExpectedValue;
};

struct ParamDef {
  enum Flags {
    Standard = 0,
    Rest     = 1 << 1,
    Keyword  = 1 << 2,
    Quoted   = 1 << 3,
  };

  std::string Name;
  TypeInfo &Type;
  int Flags;
  ExpressionPtr DefaultValue;
  std::string Doc;
};

struct SignatureDef {
  TypeInfo &Type;
  std::vector<ParamDef> Params;
};

struct FuncDef2 {
  std::string Name;
  std::vector<SignatureDef> Signatures;
  std::string Doc;
  std::vector<ExampleDef> Examples;
};
// End 0.2

struct Function: public Literal {
  static const TypeInfo TypeInstance;

  FuncDef Def;
  ExpressionPtr Symbol;
  std::string Signatures;
  std::string Doc;
  std::vector<ExampleDef> Examples;

  explicit Function();
  explicit Function(FuncDef &&func);
  explicit Function(FuncDef &&func, ExpressionPtr &sym);
  explicit Function(const Function &rhs);
  virtual void Display(std::ostream &out) const override;
  bool operator==(const Function &rhs) const;
};
using FunctionPtr = std::unique_ptr<Function>;

struct CompiledFunction: public Function {
  static const TypeInfo TypeInstance;

  SlipFunction Fn;

  explicit CompiledFunction();
  explicit CompiledFunction(const CompiledFunction &rhs);
  explicit CompiledFunction(FuncDef &&def, SlipFunction fn);
  virtual ExpressionPtr Clone() const override;
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
  explicit InterpretedFunction(const InterpretedFunction &rhs);
  virtual ExpressionPtr Clone() const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const InterpretedFunction &rhs) const;
  bool operator!=(const InterpretedFunction &rhs) const;
};

class TypeHelper {
public:
  static const ExpressionPtr Null;
  static bool TypeMatches(const TypeInfo &expected, const TypeInfo &actual);
  static bool IsAtom(const TypeInfo &type);
  static bool IsAtom(const ExpressionPtr &expr);

  template<class T>
  static bool IsA(const TypeInfo &type) {
    return SimpleIsA<T>(type);
  }

  template<>
  static bool IsA<Literal>(const TypeInfo &type) {
    return IsAtom(type)
        || SimpleIsA<Literal>(type)
        || SimpleIsA<Quote>(type)
        || SimpleIsA<Ref>(type)
        ; 
  }

  template<>
  static bool IsA<Function>(const TypeInfo &type) {
    return SimpleIsA<Function>(type)
        || SimpleIsA<CompiledFunction>(type)
        || SimpleIsA<InterpretedFunction>(type)
        ;
  }

  template<class T>
  static bool IsA(const ExpressionPtr &expr) {
    if (SimpleIsA<T>(expr))
      return true;
    else if (auto *quote = dynamic_cast<Quote*>(expr.get()))
      return SimpleIsA<T>(quote->Value);
    else if (auto *ref = dynamic_cast<Ref*>(expr.get()))
      return SimpleIsA<T>(ref->Value);
    else
      return false;
  }

  template<>
  static bool IsA<Literal>(const ExpressionPtr &expr) {
    return IsA<Literal>(expr->Type());
  }

  template<>
  static bool IsA<Function>(const ExpressionPtr &expr) {
    return IsA<Function>(expr->Type());
  }

  template<class T> 
  static ExpressionPtr GetCopy(const ExpressionPtr &expr) {
    if (SimpleIsA<T>(expr))
      return expr->Clone();
    else if (auto *ref = dynamic_cast<Ref*>(expr.get())) {
      if (SimpleIsA<T>(ref->Value))
        return ref->Value->Clone();
    }
    else if (auto *quote = dynamic_cast<Quote*>(expr.get())) {
      if (SimpleIsA<T>(quote->Value))
        return quote->Value->Clone();
    }
    return ExpressionPtr { };
  }

  template<class T> 
  static ExpressionPtr& GetRef(ExpressionPtr &expr) {
    if (SimpleIsA<T>(expr))
      return expr;
    else if (auto *ref = dynamic_cast<Ref*>(expr.get())) {
      if (SimpleIsA<T>(ref->Value))
        return ref->Value;
    }
    else if (auto *quote = dynamic_cast<Quote*>(expr.get())) {
      if (SimpleIsA<T>(quote->Value))
        return quote->Value;
    }
    return const_cast<ExpressionPtr&>(Null); 
  }

  template<class T>
  static T* GetValue(ExpressionPtr &expr) {
    auto &ref = GetRef<T>(expr);
    if (ref) {
      return static_cast<T*>(ref.get());
    }
    return nullptr;
  }

  template<class T>
  static bool SimpleIsA(const ExpressionPtr &expr) {
    return SimpleIsA<T>(expr->Type());
  }

  template<class T>
  static bool SimpleIsA(const TypeInfo &type) {
    return &type == &T::TypeInstance;
  }

};

