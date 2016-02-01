#include <functional>

#include "gtest\gtest.h"
#include "Interpreter.h"

#include "Common.h"

using PutFn = std::function<void(SymbolTable&, const std::string &name, ExpressionPtr &)>;
void TestPutSymbolFn(PutFn fn, ExpressionPtr &&val1, ExpressionPtr &&val2) {
  SymbolTable table;
  ExpressionPtr val1Expected = std::move(val1),
                val1Actual,
                val2Expected = std::move(val2),
                val2Actual;
  fn(table, "val1", val1Expected);
  fn(table, "val2", val2Expected);
  ASSERT_TRUE(table.GetSymbol("val1", val1Actual));
  ASSERT_TRUE(table.GetSymbol("val2", val2Actual));
  ASSERT_NE(*val1Actual, *val2Actual);
  ASSERT_EQ(*val1Expected, *val1Actual);
  ASSERT_EQ(*val2Expected, *val2Actual);
  
  fn(table, "val2", val1Expected);
  ASSERT_EQ(*val2Expected, *val2Actual);
  ASSERT_TRUE(table.GetSymbol("val2", val2Actual));
  ASSERT_EQ(*val1Actual, *val2Actual);
}

TEST(SymbolTable, TestPutSymbolBool) {
  TestPutSymbolFn([](SymbolTable &table, const std::string &name, ExpressionPtr &val) {
    table.PutSymbolBool(name, dynamic_cast<Bool&>(*val).Value);
  }, ExpressionPtr { new Bool(true) }, ExpressionPtr { new Bool(false) });
}

TEST(SymbolTable, TestPutSymbolInt) {
  TestPutSymbolFn([](SymbolTable &table, const std::string &name, ExpressionPtr &val) {
    table.PutSymbolInt(name, dynamic_cast<Int&>(*val).Value);
  }, ExpressionPtr { new Int(2) }, ExpressionPtr { new Int(6) });
}

TEST(SymbolTable, TestPutSymbolFloat) {
  TestPutSymbolFn([](SymbolTable &table, const std::string &name, ExpressionPtr &val) {
    table.PutSymbolFloat(name, dynamic_cast<Float&>(*val).Value);
  }, ExpressionPtr { new Float(3.14) }, ExpressionPtr { new Float(1.5) });
}

TEST(SymbolTable, TestPutSymbolString) {
  TestPutSymbolFn([](SymbolTable &table, const std::string &name, ExpressionPtr &val) {
    table.PutSymbolString(name, dynamic_cast<String&>(*val).Value);
  }, ExpressionPtr { new String("foo") }, ExpressionPtr { new String("bar") });
}

TEST(SymbolTable, TestPutSymbolQuote) {
  TestPutSymbolFn([](SymbolTable &table, const std::string &name, ExpressionPtr &val) {
    table.PutSymbolQuote(name, dynamic_cast<Quote&>(*val).Value->Clone());
  },
  ExpressionPtr { new Quote(ExpressionPtr { new String("foo") }) },
  ExpressionPtr { new Quote(ExpressionPtr { new Int(123) }) }
  );
}

TEST(SymbolTable, TestPutSymbolFunction) {
  SymbolTable table;
  auto slispFn1 = [](EvaluationContext&) { return false; };
  auto slispFn2 = [](EvaluationContext&) { return true; };
  FuncDef def1  { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() };
  FuncDef def2  { FuncDef::OneArg(String::TypeInstance), FuncDef::NoArgs() };
  ExpressionPtr temp;
  ExpressionPtr f1 { new CompiledFunction(std::move(def1), slispFn1) };
  ExpressionPtr f2 { new CompiledFunction(std::move(def2), slispFn2) };
  table.PutSymbolFunction("f1", std::move(dynamic_cast<CompiledFunction&>(*f1->Clone())));
  table.PutSymbolFunction("f2", std::move(dynamic_cast<CompiledFunction&>(*f2->Clone())));
  table.PutSymbolFunction("f3", slispFn2, def2.Clone());
  ASSERT_TRUE(table.GetSymbol("f1", temp));
  ASSERT_TRUE(table.GetSymbol("f2", temp));
  ASSERT_TRUE(table.GetSymbol("f3", temp));
}

