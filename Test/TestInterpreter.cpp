#include <functional>

#include "gtest/gtest.h"
#include "Interpreter.h"

#include "Common.h"
#include "BaseTest.h"

using namespace std;

class SymbolTableTest: public BaseTest {
};

using PutFn = function<void(SymbolTable&, const string &name, ExpressionPtr &)>;
void TestPutSymbolFn(PutFn fn, ExpressionPtr &&val1, ExpressionPtr &&val2) {
  SymbolTableType store;
  SymbolTable table(store, NullSourceContext);
  ExpressionPtr val1Expected = move(val1),
                val1Actual,
                val2Expected = move(val2),
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

TEST_F(SymbolTableTest, TestPutSymbolBool) {
  TestPutSymbolFn([](SymbolTable &table, const string &name, ExpressionPtr &val) {
    table.PutSymbolBool(name, dynamic_cast<Bool&>(*val).Value);
  }, ExpressionPtr { Factory.Alloc<Bool>(true) }, ExpressionPtr { Factory.Alloc<Bool>(false) });
}

TEST_F(SymbolTableTest, TestPutSymbolInt) {
  TestPutSymbolFn([](SymbolTable &table, const string &name, ExpressionPtr &val) {
    table.PutSymbolInt(name, dynamic_cast<Int&>(*val).Value);
  }, ExpressionPtr { Factory.Alloc<Int>(2) }, ExpressionPtr { Factory.Alloc<Int>(6) });
}

TEST_F(SymbolTableTest, TestPutSymbolFloat) {
  TestPutSymbolFn([](SymbolTable &table, const string &name, ExpressionPtr &val) {
    table.PutSymbolFloat(name, dynamic_cast<Float&>(*val).Value);
  }, ExpressionPtr { Factory.Alloc<Float>(3.14) }, ExpressionPtr { Factory.Alloc<Float>(1.5) });
}

TEST_F(SymbolTableTest, TestPutSymbolStr) {
  TestPutSymbolFn([](SymbolTable &table, const string &name, ExpressionPtr &val) {
    table.PutSymbolStr(name, dynamic_cast<Str&>(*val).Value);
  }, ExpressionPtr { Factory.Alloc<Str>("foo") }, ExpressionPtr { Factory.Alloc<Str>("bar") });
}

TEST_F(SymbolTableTest, TestPutSymbolQuote) {
  TestPutSymbolFn([](SymbolTable &table, const string &name, ExpressionPtr &val) {
    table.PutSymbolQuote(name, dynamic_cast<Quote&>(*val).Value->Clone());
  },
  ExpressionPtr { Factory.Alloc<Quote>(ExpressionPtr { Factory.Alloc<Str>("foo") }) },
  ExpressionPtr { Factory.Alloc<Quote>(ExpressionPtr { Factory.Alloc<Int>(123) }) }
  );
}

TEST_F(SymbolTableTest, TestPutSymbolFunction) {
  SymbolTableType store;
  SymbolTable table(store, NullSourceContext);
  auto slispFn1 = [](EvaluationContext&) { return false; };
  auto slispFn2 = [](EvaluationContext&) { return true; };
  FuncDef def1  { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() };
  FuncDef def2  { FuncDef::OneArg(Str::TypeInstance), FuncDef::NoArgs() };
  ExpressionPtr temp;
  ExpressionPtr f1 { Factory.Alloc<CompiledFunction>(move(def1), slispFn1) };
  ExpressionPtr f2 { Factory.Alloc<CompiledFunction>(move(def2), slispFn2) };
  table.PutSymbolFunction("f1", move(dynamic_cast<CompiledFunction&>(*f1->Clone())));
  table.PutSymbolFunction("f2", move(dynamic_cast<CompiledFunction&>(*f2->Clone())));
  table.PutSymbolFunction(
    "f3", 
    {"(f3 string) -> nil"},
    "no help",
    {},
    slispFn2, 
    def2.Clone()
  );
  ASSERT_TRUE(table.GetSymbol("f1", temp));
  ASSERT_TRUE(table.GetSymbol("f2", temp));
  ASSERT_TRUE(table.GetSymbol("f3", temp));
}

TEST_F(SymbolTableTest, TestPutSymbol_Empty) {
  SymbolTableType store;
  SymbolTable table(store, NullSourceContext);
  ExpressionPtr empty;
  table.PutSymbol("empty", ExpressionPtr {});
  ASSERT_TRUE(table.GetSymbol("empty", empty));
  ASSERT_FALSE(empty);
}

TEST_F(SymbolTableTest, TestPutSymbol) {
  SymbolTableType store;
  SymbolTable table(store, NullSourceContext);
  ExpressionPtr b, i, f, s;
  table.PutSymbol("b", ExpressionPtr { Factory.Alloc<Bool>(true) });
  table.PutSymbol("i", ExpressionPtr { Factory.Alloc<Int>(42) });
  table.PutSymbol("f", ExpressionPtr { Factory.Alloc<Float>(3.14) });
  table.PutSymbol("s", ExpressionPtr { Factory.Alloc<Str>("foo") });
  ASSERT_TRUE(table.GetSymbol("b", b));
  ASSERT_TRUE(table.GetSymbol("i", i));
  ASSERT_TRUE(table.GetSymbol("f", f));
  ASSERT_TRUE(table.GetSymbol("s", s));
  ASSERT_EQ(true, dynamic_cast<Bool&>(*b).Value);
  ASSERT_EQ(42, dynamic_cast<Int&>(*i).Value);
  ASSERT_EQ(3.14, dynamic_cast<Float&>(*f).Value);
  ASSERT_EQ("foo", dynamic_cast<Str&>(*s).Value);
  ASSERT_EQ(4, table.GetCount());
  
  //Overwrite tests
  table.PutSymbol("b", ExpressionPtr { Factory.Alloc<Bool>(false) });
  table.PutSymbol("i", ExpressionPtr { Factory.Alloc<Int>(123) });
  table.PutSymbol("f", ExpressionPtr { Factory.Alloc<Float>(1.23) });
  table.PutSymbol("s", ExpressionPtr { Factory.Alloc<Str>("hello, world!") });
  ASSERT_TRUE(table.GetSymbol("b", b));
  ASSERT_TRUE(table.GetSymbol("i", i));
  ASSERT_TRUE(table.GetSymbol("f", f));
  ASSERT_TRUE(table.GetSymbol("s", s));
  ASSERT_FALSE(dynamic_cast<Bool&>(*b).Value);
  ASSERT_EQ(123, dynamic_cast<Int&>(*i).Value);
  ASSERT_EQ(1.23, dynamic_cast<Float&>(*f).Value);
  ASSERT_EQ("hello, world!", dynamic_cast<Str&>(*s).Value);
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
  table.PutSymbol("b", ExpressionPtr { Factory.Alloc<Bool>(true) });
  table.PutSymbol("i", ExpressionPtr { Factory.Alloc<Int>(42) });
  table.PutSymbol("f", ExpressionPtr { Factory.Alloc<Float>(3.14) });
  table.PutSymbol("s", ExpressionPtr { Factory.Alloc<Str>("foo") });
  ASSERT_TRUE(table.GetSymbol("b", b));
  ASSERT_TRUE(table.GetSymbol("i", i));
  ASSERT_TRUE(table.GetSymbol("f", f));
  ASSERT_TRUE(table.GetSymbol("s", s));
  ASSERT_EQ(true, dynamic_cast<Bool&>(*b).Value);
  ASSERT_EQ(42, dynamic_cast<Int&>(*i).Value);
  ASSERT_EQ(3.14, dynamic_cast<Float&>(*f).Value);
  ASSERT_EQ("foo", dynamic_cast<Str&>(*s).Value);
  ASSERT_EQ(4, table.GetCount());
}

TEST_F(SymbolTableTest, TestDeleteSymbol) {
  SymbolTableType store;
  SymbolTable table(store, NullSourceContext);
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
  string LastName;
  ExpressionPtr LastValue;
  int Count;
  
  void Reset() {
    LastName = "";
    LastValue.reset();
    Count = 0;
  }
};

TEST_F(SymbolTableTest, TestForEach) {
  SymbolTableType store;
  SymbolTable table(store, NullSourceContext);
  ForEachData data;
  auto fn = [&data](const string &name, ExpressionPtr &val) {
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
  ExpressionPtr fortyTwo { Factory.Alloc<Int>(42) };
  ASSERT_EQ(*fortyTwo, *data.LastValue);

  table.DeleteSymbol("num");
  data.Reset();
  table.ForEach(fn);
  ASSERT_EQ(0, data.Count);
  ASSERT_EQ("", data.LastName);
  ASSERT_FALSE(data.LastValue);

  table.PutSymbolInt("num", 42);
  table.PutSymbolStr("str", "foo");
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

class ScopeTest: public BaseTest {
};

TEST_F(ScopeTest, TestPutSymbol_Simple) {
  SymbolTableType store;
  SymbolTable table(store, NullSourceContext);
  ExpressionPtr temp;
  table.PutSymbolInt("a", 1);
  {
    Scope outer(table, NullSourceContext);
    table.PutSymbolInt("b", 2);
    outer.PutSymbol("c", ExpressionPtr { Factory.Alloc<Int>(3) });
    outer.PutSymbol("c", ExpressionPtr { Factory.Alloc<Int>(2) }); // #20
    
    ASSERT_FALSE(outer.IsScopedSymbol("a"));
    ASSERT_FALSE(outer.IsScopedSymbol("b"));
    ASSERT_TRUE(outer.IsScopedSymbol("c"));

    ASSERT_TRUE(table.GetSymbol("a", temp));
    ASSERT_TRUE(table.GetSymbol("b", temp));
    ASSERT_TRUE(table.GetSymbol("c", temp));
    {
      Scope inner(table, NullSourceContext);
      table.PutSymbolInt("d", 4);
      inner.PutSymbol("e", ExpressionPtr { Factory.Alloc<Int>(5) });

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

TEST_F(ScopeTest, TestPutSymbol_ShadowingSameType) {
  SymbolTableType store;
  SymbolTable table(store, NullSourceContext);
  ExpressionPtr one { Factory.Alloc<Int>(1) };
  ExpressionPtr temp;
  table.PutSymbol("a", one->Clone());
  {
    Scope outer(table, NullSourceContext);
    ExpressionPtr two { Factory.Alloc<Int>(2) };
    outer.PutSymbol("a", two->Clone());
    ASSERT_TRUE(table.GetSymbol("a", temp));
    ASSERT_EQ(*two, *temp);
    {
      Scope inner(table, NullSourceContext);
      ExpressionPtr three { Factory.Alloc<Int>(3) };
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

TEST_F(ScopeTest, TestPutSymbol_ShadowingDifferentType) {
  SymbolTableType store;
  SymbolTable table(store, NullSourceContext);
  ExpressionPtr one { Factory.Alloc<Int>(1) };
  ExpressionPtr temp;
  table.PutSymbol("a", one->Clone());
  {
    Scope outer(table, NullSourceContext);
    ExpressionPtr tru { Factory.Alloc<Bool>(true) };
    outer.PutSymbol("a", tru->Clone());
    ASSERT_TRUE(table.GetSymbol("a", temp));
    ASSERT_EQ(*tru, *temp);
    {
      Scope inner(table, NullSourceContext);
      ExpressionPtr hello { Factory.Alloc<Str>("hello") };
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

class InterpreterTest: public BaseTest {
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
    InterpretedFunction outerFunc(NullSourceContext);
    StackFrame outer(Interpreter_, outerFunc);
    outer.PutDynamicSymbol("dynamic", ExpressionPtr { Factory.Alloc<Int>(1) });
    outer.PutLocalSymbol("outer", ExpressionPtr { Factory.Alloc<Int>(2) });
    ASSERT_TRUE(outer.GetSymbol("dynamic", temp));
    ASSERT_TRUE(outer.GetSymbol("outer", temp));
    ASSERT_EQ(1, outer.GetLocalSymbols().GetCount());
    {
      InterpretedFunction innerFun(NullSourceContext);
      StackFrame inner(Interpreter_, innerFun);
      inner.PutDynamicSymbol("dynamic2", ExpressionPtr { Factory.Alloc<Int>(3) });
      inner.PutLocalSymbol("inner", ExpressionPtr { Factory.Alloc<Int>(4) });
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
    InterpretedFunction outerFunc(NullSourceContext);
    StackFrame outer(Interpreter_, outerFunc);
    ExpressionPtr one { Factory.Alloc<Int>(1) };
    ExpressionPtr two { Factory.Alloc<Int>(2) };
    outer.PutDynamicSymbol("dynamic", one->Clone());
    outer.PutLocalSymbol("local", two->Clone());
    ASSERT_TRUE(outer.GetSymbol("dynamic", temp));
    ASSERT_EQ(*one, *temp);
    ASSERT_TRUE(outer.GetSymbol("local", temp));
    ASSERT_EQ(*two, *temp);
    ASSERT_EQ(1, outer.GetLocalSymbols().GetCount());
    {
      InterpretedFunction innerFun(NullSourceContext);
      StackFrame inner(Interpreter_, innerFun);
      ExpressionPtr foo { Factory.Alloc<Str>("foo") };
      ExpressionPtr bar { Factory.Alloc<Str>("bar") };
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
  ExpressionPtr func { Factory.Alloc<CompiledFunction>(move(def), slispFn) };
  FunctionPtr defaultFn;
  auto &settings = Interpreter_.GetSettings();
  ASSERT_FALSE(settings.GetDefaultFunction(defaultFn));
  ASSERT_FALSE(defaultFn);
  settings.PutDefaultFunction(move(dynamic_cast<CompiledFunction&>(*func->Clone())));
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
    ASSERT_EQ("here" + to_string(errNum), error.Where);
    ASSERT_EQ("err" + to_string(errNum), error.What);
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
    return initialDynCount + Interpreter_.GetDynamicSymbols(NullSourceContext).GetCount();
  };
  auto getCurrLocalCount = [this, &initialLocalCount]() {
    return initialLocalCount + Interpreter_.GetCurrentStackFrame().GetLocalSymbols().GetCount();
  };
  initialDynCount = getCurrDynCount();
  initialLocalCount = getCurrLocalCount();
  {
    StackFrame outer(Interpreter_, InterpretedFunction(NullSourceContext));
    outer.PutDynamicSymbol("d1", ExpressionPtr { Factory.Alloc<Int>(1) });
    outer.PutLocalSymbol("outer1", ExpressionPtr { Factory.Alloc<Int>(2) });
    ASSERT_EQ(initialDynCount + 1, getCurrDynCount());
    ASSERT_EQ(initialLocalCount + 1, getCurrLocalCount());
    {
      StackFrame inner(Interpreter_, InterpretedFunction(NullSourceContext));
      inner.PutDynamicSymbol("d2", ExpressionPtr { Factory.Alloc<Int>(3) });
      inner.PutLocalSymbol("inner1", ExpressionPtr { Factory.Alloc<Int>(4) });
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
        NullSourceContext,
        FuncDef { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() },
        &DefaultFunction
      }); 

      Interpreter_.GetDynamicSymbols(NullSourceContext).PutSymbolFunction("myCompiledFunc", CompiledFunction {
        NullSourceContext,
        FuncDef { FuncDef::OneArg(Bool::TypeInstance), FuncDef::NoArgs() },
        &MyCompiledFunc
      });

      ExpressionPtr trueValue { Factory.Alloc<Bool>(true) };
      ExpressionPtr code {
        new Sexp(NullSourceContext, {
          ExpressionPtr { Factory.Alloc<Symbol>("myCompiledFunc") },
          ExpressionPtr { Factory.Alloc<Symbol>("x") }
        })
      };
      ArgList args;
      args.emplace_back(Factory.Alloc<Symbol>("x"));
      Interpreter_.GetDynamicSymbols(NullSourceContext).PutSymbolFunction("myInterpretedFunc", InterpretedFunction {
        NullSourceContext,
        FuncDef { FuncDef::OneArg(Bool::TypeInstance), FuncDef::NoArgs() },
        move(code),
        move(args)
       });


      ExpressionPtr badFnCode {
        Factory.Alloc<Symbol>("undefinedSymbol")
      };
      ArgList badFnArgs;
      Interpreter_.GetDynamicSymbols(NullSourceContext).PutSymbolFunction("myInterpretedBadFunc", InterpretedFunction {
        NullSourceContext,
        FuncDef { FuncDef::NoArgs(), FuncDef::NoArgs() },
        move(badFnCode),
        move(badFnArgs)
       });
    }

    void TestSexpFunction(const string &funcName);
    void TestBasicArg(const string &funcName);
    void TestWrongTypeArg(const string &funcName);
    void TestSymbolArg(const string &funcName);
    void TestNotEnoughArgs(const string &funcName);
    void TestTooManyArgs(const string &funcName);
    
    static bool DefaultFunction(EvaluationContext &ctx) {
      LastResult = move(ctx.Expr_);
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
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Bool>(true) }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Int>(42) }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Float>(3.14) }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Str>("hello, world!") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Quote>(ExpressionPtr { Factory.Alloc<Bool>(true) })}));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<CompiledFunction>() }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<InterpretedFunction>() }));
  
  ExpressionPtr code {};
  ArgList args {};
  ExpressionPtr e { Factory.Alloc<InterpretedFunction>(
    FuncDef { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() },
    move(code),
    move(args)
  ) };
  ASSERT_TRUE(Interpreter_.Evaluate(e));
  ASSERT_EQ(0, Interpreter_.GetErrors().size());
}

TEST_F(EvaluationTest, TestSymbol) {
  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Symbol>("b") }));
  ASSERT_GT(Interpreter_.GetErrors().size(), 0U);

  auto &currStackFrame = Interpreter_.GetCurrentStackFrame();
  currStackFrame.PutLocalSymbol("b", ExpressionPtr { Factory.Alloc<Bool>(true) });
  currStackFrame.PutLocalSymbol("i", ExpressionPtr { Factory.Alloc<Int>(42) });
  currStackFrame.PutLocalSymbol("f", ExpressionPtr { Factory.Alloc<Float>(3.14) });
  currStackFrame.PutLocalSymbol("s", ExpressionPtr { Factory.Alloc<Str>("foo") });
  currStackFrame.PutLocalSymbol("fn", ExpressionPtr { Factory.Alloc<CompiledFunction>() });
  currStackFrame.PutLocalSymbol("q", ExpressionPtr { Factory.Alloc<Quote>(ExpressionPtr { Factory.Alloc<Bool>(true) })});

  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Symbol>("b") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Symbol>("i") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Symbol>("f") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Symbol>("s") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Symbol>("fn") }));
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { Factory.Alloc<Symbol>("q") }));
  ASSERT_EQ(Interpreter_.GetErrors().size(), 0);
}

