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
    static bool DefaultFunction(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Print(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Quit(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Help(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Set(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool UnSet(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);

    // Generic
    static bool Add(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);

    // Numerical
    static bool AddNum(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Inc(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Dec(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Sub(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Mult(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Div(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Mod(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);

    // Bitwise
    static bool LeftShift(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool RightShift(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool BitAnd(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool BitOr(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool BitXor(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool BitNot(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);

    // String
    static bool AddString(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Reverse(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);

    // Lists
    static bool AddList(Interpreter &intepreter, ExpressionPtr &expr, ArgList &args);
    static bool List(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Map(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Head(Interpreter &intepreter, ExpressionPtr &expr, ArgList &args);
    static bool Tail(Interpreter &intepreter, ExpressionPtr &expr, ArgList &args);

    // Logical
    static bool And(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Or(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Not(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);

    // Comparison
    static bool Eq(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Ne(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Lt(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Gt(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Lte(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Gte(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);

    // Branching, scoping, evaluation
    static bool If(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Let(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool QuoteFn(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Unquote(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Begin(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool Lambda(Interpreter &interpreted, ExpressionPtr &expr, ArgList &args);
    static bool Def(Interpreter &interpreted, ExpressionPtr &expr, ArgList &args);
    static bool Apply(Interpreter &interpreted, ExpressionPtr &expr, ArgList &args);

    // Helpers
    static bool EvaluateImplicitSexp(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
    static bool PrintExpression(Interpreter &interpreter, ExpressionPtr &expr, std::ostream &out);

    template<class T>
    static bool PrintLiteral(T *expr, std::ostream &out, char *wrapper = nullptr);

    static bool PrintSexp(Interpreter &interpreter, Sexp &sexp, std::ostream &out);
    static bool PrintBool(bool expr, std::ostream &out);

    template<class F>
    static bool BinaryFunction(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args, F fn, const std::string &name);

    static bool BinaryLogicalFunc(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args, bool isAnd);

    static void RegisterBinaryFunction(SymbolTable &symbolTable, const std::string &name, SlipFunction fn);
    static void RegisterComparator(SymbolTable &symbolTable, const std::string& name, SlipFunction fn);
    static bool UnknownSymbol(Interpreter &interpreter, const std::string &where, const std::string &symName);
    static bool TypeError(Interpreter &interpreter, const std::string &where, const ExpressionPtr &expected, const ExpressionPtr &actual);
    static bool TypeError(Interpreter &interpreter, const std::string &where, const std::string &expectedName, const ExpressionPtr &actual);

    template <class T, class F, class R>
    static bool PredicateHelper(const std::string &name, Interpreter &interpreter, ExpressionPtr &expr, ArgList &args, F fn, R defaultResult);

    template <class B, class N, class S>
    static bool BinaryPredicate(const std::string &name, Interpreter &interpreter, ExpressionPtr &expr, ArgList &args, B bFn = nullptr, N nFn = nullptr, S sFn = nullptr);


    template <class F>
    static bool UnaryNumberFn(const std::string &name, Interpreter &interpreter, ExpressionPtr &expr, ArgList &args, F fn);

    static ExpressionPtr GetNil();

    static bool LambdaPrepareFormals(Interpreter &interpreter, ExpressionPtr &formalsExpr, ArgList &anonFuncArgs, int &nArgs);

    static bool CheckDivideByZero(const std::string &name, Interpreter &interpreter, ExpressionPtr &expr, ArgList &args);
};