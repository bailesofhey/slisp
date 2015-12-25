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

    // Numerical
    static bool AddNum(EvaluationContext &ctx);
    static bool Inc(EvaluationContext &ctx);
    static bool Dec(EvaluationContext &ctx);
    static bool Sub(EvaluationContext &ctx);
    static bool Mult(EvaluationContext &ctx);
    static bool Div(EvaluationContext &ctx);
    static bool Mod(EvaluationContext &ctx);

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

    // Helpers
    static bool EvaluateListSexp(EvaluationContext &ctx); 
    static bool PrintExpression(EvaluationContext &ctx, ExpressionPtr &curr, std::ostream &out);

    template<class T>
    static bool PrintLiteral(T *expr, std::ostream &out, char *wrapper = nullptr);

    static bool PrintSexp(EvaluationContext &ctx, Sexp &sexp, std::ostream &out);
    static bool PrintBool(bool expr, std::ostream &out);
    static bool InfixRegistrationFunction(EvaluationContext &ctx, const std::string &name, bool unregister);

    template<class F>
    static bool BinaryFunction(EvaluationContext &ctx, F fn, const std::string &name);

    static bool BinaryLogicalFunc(EvaluationContext &ctx, bool isAnd);

    static void RegisterBinaryFunction(SymbolTable &symbolTable, const std::string &name, SlipFunction fn);
    static void RegisterComparator(SymbolTable &symbolTable, const std::string& name, SlipFunction fn);

    template <class T, class F, class R>
    static bool PredicateHelper(EvaluationContext &ctx, const std::string &name, F fn, R defaultResult);

    template <class B, class N, class S>
    static bool BinaryPredicate(EvaluationContext &ctx, const std::string &name, B bFn = nullptr, N nFn = nullptr, S sFn = nullptr);


    template <class F>
    static bool UnaryNumberFn(EvaluationContext &ctx, const std::string &name, F fn);

    static ExpressionPtr GetNil();

    static bool LambdaPrepareFormals(EvaluationContext &ctx, ExpressionPtr &formalsExpr, ArgList &anonFuncArgs, int &nArgs);

    static bool CheckDivideByZero(EvaluationContext &ctx, const std::string &name);
};