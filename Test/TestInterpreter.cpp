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

TEST(SymbolTable, TestPutSymbolNumber) {
  TestPutSymbolFn([](SymbolTable &table, const std::string &name, ExpressionPtr &val) {
    table.PutSymbolNumber(name, dynamic_cast<Number&>(*val).Value);
  }, ExpressionPtr { new Number(2) }, ExpressionPtr { new Number(6) });
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
  ExpressionPtr { new Quote(ExpressionPtr { new Number(123) }) }
  );
}

TEST(SymbolTable, TestPutSymbolFunction) {
  SymbolTable table;
  auto slispFn1 = [](Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) { return false; };
  auto slispFn2 = [](Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) { return true; };
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
  ExpressionPtr b, n, s;
  table.PutSymbol("b", ExpressionPtr { new Bool(true) });
  table.PutSymbol("n", ExpressionPtr { new Number(42) });
  table.PutSymbol("s", ExpressionPtr { new String("foo") });
  ASSERT_TRUE(table.GetSymbol("b", b));
  ASSERT_TRUE(table.GetSymbol("n", n));
  ASSERT_TRUE(table.GetSymbol("s", s));
  ASSERT_EQ(true, dynamic_cast<Bool&>(*b).Value);
  ASSERT_EQ(42, dynamic_cast<Number&>(*n).Value);
  ASSERT_EQ("foo", dynamic_cast<String&>(*s).Value);
  ASSERT_EQ(3, table.GetCount());
  
  //Overwrite tests
  table.PutSymbol("b", ExpressionPtr { new Bool(false) });
  table.PutSymbol("n", ExpressionPtr { new Number(123) });
  table.PutSymbol("s", ExpressionPtr { new String("hello, world!") });
  ASSERT_TRUE(table.GetSymbol("b", b));
  ASSERT_TRUE(table.GetSymbol("n", n));
  ASSERT_TRUE(table.GetSymbol("s", s));
  ASSERT_EQ(false, dynamic_cast<Bool&>(*b).Value);
  ASSERT_EQ(123, dynamic_cast<Number&>(*n).Value);
  ASSERT_EQ("hello, world!", dynamic_cast<String&>(*s).Value);
  ASSERT_EQ(3, table.GetCount());

  //Overwrite with empty
  table.PutSymbol("b", ExpressionPtr { });
  table.PutSymbol("n", ExpressionPtr { });
  table.PutSymbol("s", ExpressionPtr { });
  ASSERT_TRUE(table.GetSymbol("b", b));
  ASSERT_TRUE(table.GetSymbol("n", n));
  ASSERT_TRUE(table.GetSymbol("s", s));
  ASSERT_FALSE(b);
  ASSERT_FALSE(n);
  ASSERT_FALSE(s);
  ASSERT_EQ(3, table.GetCount());

  //Overwrite to original
  table.PutSymbol("b", ExpressionPtr { new Bool(true) });
  table.PutSymbol("n", ExpressionPtr { new Number(42) });
  table.PutSymbol("s", ExpressionPtr { new String("foo") });
  ASSERT_TRUE(table.GetSymbol("b", b));
  ASSERT_TRUE(table.GetSymbol("n", n));
  ASSERT_TRUE(table.GetSymbol("s", s));
  ASSERT_EQ(true, dynamic_cast<Bool&>(*b).Value);
  ASSERT_EQ(42, dynamic_cast<Number&>(*n).Value);
  ASSERT_EQ("foo", dynamic_cast<String&>(*s).Value);
  ASSERT_EQ(3, table.GetCount());
}

