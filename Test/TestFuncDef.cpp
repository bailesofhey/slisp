#include "gtest\gtest.h"
#include "FunctionDef.h"

ExpressionPtr PrimitiveExpressions[] {
  ExpressionPtr { new Bool() },
  ExpressionPtr { new Int() },
  ExpressionPtr { new String() },
  ExpressionPtr { new Symbol("+") },
};

bool TestEvaluator(ExpressionPtr &) {
  return false;
}

bool TestSlispFn(EvaluationContext &ctx) {
  throw std::exception("This should not be called");
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

static const int ANY_NARGS = -1;

bool HomogeneousArgSuccessFn(int expectedMinArgs, int expectedMaxArgs, const ExpressionPtr &expr, const Sexp &sexp) {
  auto actualArgs = sexp.Args.size();
  if ((expectedMinArgs == ANY_NARGS || actualArgs >= (expectedMinArgs + 1)) &&
      (expectedMaxArgs == ANY_NARGS || actualArgs <= (expectedMaxArgs + 1))) { // (func arg[1] ... arg[expectedNArgs])
    int argNum = 0;
    for (auto &arg : sexp.Args) {
      if (argNum && !TypeHelper::TypeMatches(expr->Type(), arg->Type()))
        return false;
      ++argNum;
    }
    return true;
  }
  return false;
}

bool HomogeneousArgSuccessFn(int expectedNArgs, const ExpressionPtr &expr, const Sexp &sexp) {
  return HomogeneousArgSuccessFn(expectedNArgs, expectedNArgs, expr, sexp);
}

TEST(FuncDef, TestFuncDef_NoArgs) {
  FuncDef funcDef(FuncDef::NoArgs(), FuncDef::NoArgs());
  ASSERT_NO_FATAL_FAILURE(TestArgs(funcDef, 1, [](const Sexp &sexp) {
    return HomogeneousArgSuccessFn(0, nullptr, sexp);
  }));
}

TEST(FuncDef, TestFuncDef_OneArg) {
  for (const auto &expr : PrimitiveExpressions) {
    FuncDef funcDef(FuncDef::OneArg(expr->Type()), FuncDef::NoArgs());
    ASSERT_NO_FATAL_FAILURE(TestArgs(funcDef, 2, [&expr](const Sexp &sexp) {
      return HomogeneousArgSuccessFn(1, expr, sexp);
    })) << expr->ToString();
  }
}

TEST(FuncDef, TestFuncDef_AnyArgs_Homogeneous) {
  for (const auto &expr : PrimitiveExpressions) {
    FuncDef funcDef(FuncDef::AnyArgs(expr->Type()), FuncDef::NoArgs());
    ASSERT_NO_FATAL_FAILURE(TestArgs(funcDef, 3, [&expr](const Sexp &sexp) {
      return HomogeneousArgSuccessFn(ANY_NARGS, expr, sexp);
    })) << expr->ToString();
  }
}

TEST(FuncDef, TestFuncDef_AtleastOneArg_Homogeneous) {
  for (const auto &expr : PrimitiveExpressions) {
    FuncDef funcDef(FuncDef::AtleastOneArg(expr->Type()), FuncDef::NoArgs());
    ASSERT_NO_FATAL_FAILURE(TestArgs(funcDef, 3, [&expr](const Sexp &sexp) {
      return HomogeneousArgSuccessFn(1, ANY_NARGS, expr, sexp);
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
        return HomogeneousArgSuccessFn(expectedNArgs, expr, sexp);
      })) << expr->ToString();
    }
  }
}

bool ThreeArgSuccessFn(const Sexp &sexp, const ExpressionPtr &arg1, const ExpressionPtr &arg2, const ExpressionPtr &arg3) {
  int expectedNArgs = 0;
  for (auto *arg : { &arg1, &arg2, &arg3 }) {
    if (*arg)
      ++expectedNArgs;
  }

  if (sexp.Args.size() == (expectedNArgs + 1)) {
    int argNum = 0;
    for (auto &sexpArg : sexp.Args) {
      auto sexpArgType = &sexpArg->Type();
      if (argNum == 1 && !TypeHelper::TypeMatches(arg1->Type(), *sexpArgType))
        return false;
      else if (argNum == 2 && !TypeHelper::TypeMatches(arg2->Type(), *sexpArgType))
        return false;
      else if (argNum == 3 && !TypeHelper::TypeMatches(arg3->Type(), *sexpArgType))
        return false;
      ++argNum;
    }
    return true;
  }
  return false;
}

TEST(FuncDef, TestFuncDef_Args) {
  FuncDef noArgFuncDef(FuncDef::Args({}), FuncDef::NoArgs());
  ASSERT_NO_FATAL_FAILURE(TestArgs(noArgFuncDef, 1, [](const Sexp &sexp) {
    return ThreeArgSuccessFn(sexp, nullptr, nullptr, nullptr);
  }));

  for (const auto &arg1 : PrimitiveExpressions) {
    FuncDef oneArgFuncDef(FuncDef::Args({ &arg1->Type() }), FuncDef::NoArgs());
    ASSERT_NO_FATAL_FAILURE(TestArgs(oneArgFuncDef, 2, [&arg1](const Sexp &sexp){
      return ThreeArgSuccessFn(sexp, arg1, nullptr, nullptr);
    }));
    
    for (const auto &arg2 : PrimitiveExpressions) {
      FuncDef twoArgFuncDef(FuncDef::Args({ &arg1->Type(), &arg2->Type() }), FuncDef::NoArgs());
      ASSERT_NO_FATAL_FAILURE(TestArgs(twoArgFuncDef, 3, [&arg1, &arg2](const Sexp &sexp) {
        return ThreeArgSuccessFn(sexp, arg1, arg2, nullptr);
      }));
      
      for (const auto &arg3 : PrimitiveExpressions) {
        FuncDef threeArgFuncDef(FuncDef::Args({ &arg1->Type(), &arg2->Type(), &arg3->Type() }), FuncDef::NoArgs());
        ASSERT_NO_FATAL_FAILURE(TestArgs(threeArgFuncDef, 4, [&arg1, &arg2, &arg3](const Sexp &sexp) {
          return ThreeArgSuccessFn(sexp, arg1, arg2, arg3);
        }));
      }
    }
  }
}