TEST(SymbolTable, TestPutSymbol_Empty) {
  SymbolTable table;
  ExpressionPtr empty;
  table.PutSymbol("empty", ExpressionPtr {});
  ASSERT_TRUE(table.GetSymbol("empty", empty));
  ASSERT_FALSE(empty);
}

TEST(SymbolTable, TestPutSymbol) {
  SymbolTable table;
  ExpressionPtr b, i, f, s;
  table.PutSymbol("b", ExpressionPtr { new Bool(true) });
  table.PutSymbol("i", ExpressionPtr { new Int(42) });
  table.PutSymbol("f", ExpressionPtr { new Float(3.14) });
  table.PutSymbol("s", ExpressionPtr { new String("foo") });
  ASSERT_TRUE(table.GetSymbol("b", b));
  ASSERT_TRUE(table.GetSymbol("i", i));
  ASSERT_TRUE(table.GetSymbol("f", f));
  ASSERT_TRUE(table.GetSymbol("s", s));
  ASSERT_EQ(true, dynamic_cast<Bool&>(*b).Value);
  ASSERT_EQ(42, dynamic_cast<Int&>(*i).Value);
  ASSERT_EQ(3.14, dynamic_cast<Float&>(*f).Value);
  ASSERT_EQ("foo", dynamic_cast<String&>(*s).Value);
  ASSERT_EQ(4, table.GetCount());
  
  //Overwrite tests
  table.PutSymbol("b", ExpressionPtr { new Bool(false) });
  table.PutSymbol("i", ExpressionPtr { new Int(123) });
  table.PutSymbol("f", ExpressionPtr { new Float(1.23) });
  table.PutSymbol("s", ExpressionPtr { new String("hello, world!") });
  ASSERT_TRUE(table.GetSymbol("b", b));
  ASSERT_TRUE(table.GetSymbol("i", i));
  ASSERT_TRUE(table.GetSymbol("f", f));
  ASSERT_TRUE(table.GetSymbol("s", s));
  ASSERT_EQ(false, dynamic_cast<Bool&>(*b).Value);
  ASSERT_EQ(123, dynamic_cast<Int&>(*i).Value);
  ASSERT_EQ(1.23, dynamic_cast<Float&>(*f).Value);
  ASSERT_EQ("hello, world!", dynamic_cast<String&>(*s).Value);
  ASSERT_EQ(4, table.GetCount());

  //Overwrite with empty
  table.PutSymbol("b", ExpressionPtr { });
  table.PutSymbol("i", ExpressionPtr { });
  table.PutSymbol("f", ExpressionPtr { });
  table.PutSymbol("s", ExpressionPtr { });
  ASSERT_TRUE(table.GetSymbol("b", b));
  ASSERT_TRUE(table.GetSymbol("i", i));
  ASSERT_TRUE(table.GetSymbol("f", f));
  ASSERT_TRUE(table.GetSymbol("s", s));
  ASSERT_FALSE(b);
  ASSERT_FALSE(i);
  ASSERT_FALSE(f);
  ASSERT_FALSE(s);
  ASSERT_EQ(4, table.GetCount());

  //Overwrite to original
  table.PutSymbol("b", ExpressionPtr { new Bool(true) });
  table.PutSymbol("i", ExpressionPtr { new Int(42) });
  table.PutSymbol("f", ExpressionPtr { new Float(3.14) });
  table.PutSymbol("s", ExpressionPtr { new String("foo") });
  ASSERT_TRUE(table.GetSymbol("b", b));
  ASSERT_TRUE(table.GetSymbol("i", i));
  ASSERT_TRUE(table.GetSymbol("f", f));
  ASSERT_TRUE(table.GetSymbol("s", s));
  ASSERT_EQ(true, dynamic_cast<Bool&>(*b).Value);
  ASSERT_EQ(42, dynamic_cast<Int&>(*i).Value);
  ASSERT_EQ(3.14, dynamic_cast<Float&>(*f).Value);
  ASSERT_EQ("foo", dynamic_cast<String&>(*s).Value);
  ASSERT_EQ(4, table.GetCount());
}