TEST_F(EvaluationTest, TestSexpDefaultFunction) {
  ExpressionPtr trueValue { Factory.Alloc<Bool>(true) },
                falseValue { Factory.Alloc<Bool>(false) };
  auto &settings = Interpreter_.GetSettings();
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Sexp(NullSourceContext, {
    ExpressionPtr { Factory.Alloc<Symbol>(settings.GetDefaultSexp()) },
    falseValue->Clone()
  })}));
  ASSERT_TRUE(CommandInterface.Output.find("false") != string::npos);
  ASSERT_EQ(*falseValue, *EvaluationTest::LastArgs.front());

  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Sexp(NullSourceContext, {
    ExpressionPtr { Factory.Alloc<Symbol>(settings.GetDefaultSexp()) },
    trueValue->Clone()
  })}));
  ASSERT_TRUE(CommandInterface.Output.find("true") != string::npos);
  ASSERT_EQ(*trueValue, *EvaluationTest::LastArgs.front());
}

void EvaluationTest::TestBasicArg(const string &funcName) {
  ExpressionPtr trueValue { Factory.Alloc<Bool>(true) };
 
  ExpressionPtr sym { Factory.Alloc<Symbol>(funcName) };
  ExpressionPtr sexp { new Sexp(NullSourceContext, { move(sym), move(trueValue) }) };

  EvaluationTest::MyFuncCalled = false;
  ASSERT_TRUE(Interpreter_.Evaluate(sexp));
  ASSERT_TRUE(EvaluationTest::MyFuncCalled);
}

