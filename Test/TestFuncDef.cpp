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

void TestArgRecursive(FuncDef &funcDef, Sexp &s, int maxArgs, int argNum, std::function<bool(const Sexp &)> argSuccessFn) {
  if (argNum) {
    for (const auto &arg : PrimitiveExpressions) {
      ExpressionPtr sCopyPtr = s.Clone();
      Sexp *sCopy = static_cast<Sexp*>(sCopyPtr.get());
      sCopy->Args.push_back(arg->Clone());
      ASSERT_NO_FATAL_FAILURE(AssertValidate(funcDef, sCopyPtr->Clone(), argSuccessFn(*sCopy))) << "argNum: " << argNum;
      if (argNum != maxArgs)
        ASSERT_NO_FATAL_FAILURE(TestArgRecursive(funcDef, *sCopy, maxArgs, argNum + 1, argSuccessFn));
    }
  }
  else {
    ASSERT_NO_FATAL_FAILURE(AssertValidate(funcDef, s.Clone(), argSuccessFn(s)));
    if (argNum != maxArgs)
      ASSERT_NO_FATAL_FAILURE(TestArgRecursive(funcDef, s, maxArgs, argNum + 1, argSuccessFn));
  }
}

void TestArgs(FuncDef &funcDef, int maxArgs, std::function<bool(const Sexp &)> argSuccessFn) {
  Sexp s;
  s.Args.push_back(ExpressionPtr { new CompiledFunction(FuncDef(funcDef), TestSlispFn) });
  ASSERT_NO_FATAL_FAILURE(TestBadExpressions(funcDef)) << "TestBadExpressions";
  ASSERT_NO_FATAL_FAILURE(TestArgRecursive(funcDef, s, maxArgs, 0, argSuccessFn));
}

TEST(FuncDef, TestFuncDef_NoArgs) {
  FuncDef funcDef(FuncDef::NoArgs(), FuncDef::NoArgs());
  ASSERT_NO_FATAL_FAILURE(TestArgs(funcDef, 1, [](const Sexp &sexp) {
    return sexp.Args.size() == 1; // (func)
  }));
}

TEST(FuncDef, TestFuncDef_OneArg) {
  for (const auto &expr : PrimitiveExpressions) {
    FuncDef funcDef(FuncDef::OneArg(expr->Type()), FuncDef::NoArgs());
    ASSERT_NO_FATAL_FAILURE(TestArgs(funcDef, 2, [&expr](const Sexp &sexp) {
      if (sexp.Args.size() == 2) {
        int argNum = 0;
        for (auto &arg : sexp.Args) {
          if (argNum)
            return &arg->Type() == &expr->Type();
          else
            ++argNum;
        }
      }
      return false;
    })) << expr->ToString();
  }
}

TEST(FuncDef, TestFuncDef_AnyArgs_Homogeneous) {
  for (const auto &expr : PrimitiveExpressions) {
    FuncDef funcDef(FuncDef::AnyArgs(expr->Type()), FuncDef::NoArgs());
    ASSERT_NO_FATAL_FAILURE(TestArgs(funcDef, 3, [&expr](const Sexp &sexp) {
      int argNum = 0;
      for (auto &arg : sexp.Args) {
        if (argNum && &arg->Type() != &expr->Type())
          return false;
        ++argNum;
      }
      return true;
    })) << expr->ToString();
  }
}

TEST(FuncDef, TestFuncDef_AnyArgs_Heterogeneous) {
  FuncDef funcDef(FuncDef::AnyArgs(), FuncDef::NoArgs());
  ASSERT_NO_FATAL_FAILURE(TestArgs(funcDef, 3, [](const Sexp &sexp) {
    return true;
  }));
}

TEST(FuncDef, TestFuncDef_ManyArgs) {
  int maxExpectedNArgs = 3;
  for (int expectedNArgs = 0; expectedNArgs <= maxExpectedNArgs; ++expectedNArgs) {
    for (const auto &expr : PrimitiveExpressions) {
      FuncDef funcDef(FuncDef::ManyArgs(expr->Type(), expectedNArgs), FuncDef::NoArgs());
      ASSERT_NO_FATAL_FAILURE(TestArgs(funcDef, maxExpectedNArgs + 1, [&expectedNArgs, &expr](const Sexp &sexp) {
        if (sexp.Args.size() == (expectedNArgs + 1)) { // (func arg[1] ... arg[expectedNArgs])
          int argNum = 0;
          for (auto &arg : sexp.Args) {
            if (argNum && &arg->Type() != &expr->Type())
              return false;
            ++argNum;
          }
          return true;
        }
        return false;
      })) << expr->ToString();
    }
  }
}

TEST(FuncDef, TestFuncDef_Args) {
}

TEST(FuncDef, TestFuncDef_Explicit) {
}