TEST(SymbolTable, TestDeleteSymbol) {
  SymbolTable table;
  ExpressionPtr val;
  ASSERT_FALSE(table.GetSymbol("val", val));
  table.DeleteSymbol("val");
  table.PutSymbolInt("val", 42);
  ASSERT_TRUE(table.GetSymbol("val", val));
  ASSERT_EQ(1, table.GetCount());
  table.DeleteSymbol("val");
  ASSERT_FALSE(table.GetSymbol("val", val));
  ASSERT_EQ(0, table.GetCount());
}

struct ForEachData {
  std::string LastName;
  ExpressionPtr LastValue;
  int Count;
  
  void Reset() {
    LastName = "";
    LastValue.reset();
    Count = 0;
  }
};

TEST(SymbolTable, TestForEach) {
  SymbolTable table;
  ForEachData data;
  auto fn = [&data](const std::string &name, ExpressionPtr &val) {
    data.LastName = name;
    if (val)
      data.LastValue = val->Clone();
    ++data.Count;
  };

  data.Reset();
  table.ForEach(fn);
  ASSERT_EQ(0, data.Count);
  ASSERT_EQ("", data.LastName);
  ASSERT_FALSE(data.LastValue);

  table.PutSymbolInt("num", 42);
  data.Reset();
  table.ForEach(fn);
  ASSERT_EQ(1, data.Count);
  ASSERT_EQ("num", data.LastName);
  ExpressionPtr fortyTwo { new Int(42) };
  ASSERT_EQ(*fortyTwo, *data.LastValue);

  table.DeleteSymbol("num");
  data.Reset();
  table.ForEach(fn);
  ASSERT_EQ(0, data.Count);
  ASSERT_EQ("", data.LastName);
  ASSERT_FALSE(data.LastValue);

  table.PutSymbolInt("num", 42);
  table.PutSymbolString("str", "foo");
  table.PutSymbolInt("num", 43);
  table.PutSymbolBool("b", false);
  data.Reset();
  table.ForEach(fn);
  ASSERT_EQ(3, data.Count);

  table.DeleteSymbol("num");
  table.DeleteSymbol("str");
  table.DeleteSymbol("b");
  data.Reset();
  table.ForEach(fn);
  ASSERT_EQ(0, data.Count);
  ASSERT_EQ("", data.LastName);
  ASSERT_FALSE(data.LastValue);

  data.Reset();
  table.PutSymbol("empty", ExpressionPtr {});
  table.ForEach(fn);
  ASSERT_EQ(1, data.Count);
  ASSERT_EQ("empty", data.LastName);
  ASSERT_FALSE(data.LastValue);
}

TEST(Scope, TestPutSymbol_Simple) {
  SymbolTable table;
  ExpressionPtr temp;
  table.PutSymbolInt("a", 1);
  {
    Scope outer(table);
    table.PutSymbolInt("b", 2);
    outer.PutSymbol("c", ExpressionPtr { new Int(3) });
    outer.PutSymbol("c", ExpressionPtr { new Int(2) }); // #20
    
    ASSERT_FALSE(outer.IsScopedSymbol("a"));
    ASSERT_FALSE(outer.IsScopedSymbol("b"));
    ASSERT_TRUE(outer.IsScopedSymbol("c"));

    ASSERT_TRUE(table.GetSymbol("a", temp));
    ASSERT_TRUE(table.GetSymbol("b", temp));
    ASSERT_TRUE(table.GetSymbol("c", temp));
    {
      Scope inner(table);
      table.PutSymbolInt("d", 4);
      inner.PutSymbol("e", ExpressionPtr { new Int(5) });

      ASSERT_FALSE(inner.IsScopedSymbol("a"));
      ASSERT_FALSE(inner.IsScopedSymbol("b"));
      ASSERT_FALSE(inner.IsScopedSymbol("c"));
      ASSERT_FALSE(inner.IsScopedSymbol("d"));
      ASSERT_TRUE(inner.IsScopedSymbol("e"));

      ASSERT_TRUE(table.GetSymbol("a", temp));
      ASSERT_TRUE(table.GetSymbol("b", temp));
      ASSERT_TRUE(table.GetSymbol("c", temp));
      ASSERT_TRUE(table.GetSymbol("d", temp));
      ASSERT_TRUE(table.GetSymbol("e", temp));
    }
    ASSERT_TRUE(table.GetSymbol("a", temp));
    ASSERT_TRUE(table.GetSymbol("b", temp));
    ASSERT_TRUE(table.GetSymbol("c", temp));
    ASSERT_TRUE(table.GetSymbol("d", temp));
    ASSERT_FALSE(table.GetSymbol("e", temp));
  }
  ASSERT_TRUE(table.GetSymbol("a", temp));
  ASSERT_TRUE(table.GetSymbol("b", temp));
  ASSERT_FALSE(table.GetSymbol("c", temp));
  ASSERT_TRUE(table.GetSymbol("d", temp));
  ASSERT_FALSE(table.GetSymbol("e", temp));
}