TEST(SymbolTable, TestDeleteSymbol) {
  SymbolTable table;
  ExpressionPtr val;
  ASSERT_FALSE(table.GetSymbol("val", val));
  table.DeleteSymbol("val");
  table.PutSymbolNumber("val", 42);
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

  table.PutSymbolNumber("num", 42);
  data.Reset();
  table.ForEach(fn);
  ASSERT_EQ(1, data.Count);
  ASSERT_EQ("num", data.LastName);
  ExpressionPtr fortyTwo { new Number(42) };
  ASSERT_EQ(*fortyTwo, *data.LastValue);

  table.DeleteSymbol("num");
  data.Reset();
  table.ForEach(fn);
  ASSERT_EQ(0, data.Count);
  ASSERT_EQ("", data.LastName);
  ASSERT_FALSE(data.LastValue);

  table.PutSymbolNumber("num", 42);
  table.PutSymbolString("str", "foo");
  table.PutSymbolNumber("num", 43);
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
  table.PutSymbolNumber("a", 1);
  {
    Scope outer(table);
    table.PutSymbolNumber("b", 2);
    outer.PutSymbol("c", ExpressionPtr { new Number(3) });
    ASSERT_TRUE(table.GetSymbol("a", temp));
    ASSERT_TRUE(table.GetSymbol("b", temp));
    ASSERT_TRUE(table.GetSymbol("c", temp));
    {
      Scope inner(table);
      table.PutSymbolNumber("d", 4);
      inner.PutSymbol("e", ExpressionPtr { new Number(5) });
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
  ExpressionPtr one { new Number(1) };
  ExpressionPtr temp;
  table.PutSymbol("a", one->Clone());
  {
    Scope outer(table);
    ExpressionPtr two { new Number(2) };
    outer.PutSymbol("a", two->Clone());
    ASSERT_TRUE(table.GetSymbol("a", temp));
    ASSERT_EQ(*two, *temp);
    {
      Scope inner(table);
      ExpressionPtr three { new Number(3) };
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
  ExpressionPtr one { new Number(1) };
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
    outer.PutDynamicSymbol("dynamic", ExpressionPtr { new Number(1) });
    outer.PutLocalSymbol("outer", ExpressionPtr { new Number(2) });
    ASSERT_TRUE(outer.GetSymbol("dynamic", temp));
    ASSERT_TRUE(outer.GetSymbol("outer", temp));
    ASSERT_EQ(1, outer.GetLocalSymbols().GetCount());
    {
      CompiledFunction innerFun;
      StackFrame inner(Interpreter_, innerFun);
      inner.PutDynamicSymbol("dynamic2", ExpressionPtr { new Number(3) });
      inner.PutLocalSymbol("inner", ExpressionPtr { new Number(4) });
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
    ExpressionPtr one { new Number(1) };
    ExpressionPtr two { new Number(2) };
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
  auto defaultSexp = Interpreter_.GetDefaultSexp();
  ASSERT_FALSE(defaultSexp.empty());
}

TEST_F(InterpreterTest, TestDefaultFunction) {
  auto slispFn = [](Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) { return true; };
  FuncDef def { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() };
  ExpressionPtr temp;
  ExpressionPtr func { new CompiledFunction(std::move(def), slispFn) };
  FunctionPtr defaultFn;
  ASSERT_FALSE(Interpreter_.GetDefaultFunction(defaultFn));
  ASSERT_FALSE(defaultFn);
  Interpreter_.PutDefaultFunction(std::move(dynamic_cast<CompiledFunction&>(*func->Clone())));
  ASSERT_TRUE(Interpreter_.GetDefaultFunction(defaultFn));
  ASSERT_TRUE(defaultFn);
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
    outer.PutDynamicSymbol("d1", ExpressionPtr { new Number(1) });
    outer.PutLocalSymbol("outer1", ExpressionPtr { new Number(2) });
    ASSERT_EQ(initialDynCount + 1, getCurrDynCount());
    ASSERT_EQ(initialLocalCount + 1, getCurrLocalCount());
    {
      StackFrame inner(Interpreter_, CompiledFunction());
      inner.PutDynamicSymbol("d2", ExpressionPtr { new Number(3) });
      inner.PutLocalSymbol("inner1", ExpressionPtr { new Number(4) });
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

    EvaluationTest():
      InterpreterTest()
    {
      EvaluationTest::Result = true;

      Interpreter_.PutDefaultFunction(CompiledFunction {
        FuncDef { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() },
        &DefaultFunction
      }); 
    }

    static bool DefaultFunction(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
      LastResult = std::move(expr);
      LastArgs.clear();
      ArgListHelper::CopyTo(args, LastArgs);
      interpreter.GetCommandInterface().WriteOutputLine(LastArgs.front()->ToString());
      return Result;
    }
};

ExpressionPtr EvaluationTest::LastResult;
ArgList EvaluationTest::LastArgs;
bool EvaluationTest::Result;

TEST_F(EvaluationTest, TestLiteral) {
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Bool(true) }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Number(42) }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new String("hello, world!") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Quote(ExpressionPtr { new Bool(true) })}));

  FunctionPtr defaultFn;
  Interpreter_.GetDefaultFunction(defaultFn);
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
  currStackFrame.PutLocalSymbol("n", ExpressionPtr { new Number(42) });
  currStackFrame.PutLocalSymbol("s", ExpressionPtr { new String("foo") });
  currStackFrame.PutLocalSymbol("f", ExpressionPtr { new CompiledFunction() });
  currStackFrame.PutLocalSymbol("q", ExpressionPtr { new Quote(ExpressionPtr { new Bool(true) })});

  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("b") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("n") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("s") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("f") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Symbol("q") }));
  ASSERT_EQ(Interpreter_.GetErrors().size(), 0);
}

TEST_F(EvaluationTest, TestSexpDefaultFunction) {
  ExpressionPtr trueValue { new Bool(true) },
                falseValue { new Bool(false) };
  Interpreter_.Evaluate(ExpressionPtr { new Sexp({
    ExpressionPtr { new Symbol(Interpreter_.GetDefaultSexp()) },
    falseValue->Clone()
  })});
  ASSERT_TRUE(CommandInterface.Output.find("0") != std::string::npos);
  ASSERT_EQ(*falseValue, *EvaluationTest::LastArgs.front());

  Interpreter_.Evaluate(ExpressionPtr { new Sexp({
    ExpressionPtr { new Symbol(Interpreter_.GetDefaultSexp()) },
    trueValue->Clone()
  })});
  ASSERT_TRUE(CommandInterface.Output.find("1") != std::string::npos);
  ASSERT_EQ(*trueValue, *EvaluationTest::LastArgs.front());
}

TEST_F(EvaluationTest, TestSexpCompiledFunction) {
// Make sure function is called
// Quote -> empty
// Quote -> Bool
// Quote -> Number
// Quote -> String
// Quote -> Function
// Quote -> Symbol -> undefined
// Quote -> Symbol -> Bool
// Quote -> Symbol -> Number
// Quote -> Symbol -> String
// Quote -> Symbol -> Function
// Quote -> Quote -> empty
// Quote -> Quote -> Bool
// Quote -> Quote -> Number
// Quote -> Quote -> String
// Quote -> Quote -> Function
// Quote -> Quote -> Symbol -> undefined
// Quote -> Quote -> Symbol -> Bool
// Quote -> Quote -> Symbol -> Number
// Quote -> Quote -> Symbol -> String
// Quote -> Quote -> Symbol -> Function
}

TEST_F(EvaluationTest, DISABLED_TestSexpInterpretedFunction) {
// Make sure function is called
}

TEST_F(EvaluationTest, DISABLED_TestSexpList) {
// ()
// (true 1 "string" +) - make sure everything shows up
}