void EvaluationTest::TestSymbolArg(const string &funcName) {
  ExpressionPtr sym { Factory.Alloc<Symbol>("sym") };
  EvaluationTest::MyFuncCalled = false;
  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Sexp(NullSourceContext, {
    ExpressionPtr { Factory.Alloc<Symbol>(funcName) },
    sym->Clone()
  })}));
  ASSERT_FALSE(EvaluationTest::MyFuncCalled);

  Interpreter_.GetCurrentStackFrame().PutLocalSymbol("sym", ExpressionPtr { Factory.Alloc<Bool>(false) });
  EvaluationTest::MyFuncCalled = false;
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Sexp(NullSourceContext, {
    ExpressionPtr { Factory.Alloc<Symbol>(funcName) },
    sym->Clone()
  })}));
  ASSERT_TRUE(EvaluationTest::MyFuncCalled);
}

void EvaluationTest::TestNotEnoughArgs(const string &funcName) {
  EvaluationTest::MyFuncCalled = false;
  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Sexp(NullSourceContext, {
    ExpressionPtr { Factory.Alloc<Symbol>(funcName) }
  })}));
  ASSERT_FALSE(EvaluationTest::MyFuncCalled);
}

void EvaluationTest::TestTooManyArgs(const string &funcName) {
  ExpressionPtr trueValue { Factory.Alloc<Bool>(true) };
 
  EvaluationTest::MyFuncCalled = false;
  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Sexp(NullSourceContext, {
    ExpressionPtr { Factory.Alloc<Symbol>(funcName) },
    trueValue->Clone(),
    trueValue->Clone()
  })}));
  ASSERT_FALSE(EvaluationTest::MyFuncCalled);
}