TEST(Scope, TestPutSymbol_ShadowingSameType) {
  SymbolTable table;
  ExpressionPtr one { new Int(1) };
  ExpressionPtr temp;
  table.PutSymbol("a", one->Clone());
  {
    Scope outer(table);
    ExpressionPtr two { new Int(2) };
    outer.PutSymbol("a", two->Clone());
    ASSERT_TRUE(table.GetSymbol("a", temp));
    ASSERT_EQ(*two, *temp);
    {
      Scope inner(table);
      ExpressionPtr three { new Int(3) };
      inner.PutSymbol("a", three->Clone());
      ASSERT_TRUE(table.GetSymbol("a", temp));
      ASSERT_EQ(*three, *temp);
    }
    ASSERT_TRUE(table.GetSymbol("a", temp));
    ASSERT_EQ(*two, *temp);
  }
  ASSERT_TRUE(table.GetSymbol("a", temp));
  ASSERT_EQ(*one, *temp);
}

TEST(Scope, TestPutSymbol_ShadowingDifferentType) {
  SymbolTable table;
  ExpressionPtr one { new Int(1) };
  ExpressionPtr temp;
  table.PutSymbol("a", one->Clone());
  {
    Scope outer(table);
    ExpressionPtr tru { new Bool(true) };
    outer.PutSymbol("a", tru->Clone());
    ASSERT_TRUE(table.GetSymbol("a", temp));
    ASSERT_EQ(*tru, *temp);
    {
      Scope inner(table);
      ExpressionPtr hello { new String("hello") };
      inner.PutSymbol("a", hello->Clone());
      ASSERT_TRUE(table.GetSymbol("a", temp));
      ASSERT_EQ(*hello, *temp);
    }
    ASSERT_TRUE(table.GetSymbol("a", temp));
    ASSERT_EQ(*tru, *temp);
  }
  ASSERT_TRUE(table.GetSymbol("a", temp));
  ASSERT_EQ(*one, *temp);
}

class InterpreterTest: public ::testing::Test {
  public:
    InterpreterTest():
      CommandInterface(),
      Interpreter_(CommandInterface)
    {
    }
  
  protected:
    TestCommandInterface CommandInterface;
    Interpreter          Interpreter_;
};

class StackFrameTest: public InterpreterTest {
};

TEST_F(StackFrameTest, TestSimple) {
  ExpressionPtr temp;
  {
    CompiledFunction outerFunc;
    StackFrame outer(Interpreter_, outerFunc);
    outer.PutDynamicSymbol("dynamic", ExpressionPtr { new Int(1) });
    outer.PutLocalSymbol("outer", ExpressionPtr { new Int(2) });
    ASSERT_TRUE(outer.GetSymbol("dynamic", temp));
    ASSERT_TRUE(outer.GetSymbol("outer", temp));
    ASSERT_EQ(1, outer.GetLocalSymbols().GetCount());
    {
      CompiledFunction innerFun;
      StackFrame inner(Interpreter_, innerFun);
      inner.PutDynamicSymbol("dynamic2", ExpressionPtr { new Int(3) });
      inner.PutLocalSymbol("inner", ExpressionPtr { new Int(4) });
      ASSERT_TRUE(inner.GetSymbol("dynamic", temp));
      ASSERT_FALSE(inner.GetSymbol("outer", temp));
      ASSERT_TRUE(inner.GetSymbol("dynamic2", temp));
      ASSERT_TRUE(inner.GetSymbol("inner", temp));
      ASSERT_EQ(1, inner.GetLocalSymbols().GetCount());
    }
    ASSERT_TRUE(outer.GetSymbol("dynamic", temp));
    ASSERT_TRUE(outer.GetSymbol("outer", temp));
    ASSERT_FALSE(outer.GetSymbol("dynamic2", temp));
    ASSERT_FALSE(outer.GetSymbol("inner", temp));
    ASSERT_EQ(1, outer.GetLocalSymbols().GetCount());
  }
}

