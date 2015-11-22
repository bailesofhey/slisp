#include <functional>

#include "gtest\gtest.h"
#include "Interpreter.h"

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

//void PutSymbolFunction(const std::string &symbolName, Function &&func);
//void PutSymbolFunction(const std::string &symbolName, SlipFunction fn, FuncDef &&def);

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
}

TEST(SymbolTable, TestDeleteSymbol) {
  SymbolTable table;
  ExpressionPtr val;
  ASSERT_FALSE(table.GetSymbol("val", val));
  table.DeleteSymbol("val");
  table.PutSymbolNumber("val", 42);
  ASSERT_TRUE(table.GetSymbol("val", val));
  table.DeleteSymbol("val");
  ASSERT_FALSE(table.GetSymbol("val", val));
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

/*
class Scope {
explicit Scope(SymbolTable &symbols);
void PutSymbol(const std::string &symbolName, ExpressionPtr &value);
*/

/*
class StackFrame {
explicit StackFrame(Interpreter &interp, Function &func);
void PutLocalSymbol(const std::string &symbolName, ExpressionPtr &value);
void PutDynamicSymbol(const std::string &symbolName, ExpressionPtr &value);
bool GetSymbol(const std::string &symbolName, ExpressionPtr &value);
void DeleteSymbol(const std::string &symbolName);
SymbolTable& GetLocalSymbols();
*/

/*
class Interpreter {
  bool Evaluate(ExpressionPtr &expr);
  bool PushError(const EvalError &error);
  std::list<EvalError> GetErrors() const;
  void ClearErrors();
  void Stop();
  bool StopRequested() const;
  SymbolTable& GetDynamicSymbols();
  void PutDefaultFunction(Function &&func);
  bool GetDefaultFunction(Function *func);
  const std::string GetDefaultSexp() const;
  StackFrame& GetCurrentStackFrame();
  void PushStackFrame(StackFrame &stackFrame);
  void PopStackFrame();
*/

TEST(Interpreter, TestSimple) {
  Interpreter interpreter;
  auto defaultSexp = interpreter.GetDefaultSexp();
  ASSERT_FALSE(defaultSexp.empty());
}