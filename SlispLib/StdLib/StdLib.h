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
    static bool Display(EvaluationContext &ctx);
    static bool Print(EvaluationContext &ctx);
    static bool Prompt(EvaluationContext &ctx);
    static bool Quit(EvaluationContext &ctx);
    static bool Help(EvaluationContext &ctx);
    static bool InfixRegister(EvaluationContext &ctx);
    static bool InfixUnregister(EvaluationContext &ctx);

    // IO
    static bool Exists(EvaluationContext &ctx);
    static bool Delete(EvaluationContext &ctx);
    static bool ReadLines(EvaluationContext &ctx);
    static bool WriteLines(EvaluationContext &ctx);

    // Assignment operators
    static bool Set(EvaluationContext &ctx);
    static bool UnSet(EvaluationContext &ctx);

    // Generic
    static bool Add(EvaluationContext &ctx);
    static bool EmptyQ(EvaluationContext &ctx);
    static bool Length(EvaluationContext &ctx);
    static bool Sub(EvaluationContext &ctx);
    static bool Mult(EvaluationContext &ctx);
    static bool Div(EvaluationContext &ctx);
    static bool Pow(EvaluationContext &ctx);
    static bool Abs(EvaluationContext &ctx);
    static bool Max(EvaluationContext &ctx);
    static bool Min(EvaluationContext &ctx);
    static bool Foreach(EvaluationContext &ctx);
    static bool Reverse(EvaluationContext &ctx);
    static bool At(EvaluationContext &ctx);
    static bool Head(EvaluationContext &ctx);
    static bool Tail(EvaluationContext &ctx);
    static bool Last(EvaluationContext &ctx);

    // Int
    static bool AddInt(EvaluationContext &ctx);
    static bool Incr(EvaluationContext &ctx);
    static bool Decr(EvaluationContext &ctx);
    static bool SubInt(EvaluationContext &ctx);
    static bool MultInt(EvaluationContext &ctx);
    static bool DivInt(EvaluationContext &ctx);
    static bool Mod(EvaluationContext &ctx);
    static bool PowInt(EvaluationContext &ctx);
    static bool AbsInt(EvaluationContext &ctx);
    static bool MaxInt(EvaluationContext &ctx);
    static bool MinInt(EvaluationContext &ctx);
    static bool Hex(EvaluationContext &ctx);
    static bool Bin(EvaluationContext &ctx);
    static bool Dec(EvaluationContext &ctx);
    static bool Even(EvaluationContext &ctx);
    static bool Odd(EvaluationContext &ctx);
    static bool Zero(EvaluationContext &ctx);

    // Float - Common
    static bool AddFloat(EvaluationContext &ctx);
    static bool SubFloat(EvaluationContext &ctx);
    static bool MultFloat(EvaluationContext &ctx);
    static bool DivFloat(EvaluationContext &ctx);
    static bool AbsFloat(EvaluationContext &ctx);
    static bool MaxFloat(EvaluationContext &ctx);
    static bool MinFloat(EvaluationContext &ctx);

    // Float - Trig
    static bool Cos(EvaluationContext &ctx);
    static bool Sin(EvaluationContext &ctx);
    static bool Tan(EvaluationContext &ctx);
    static bool ACos(EvaluationContext &ctx);
    static bool ASin(EvaluationContext &ctx);
    static bool ATan(EvaluationContext &ctx);
    static bool ATan2(EvaluationContext &ctx);

    // Float - Hyp
    static bool Cosh(EvaluationContext &ctx);
    static bool Sinh(EvaluationContext &ctx);
    static bool Tanh(EvaluationContext &ctx);
    static bool ACosh(EvaluationContext &ctx);
    static bool ASinh(EvaluationContext &ctx);
    static bool ATanh(EvaluationContext &ctx);

    // Float - Exp and Logs
    static bool PowFloat(EvaluationContext &ctx);
    static bool Exp(EvaluationContext &ctx);
    static bool Log(EvaluationContext &ctx);
    static bool Sqrt(EvaluationContext &ctx);

    // Float - Round
    static bool Ceil(EvaluationContext &ctx);
    static bool Floor(EvaluationContext &ctx);
    static bool Round(EvaluationContext &ctx);

    // Bitwise
    static bool LeftShift(EvaluationContext &ctx);
    static bool RightShift(EvaluationContext &ctx);
    static bool BitAnd(EvaluationContext &ctx);
    static bool BitOr(EvaluationContext &ctx);
    static bool BitXor(EvaluationContext &ctx);
    static bool BitNot(EvaluationContext &ctx);

    // Str
    static bool AddStr(EvaluationContext &ctx);
    static bool Trim(EvaluationContext &ctx);
    static bool Upper(EvaluationContext &ctx);
    static bool Lower(EvaluationContext &ctx);
    static bool SubStr(EvaluationContext &ctx);
    static bool Contains(EvaluationContext &ctx);
    static bool Compare(EvaluationContext &ctx);
    static bool StartsWith(EvaluationContext &ctx);
    static bool EndsWith(EvaluationContext &ctx);
    static bool Find(EvaluationContext &ctx);
    static bool RFind(EvaluationContext &ctx);
    static bool Replace(EvaluationContext &ctx);
    static bool Split(EvaluationContext &ctx);
    static bool Join(EvaluationContext &ctx);
    static bool Format(EvaluationContext &ctx);

    // Lists
    static bool AddList(EvaluationContext &ctx);
    static bool List(EvaluationContext &ctx);
    static bool Map(EvaluationContext &ctx);
    static bool Filter(EvaluationContext &ctx);
    static bool Reduce(EvaluationContext &ctx);
    static bool Zip(EvaluationContext &ctx);
    static bool Any(EvaluationContext &ctx);
    static bool All(EvaluationContext &ctx);
    static bool Cons(EvaluationContext &ctx);
    static bool Range(EvaluationContext &ctx);

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
    static bool Cond(EvaluationContext &ctx);
    static bool Switch(EvaluationContext &ctx);
    static bool While(EvaluationContext &ctx);
    static bool Let(EvaluationContext &ctx);
    static bool QuoteFn(EvaluationContext &ctx);
    static bool Unquote(EvaluationContext &ctx);
    static bool Begin(EvaluationContext &ctx);
    static bool Lambda(EvaluationContext &ctx);
    static bool Def(EvaluationContext &ctx);
    static bool Apply(EvaluationContext &ctx);

    // Conversion operators
    static bool TypeFunc(EvaluationContext &ctx);
    static bool TypeQFunc(EvaluationContext &ctx);
    static bool BoolFunc(EvaluationContext &ctx);
    static bool IntFunc(EvaluationContext &ctx);
    static bool FloatFunc(EvaluationContext &ctx);
    static bool StrFunc(EvaluationContext &ctx);

    // Helpers
    static bool Render(EvaluationContext &ctx, bool isDisplay);
    static bool ForeachIterate(EvaluationContext &ctx, Expression *iterableArg, Symbol *currElementSym, Function *fn);
    template <class S, class L>
    static bool SequenceFn(EvaluationContext &ctx, S strFn, L listFn);

    static bool IsSexpAList(EvaluationContext &ctx, Sexp &sexp);
    static bool IsQuoteAList(EvaluationContext &ctx, Quote &quote);
    static bool EvaluateListSexp(EvaluationContext &ctx); 

    static bool InfixRegistrationFunction(EvaluationContext &ctx, const std::string &name, bool unregister);

    template <class T, class F>
    static bool UnaryFunction(EvaluationContext &ctx, F fn);

    template <class T, class R, class F>
    static bool UnaryFunction(EvaluationContext &ctx, F fn);

    template<class T, class F>
    static bool BinaryFunction(EvaluationContext &ctx, F fn);

    static bool BinaryLogicalFunc(EvaluationContext &ctx, bool isAnd);

    static void RegisterBinaryFunction(SymbolTable &symbolTable, const std::string &name, SlipFunction fn);
    static void RegisterComparator(SymbolTable &symbolTable, const std::string& name, SlipFunction fn);

    template <class T, class F, class R>
    static bool PredicateHelper(EvaluationContext &ctx, F fn, R defaultResult);

    template <class B, class I, class F, class S>
    static bool BinaryPredicate(EvaluationContext &ctx, B bFn = nullptr, I iFn = nullptr, F fFn = nullptr, S sFn = nullptr);

    template <class I, class F>
    static bool GenericNumFunc(EvaluationContext &ctx, I iFn, F fFn);

    template <class T>
    static bool CheckDivideByZero(EvaluationContext &ctx);

    static ExpressionPtr GetNil();

    static bool LambdaPrepareFormals(EvaluationContext &ctx, ExpressionPtr &formalsExpr, ArgList &anonFuncArgs, int &nArgs);


    enum class ListTransforms {
      Map,
      Filter,
      Reduce,
      Any,
      All
    };
    static bool TransformList(EvaluationContext &ctx, ListTransforms transform);
};