TEST_F(StackFrameTest, TestShadowing) {
  ExpressionPtr temp;
  {
    CompiledFunction outerFunc;
    StackFrame outer(Interpreter_, outerFunc);
    ExpressionPtr one { new Int(1) };
    ExpressionPtr two { new Int(2) };
    outer.PutDynamicSymbol("dynamic", one->Clone());
    outer.PutLocalSymbol("local", two->Clone());
    ASSERT_TRUE(outer.GetSymbol("dynamic", temp));
    ASSERT_EQ(*one, *temp);
    ASSERT_TRUE(outer.GetSymbol("local", temp));
    ASSERT_EQ(*two, *temp);
    ASSERT_EQ(1, outer.GetLocalSymbols().GetCount());
    {
      CompiledFunction innerFun;
      StackFrame inner(Interpreter_, innerFun);
      ExpressionPtr foo { new String("foo") };
      ExpressionPtr bar { new String("bar") };
      inner.PutDynamicSymbol("dynamic", foo->Clone());
      inner.PutLocalSymbol("local", bar->Clone());
      ASSERT_TRUE(inner.GetSymbol("dynamic", temp));
      ASSERT_EQ(*foo, *temp);
      ASSERT_TRUE(inner.GetSymbol("local", temp));
      ASSERT_EQ(*bar, *temp);
      ASSERT_EQ(1, inner.GetLocalSymbols().GetCount());
    }
    ASSERT_TRUE(outer.GetSymbol("dynamic", temp));
    ASSERT_EQ(*one, *temp);
    ASSERT_TRUE(outer.GetSymbol("local", temp));
    ASSERT_EQ(*two, *temp);
    ASSERT_EQ(1, outer.GetLocalSymbols().GetCount());
  }
}

TEST_F(InterpreterTest, TestDefaultSexp) {
  auto defaultSexp = Interpreter_.GetSettings().GetDefaultSexp();
  ASSERT_FALSE(defaultSexp.empty());
}

TEST_F(InterpreterTest, TestDefaultFunction) {
  auto slispFn = [](EvaluationContext&) { return true; };
  FuncDef def { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() };
  ExpressionPtr temp;
  ExpressionPtr func { new CompiledFunction(std::move(def), slispFn) };
  FunctionPtr defaultFn;
  auto &settings = Interpreter_.GetSettings();
  ASSERT_FALSE(settings.GetDefaultFunction(defaultFn));
  ASSERT_FALSE(defaultFn);
  settings.PutDefaultFunction(std::move(dynamic_cast<CompiledFunction&>(*func->Clone())));
  ASSERT_TRUE(settings.GetDefaultFunction(defaultFn));
  ASSERT_TRUE((bool)defaultFn);
}

TEST_F(InterpreterTest, TestErrors_Single) {
  ASSERT_EQ(0, Interpreter_.GetErrors().size());
  ASSERT_FALSE(Interpreter_.PushError(EvalError { "here1", "err1" }));
  auto errors = Interpreter_.GetErrors();
  ASSERT_EQ(1, errors.size());
  ASSERT_EQ("here1", errors.front().Where);
  ASSERT_EQ("err1", errors.front().What);
  Interpreter_.ClearErrors();
  ASSERT_EQ(0, Interpreter_.GetErrors().size());
}

TEST_F(InterpreterTest, TestErrors_Many) {
  ASSERT_FALSE(Interpreter_.PushError(EvalError { "here1", "err1" }));
  ASSERT_FALSE(Interpreter_.PushError(EvalError { "here2", "err2" }));
  ASSERT_FALSE(Interpreter_.PushError(EvalError { "here3", "err3" }));
  auto errors = Interpreter_.GetErrors();
  int errNum = 1;
  for (auto &error : errors) {
    ASSERT_EQ("here" + std::to_string(errNum), error.Where);
    ASSERT_EQ("err" + std::to_string(errNum), error.What);
    ++errNum;
  }
  Interpreter_.ClearErrors();
  ASSERT_EQ(0, Interpreter_.GetErrors().size());
}