void EvaluationTest::TestWrongTypeArg(const string &funcName) {
  ExpressionPtr str { Factory.Alloc<Str>("foo") };
  EvaluationTest::MyFuncCalled = false;
  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Sexp(NullSourceContext, {
    ExpressionPtr { Factory.Alloc<Symbol>(funcName) },
    str->Clone()
  })}));
  ASSERT_FALSE(EvaluationTest::MyFuncCalled);
}

void EvaluationTest::TestSexpFunction(const string &funcName) {
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

  ASSERT_FALSE(Interpreter_.Evaluate(ExpressionPtr { new Sexp(NullSourceContext, {
    ExpressionPtr { Factory.Alloc<Symbol>("myInterpretedBadFunc") }
  })}));
}

TEST_F(EvaluationTest, TestSexpList) {
  auto &settings = Interpreter_.GetSettings();
  settings.PutListFunction(CompiledFunction {
    NullSourceContext,
    FuncDef { FuncDef::AnyArgs(), FuncDef::OneArg(Quote::TypeInstance) },
    &MyListFunc
  });

  ExpressionPtr listSym { Factory.Alloc<Symbol>(settings.GetListSexp()) };
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Sexp(NullSourceContext, { listSym->Clone()} ) }));
  ASSERT_TRUE(EvaluationTest::MyListFuncCalled);
  ASSERT_EQ(0, MyListLastArgCount);

  ExpressionPtr boolValue  { Factory.Alloc<Bool>(true) },
                intValue   { Factory.Alloc<Int>(42) },
                floatValue { Factory.Alloc<Float>(3.14) },
                strValue   { Factory.Alloc<Str>("foo") },
                symValue   { Factory.Alloc<Symbol>("myCompiledFunc") };
  ASSERT_TRUE(Interpreter_.Evaluate(ExpressionPtr { new Sexp(NullSourceContext, {
    listSym->Clone(),
    boolValue->Clone(),
    intValue->Clone(),
    floatValue->Clone(),
    strValue->Clone(),
    symValue->Clone()
  }) }));
  ASSERT_EQ(5, MyListLastArgCount);
}
