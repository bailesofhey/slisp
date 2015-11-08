#include "gtest\gtest.h"
#include "FunctionDef.h"

bool TestEvaluator(ExpressionPtr &) {
  return true;
}

void AssertValidate(FuncDef &funcDef, ExpressionPtr &&expr, bool expectSuccess) {
  ExpressionPtr copy = expr ? expr->Clone() : nullptr;
  std::string error;
  ASSERT_EQ(expectSuccess, funcDef.ValidateArgs(TestEvaluator, expr, error)) << copy->ToString();
  ASSERT_EQ(expectSuccess, error.empty()) << copy->ToString();
}

void AssertValidateThrow(FuncDef &funcDef, ExpressionPtr &&expr) {
  ExpressionPtr copy = expr ? expr->Clone() : nullptr;
  std::string error;
  ASSERT_THROW(funcDef.ValidateArgs(TestEvaluator, expr, error), std::exception) << copy->ToString();
}

bool TestSlispFn(Interpreter &, ExpressionPtr&, ArgList&) {
  return true;
}

//TODO: Need global type list
//=============================================================================

ExpressionPtr PrimitiveExpressions[] {
  ExpressionPtr { new Bool() },
  ExpressionPtr { new Number() },
  ExpressionPtr { new String() },
  ExpressionPtr { new Symbol("+") },
};

void TestBadExpressions(FuncDef &funcDef) {
  ASSERT_NO_FATAL_FAILURE(AssertValidateThrow(funcDef, ExpressionPtr {}));
  for (const auto &expr : PrimitiveExpressions) {
    ASSERT_NO_FATAL_FAILURE(AssertValidate(funcDef, expr->Clone(), false));
    ASSERT_NO_FATAL_FAILURE(AssertValidate(funcDef, ExpressionPtr { new Quote(expr->Clone()) }, false)) << "Quote";
    
    Sexp s;
    s.Args.push_back(expr->Clone());
    ASSERT_NO_FATAL_FAILURE(AssertValidateThrow(funcDef, s.Clone()));
  }
}

TEST(FuncDef, TestFuncDef_NoArgs) {
  FuncDef funcDef(FuncDef::NoArgs(), FuncDef::NoArgs());
  ASSERT_NO_FATAL_FAILURE(TestBadExpressions(funcDef)) << "TestBadExpressions";

  Sexp s;
  s.Args.push_back(ExpressionPtr { new CompiledFunction(FuncDef(funcDef), TestSlispFn) });
  ASSERT_NO_FATAL_FAILURE(AssertValidate(funcDef, s.Clone(), true));

  for (const auto &expr : PrimitiveExpressions) {
    ExpressionPtr sCopyPtr = s.Clone();
    Sexp *sCopy = static_cast<Sexp*>(sCopyPtr.get());
    sCopy->Args.push_back(expr->Clone());
    ASSERT_NO_FATAL_FAILURE(AssertValidate(funcDef, std::move(sCopyPtr), false)) << "(func " << expr->ToString() << ")";
  }
}


TEST(FuncDef, TestFuncDef_OneArg) {
}

TEST(FuncDef, TestFuncDef_AnyArgs) {
}

TEST(FuncDef, TestFuncDef_ManyArgs) {
}

TEST(FuncDef, TestFuncDef_Args) {
}

TEST(FuncDef, TestFuncDef_Explicit) {
}