TEST_F(InterpreterTest, TestStop) {
  ASSERT_FALSE(Interpreter_.StopRequested());
  Interpreter_.Stop();
  ASSERT_TRUE(Interpreter_.StopRequested());
  Interpreter_.Stop();
  ASSERT_TRUE(Interpreter_.StopRequested());
}

TEST_F(InterpreterTest, TestStackFrames) {
  size_t initialDynCount = 0,
         initialLocalCount   = 0;
  auto getCurrDynCount = [this, &initialDynCount]() {
    return initialDynCount + Interpreter_.GetDynamicSymbols().GetCount();
  };
  auto getCurrLocalCount = [this, &initialLocalCount]() {
    return initialLocalCount + Interpreter_.GetCurrentStackFrame().GetLocalSymbols().GetCount();
  };
  initialDynCount = getCurrDynCount();
  initialLocalCount = getCurrLocalCount();
  {
    StackFrame outer(Interpreter_, CompiledFunction());
    outer.PutDynamicSymbol("d1", ExpressionPtr { new Int(1) });
    outer.PutLocalSymbol("outer1", ExpressionPtr { new Int(2) });
    ASSERT_EQ(initialDynCount + 1, getCurrDynCount());
    ASSERT_EQ(initialLocalCount + 1, getCurrLocalCount());
    {
      StackFrame inner(Interpreter_, CompiledFunction());
      inner.PutDynamicSymbol("d2", ExpressionPtr { new Int(3) });
      inner.PutLocalSymbol("inner1", ExpressionPtr { new Int(4) });
      ASSERT_EQ(initialDynCount + 2, getCurrDynCount());
      ASSERT_EQ(initialLocalCount + 1, getCurrLocalCount());
    }
    ASSERT_EQ(initialDynCount + 1, getCurrDynCount());
    ASSERT_EQ(initialLocalCount + 1, getCurrLocalCount());
  }
  ASSERT_EQ(initialDynCount, getCurrDynCount());
  ASSERT_EQ(initialLocalCount, getCurrLocalCount());
}

class EvaluationTest: public InterpreterTest {
  protected:
    static ExpressionPtr LastResult;
    static ArgList       LastArgs;
    static bool          Result;
    static bool          MyFuncCalled;
    static bool          MyListFuncCalled;
    static int           MyListLastArgCount;

    EvaluationTest():
      InterpreterTest()
    {
      EvaluationTest::Result = true;
      EvaluationTest::MyFuncCalled = false;
      EvaluationTest::MyListFuncCalled = false;

      auto &settings = Interpreter_.GetSettings();

      settings.PutDefaultFunction(CompiledFunction {
        FuncDef { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() },
        &DefaultFunction
      }); 

      Interpreter_.GetDynamicSymbols().PutSymbolFunction("myCompiledFunc", CompiledFunction {
        FuncDef { FuncDef::OneArg(Bool::TypeInstance), FuncDef::NoArgs() },
        &MyCompiledFunc
      });

      ExpressionPtr trueValue { new Bool(true) };
      ExpressionPtr code {
        new Sexp({
          ExpressionPtr { new Symbol("myCompiledFunc") },
          ExpressionPtr { new Symbol("x") }
        })
      };
      ArgList args;
      args.push_back(ExpressionPtr { new Symbol("x") });
      Interpreter_.GetDynamicSymbols().PutSymbolFunction("myInterpretedFunc", InterpretedFunction {
        FuncDef { FuncDef::OneArg(Bool::TypeInstance), FuncDef::NoArgs() },
        std::move(code),
        std::move(args)
       });


      ExpressionPtr badFnCode {
        new Symbol("undefinedSymbol")
      };
      ArgList badFnArgs;
      Interpreter_.GetDynamicSymbols().PutSymbolFunction("myInterpretedBadFunc", InterpretedFunction {
        FuncDef { FuncDef::NoArgs(), FuncDef::NoArgs() },
        std::move(badFnCode),
        std::move(badFnArgs)
       });
    }

