#pragma once

#include <string>

#include "../Interpreter.h"
#include "../Library.h"

class StdLib: public Library {
  public:
    virtual void Load(Interpreter &interpreter) override;
    virtual void UnLoad(Interpreter &interpreter) override;

  private:
    // Interpreter
    static bool DefaultFunction(EvaluationContext &ctx);
    static bool Print(EvaluationContext &ctx);
    static bool Quit(EvaluationContext &ctx);
    static bool Help(EvaluationContext &ctx);
    static bool InfixRegister(EvaluationContext &ctx);
    static bool InfixUnregister(EvaluationContext &ctx);

    // Assignment operators
    static bool Set(EvaluationContext &ctx);
    static bool UnSet(EvaluationContext &ctx);

    // Generic
    static bool Add(EvaluationContext &ctx);
    static bool Sub(EvaluationContext &ctx);
    static bool Mult(EvaluationContext &ctx);
    static bool Div(EvaluationContext &ctx);

    // Int
    static bool AddInt(EvaluationContext &ctx);
    static bool Incr(EvaluationContext &ctx);
    static bool Decr(EvaluationContext &ctx);
    static bool SubInt(EvaluationContext &ctx);
    static bool MultInt(EvaluationContext &ctx);
    static bool DivInt(EvaluationContext &ctx);
    static bool Mod(EvaluationContext &ctx);
    static bool Hex(EvaluationContext &ctx);
    static bool Bin(EvaluationContext &ctx);
    static bool Dec(EvaluationContext &ctx);

    // Float
    static bool AddFloat(EvaluationContext &ctx);
    static bool SubFloat(EvaluationContext &ctx);
    static bool MultFloat(EvaluationContext &ctx);
    static bool DivFloat(EvaluationContext &ctx);

    // Bitwise
    static bool LeftShift(EvaluationContext &ctx);
    static bool RightShift(EvaluationContext &ctx);
    static bool BitAnd(EvaluationContext &ctx);
    static bool BitOr(EvaluationContext &ctx);
    static bool BitXor(EvaluationContext &ctx);
    static bool BitNot(EvaluationContext &ctx);

    // String
    static bool AddString(EvaluationContext &ctx);
    static bool Reverse(EvaluationContext &ctx);

    // Lists
    static bool AddList(EvaluationContext &ctx);
    static bool List(EvaluationContext &ctx);
    static bool Map(EvaluationContext &ctx);
    static bool Head(EvaluationContext &ctx);
    static bool Tail(EvaluationContext &ctx);

    // Logical
    static bool And(EvaluationContext &ctx);
    static bool Or(EvaluationContext &ctx);
    static bool Not(EvaluationContext &ctx);

    // Comparison
    static bool Eq(EvaluationContext &ctx);
    static bool Ne(EvaluationContext &ctx);
    static bool Lt(EvaluationContext &ctx);
    static bool Gt(EvaluationContext &ctx);
    static bool Lte(EvaluationContext &ctx);
    static bool Gte(EvaluationContext &ctx);

    // Branching, scoping, evaluation
    static bool If(EvaluationContext &ctx);
    static bool Let(EvaluationContext &ctx);
    static bool QuoteFn(EvaluationContext &ctx);
    static bool Unquote(EvaluationContext &ctx);
    static bool Begin(EvaluationContext &ctx);
    static bool Lambda(EvaluationContext &ctx);
    static bool Def(EvaluationContext &ctx);
    static bool Apply(EvaluationContext &ctx);

    // Conversion operators
    static bool BoolFunc(EvaluationContext &ctx);
    static bool IntFunc(EvaluationContext &ctx);
    static bool FloatFunc(EvaluationContext &ctx);
    static bool StrFunc(EvaluationContext &ctx);

    // Helpers
    static bool EvaluateListSexp(EvaluationContext &ctx); 

    static bool InfixRegistrationFunction(EvaluationContext &ctx, const std::string &name, bool unregister);

    template<class T, class F>
    static bool BinaryFunction(EvaluationContext &ctx, F fn, const std::string &name);

    static bool BinaryLogicalFunc(EvaluationContext &ctx, bool isAnd);

    static void RegisterBinaryFunction(SymbolTable &symbolTable, const std::string &name, SlipFunction fn);
    static void RegisterComparator(SymbolTable &symbolTable, const std::string& name, SlipFunction fn);

    template <class T, class F, class R>
    static bool PredicateHelper(EvaluationContext &ctx, const std::string &name, F fn, R defaultResult);

    template <class B, class I, class F, class S>
    static bool BinaryPredicate(EvaluationContext &ctx, const std::string &name, B bFn = nullptr, I iFn = nullptr, F fFn = nullptr, S sFn = nullptr);


    template <class F>
    static bool UnaryNumberFn(EvaluationContext &ctx, const std::string &name, F fn);

    template <class I, class F>
    static bool GenericNumFunc(EvaluationContext &ctx, I iFn, F fFn);

    static ExpressionPtr GetNil();

    static bool LambdaPrepareFormals(EvaluationContext &ctx, ExpressionPtr &formalsExpr, ArgList &anonFuncArgs, int &nArgs);

    template <class T>
    static bool CheckDivideByZero(EvaluationContext &ctx, const std::string &name);
};