    void TestSexpFunction(const std::string &funcName);
    void TestBasicArg(const std::string &funcName);
    void TestWrongTypeArg(const std::string &funcName);
    void TestSymbolArg(const std::string &funcName);
    void TestNotEnoughArgs(const std::string &funcName);
    void TestTooManyArgs(const std::string &funcName);
    
    static bool DefaultFunction(EvaluationContext &ctx) {
      LastResult = std::move(ctx.Expr);
      LastArgs.clear();
      ArgListHelper::CopyTo(ctx.Args, LastArgs);
      ctx.Interp.GetCommandInterface().WriteOutputLine(LastArgs.front()->ToString());
      return Result;
    }

    static bool MyCompiledFunc(EvaluationContext &ctx) {
      MyFuncCalled = true;
      return Result;
    }

    static bool MyListFunc(EvaluationContext &ctx) {
      MyListFuncCalled = true;
      MyListLastArgCount = 0;
      for (auto &arg : ctx.Args)
        ++MyListLastArgCount;
      return Result;
    }
};

ExpressionPtr EvaluationTest::LastResult;
ArgList EvaluationTest::LastArgs;
bool EvaluationTest::Result;
bool EvaluationTest::MyFuncCalled;
bool EvaluationTest::MyListFuncCalled;
int EvaluationTest::MyListLastArgCount;

TEST_F(EvaluationTest, TestLiteral) {
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Bool(true) }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Int(42) }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Float(3.14) }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new String("hello, world!") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Quote(ExpressionPtr { new Bool(true) })}));

  FunctionPtr defaultFn;
  ASSERT_TRUE(Interpreter_.GetSettings().GetDefaultFunction(defaultFn));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { defaultFn.release() }));

  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new CompiledFunction() }));
  
  ExpressionPtr code {};
  ArgList args {};
  ExpressionPtr e { new InterpretedFunction(
    FuncDef { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() },
    std::move(code),
    std::move(args)
  ) };
  ASSERT_TRUE(Interpreter_.Evaluate(e));
  ASSERT_EQ(0, Interpreter_.GetErrors().size());
}

TEST_F(EvaluationTest, TestSymbol) {
  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("b") }));
  ASSERT_GT(Interpreter_.GetErrors().size(), 0U);

  auto &currStackFrame = Interpreter_.GetCurrentStackFrame();
  currStackFrame.PutLocalSymbol("b", ExpressionPtr { new Bool(true) });
  currStackFrame.PutLocalSymbol("i", ExpressionPtr { new Int(42) });
  currStackFrame.PutLocalSymbol("f", ExpressionPtr { new Float(3.14) });
  currStackFrame.PutLocalSymbol("s", ExpressionPtr { new String("foo") });
  currStackFrame.PutLocalSymbol("fn", ExpressionPtr { new CompiledFunction() });
  currStackFrame.PutLocalSymbol("q", ExpressionPtr { new Quote(ExpressionPtr { new Bool(true) })});

  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("b") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("i") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("f") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("s") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("fn") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("q") }));
  ASSERT_EQ(Interpreter_.GetErrors().size(), 0);
}

TEST_F(EvaluationTest, TestSexpDefaultFunction) {
  ExpressionPtr trueValue { new Bool(true) },
                falseValue { new Bool(false) };
  auto &settings = Interpreter_.GetSettings();
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Sexp({
    ExpressionPtr { new Symbol(settings.GetDefaultSexp()) },
    falseValue->Clone()
  })}));
  ASSERT_TRUE(CommandInterface.Output.find("false") != std::string::npos);
  ASSERT_EQ(*falseValue, *EvaluationTest::LastArgs.front());

  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Sexp({
    ExpressionPtr { new Symbol(settings.GetDefaultSexp()) },
    trueValue->Clone()
  })}));
  ASSERT_TRUE(CommandInterface.Output.find("true") != std::string::npos);
  ASSERT_EQ(*trueValue, *EvaluationTest::LastArgs.front());
}

void EvaluationTest::TestBasicArg(const std::string &funcName) {
  ExpressionPtr trueValue { new Bool(true) };
 
  ExpressionPtr sym { new Symbol(funcName) };
  ExpressionPtr sexp { new Sexp({std::move(sym), std::move(trueValue)}) };

  EvaluationTest::MyFuncCalled = false;
  ASSERT_TRUE(Interpreter_.Evaluate(sexp));
  ASSERT_TRUE(EvaluationTest::MyFuncCalled);
}

void EvaluationTest::TestSymbolArg(const std::string &funcName) {
  ExpressionPtr sym { new Symbol("sym") };
  EvaluationTest::MyFuncCalled = false;
  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Sexp({
    ExpressionPtr { new Symbol(funcName) },
    sym->Clone()
  })}));
  ASSERT_FALSE(EvaluationTest::MyFuncCalled);

  Interpreter_.GetCurrentStackFrame().PutLocalSymbol("sym", ExpressionPtr { new Bool(false) });
  EvaluationTest::MyFuncCalled = false;
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Sexp({
    ExpressionPtr { new Symbol(funcName) },
    sym->Clone()
  })}));
  ASSERT_TRUE(EvaluationTest::MyFuncCalled);
}

void EvaluationTest::TestNotEnoughArgs(const std::string &funcName) {
  EvaluationTest::MyFuncCalled = false;
  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Sexp({
    ExpressionPtr { new Symbol(funcName) },
  })}));
  ASSERT_FALSE(EvaluationTest::MyFuncCalled);
}

void EvaluationTest::TestTooManyArgs(const std::string &funcName) {
  ExpressionPtr trueValue { new Bool(true) };
 
  EvaluationTest::MyFuncCalled = false;
  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Sexp({
    ExpressionPtr { new Symbol(funcName) },
    trueValue->Clone(),
    trueValue->Clone()
  })}));
  ASSERT_FALSE(EvaluationTest::MyFuncCalled);
}

void EvaluationTest::TestWrongTypeArg(const std::string &funcName) {
  ExpressionPtr str { new String("foo") };
  EvaluationTest::MyFuncCalled = false;
  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Sexp({
    ExpressionPtr { new Symbol(funcName) },
    str->Clone()
  })}));
  ASSERT_FALSE(EvaluationTest::MyFuncCalled);
}

void EvaluationTest::TestSexpFunction(const std::string &funcName) {
  ASSERT_NO_FATAL_FAILURE(TestBasicArg(funcName));
  ASSERT_NO_FATAL_FAILURE(TestWrongTypeArg(funcName));
  ASSERT_NO_FATAL_FAILURE(TestSymbolArg(funcName));
  ASSERT_NO_FATAL_FAILURE(TestNotEnoughArgs(funcName));
  ASSERT_NO_FATAL_FAILURE(TestTooManyArgs(funcName));
}

TEST_F(EvaluationTest, TestSexpCompiledFunction) {
  ASSERT_NO_FATAL_FAILURE(TestSexpFunction("myCompiledFunc"));
}

TEST_F(EvaluationTest, TestSexpInterpretedFunction) {
  ASSERT_NO_FATAL_FAILURE(TestSexpFunction("myInterpretedFunc"));

  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Sexp({
    ExpressionPtr { new Symbol("myInterpretedBadFunc") },
  })}));
}

TEST_F(EvaluationTest, TestSexpList) {
  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Sexp() }));
  ASSERT_FALSE(EvaluationTest::MyListFuncCalled);

  Interpreter_.GetSettings().PutListFunction(CompiledFunction {
    FuncDef { FuncDef::AnyArgs(), FuncDef::OneArg(Quote::TypeInstance) },
    &MyListFunc
  });

  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Sexp() }));
  ASSERT_TRUE(EvaluationTest::MyListFuncCalled);
  ASSERT_EQ(0, MyListLastArgCount);

  ExpressionPtr boolValue { new Bool(true) },
                intValue { new Int(42) },
                floatValue { new Float(3.14) },
                strValue { new String("foo") },
                symValue { new Symbol("myCompiledFunc") };
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Sexp({
    boolValue->Clone(),
    intValue->Clone(),
    floatValue->Clone(),
    strValue->Clone(),
    symValue->Clone()
  }) }));
  ASSERT_EQ(5, MyListLastArgCount);
}
