#include <sstream>
#include <string>
#include "gtest\gtest.h"

#include "Controller.h"


class StdLibTest: public ::testing::Test {
  protected:
    Controller Controller_;
    std::stringstream Out;
    int NOutputLines;
    
    explicit StdLibTest() {
      Controller_.SetOutput(Out);
    }

    void Reset() {
      Out.str("");
      Out.clear();
      NOutputLines = 0;
    }

    bool RunSuccess(const std::string &code, const std::string &expectedResult) {
      Reset();
      Controller_.Run(code);

      for (auto &ch : Out.str()) {
        if (ch == '\n')
          ++NOutputLines;      
      }

      return Out.str().find(expectedResult) != std::string::npos;
    }

    bool RunFail(const std::string &code) {
      return RunSuccess(code, "Error");
    }
};

class StdLibDefaultFunctionTest: public StdLibTest {
};

TEST_F(StdLibDefaultFunctionTest, TestLiterals) {
  ASSERT_TRUE(RunFail("a"));
  ASSERT_TRUE(RunSuccess("", ""));
  ASSERT_TRUE(RunSuccess("42", "42"));
  ASSERT_TRUE(RunSuccess("\"foo\"", "\"foo\""));
  ASSERT_TRUE(RunSuccess("+", "Function"));
  ASSERT_TRUE(RunSuccess("true", "true"));
  ASSERT_TRUE(RunSuccess("false", "false"));
  ASSERT_TRUE(RunSuccess("nil", "()"));

  const char *args = "42 \"foo\" + (1 2 3)";
  ASSERT_TRUE(RunSuccess(args, "42"));
  ASSERT_TRUE(RunSuccess(args, "\"foo\""));
  ASSERT_TRUE(RunSuccess(args, "Function"));
  ASSERT_TRUE(RunSuccess(args, "1"));
  ASSERT_TRUE(RunSuccess(args, "2"));
  ASSERT_TRUE(RunSuccess(args, "3"));
}

class StdLibInterpreterTest: public StdLibTest {
  protected:
    void TestSetFunctions();
};

TEST_F(StdLibInterpreterTest, TestPrint) {
  ASSERT_TRUE(RunSuccess("(print)", ""));
  ASSERT_TRUE(RunFail("(print a)"));
  ASSERT_TRUE(RunSuccess("(print 42)", "42"));
  ASSERT_EQ(2, NOutputLines);
  ASSERT_TRUE(RunSuccess("(print \"\")", "\"\""));
  ASSERT_TRUE(RunSuccess("(print \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(print +)", "Function"));

  const char *list = "(print (1 2 3))";
  ASSERT_TRUE(RunSuccess(list, "1"));
  ASSERT_EQ(2, NOutputLines);
  ASSERT_TRUE(RunSuccess(list, "2"));
  ASSERT_TRUE(RunSuccess(list, "3"));

  const char *unWrappedList = "(print 1 2 3)";
  ASSERT_TRUE(RunSuccess(unWrappedList, "1"));
  ASSERT_EQ(4, NOutputLines);
  ASSERT_TRUE(RunSuccess(unWrappedList, "2"));
  ASSERT_TRUE(RunSuccess(unWrappedList, "3"));

  const char *variadic = "(print 42 \"foo\" + (1 2 3))"; 
  ASSERT_TRUE(RunSuccess(variadic, "42"));
  ASSERT_EQ(5, NOutputLines);
  ASSERT_TRUE(RunSuccess(variadic, "\"foo\""));
  ASSERT_TRUE(RunSuccess(variadic, "Function"));
  ASSERT_TRUE(RunSuccess(variadic, "1"));
  ASSERT_TRUE(RunSuccess(variadic, "2"));
  ASSERT_TRUE(RunSuccess(variadic, "3"));
}

TEST_F(StdLibInterpreterTest, TestQuit) {
  std::stringstream in;
  in << "(+ 2 3)" << std::endl
     << "(+ 4 6)" << std::endl
     << "(quit)" << std::endl
     << "(+ 8 12)" << std::endl;
  Controller_.Run(in);
  ASSERT_NE(Out.str().find("5"), std::string::npos);
  ASSERT_NE(Out.str().find("10"), std::string::npos);
  ASSERT_EQ(Out.str().find("20"), std::string::npos);
}

TEST_F(StdLibInterpreterTest, TestHelp) {
  ASSERT_TRUE(RunSuccess("(help)", "help"));
  ASSERT_TRUE(RunSuccess("(help)", "+"));
  int helpAllLines = NOutputLines;
  ASSERT_GT(helpAllLines, 20);
  ASSERT_TRUE(RunFail("(help 3)"));
  ASSERT_TRUE(RunFail("(help \"foo\")"));
  ASSERT_TRUE(RunSuccess("(help true)", "()"));
  ASSERT_TRUE(RunSuccess("(help +)", "+"));
  ASSERT_LT(NOutputLines, helpAllLines);
  ASSERT_TRUE(RunSuccess("(help help)", "help"));
}

void StdLibInterpreterTest::TestSetFunctions() {
  ASSERT_TRUE(RunFail("(set)"));
  ASSERT_TRUE(RunFail("(unset)"));
  ASSERT_TRUE(RunFail("(set 42)"));
  ASSERT_TRUE(RunFail("(unset 42)"));
  ASSERT_TRUE(RunFail("(set \"foo\")"));
  ASSERT_TRUE(RunFail("(unset \"foo\")"));
  ASSERT_TRUE(RunFail("(set a)"));
  ASSERT_TRUE(RunFail("(unset a)"));
  ASSERT_TRUE(RunFail("(set a b)"));

  ASSERT_TRUE(RunFail("(set a a)"));

  ASSERT_TRUE(RunSuccess("(set a 42)", "42"));
  ASSERT_TRUE(RunSuccess("(set b \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(set c +)", "Function"));
  ASSERT_TRUE(RunSuccess("(set d true)", "true"));
  ASSERT_TRUE(RunSuccess("(set e (1 2 3))", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(set f (quote (+ 2 3)))", "(+ 2 3)"));

  ASSERT_TRUE(RunSuccess("a", "42"));
  ASSERT_TRUE(RunSuccess("b", "\"foo\""));
  ASSERT_TRUE(RunSuccess("c", "Function"));
  ASSERT_TRUE(RunSuccess("d", "true"));
  ASSERT_TRUE(RunSuccess("e", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("f", "(+ 2 3)"));

  ASSERT_TRUE(RunSuccess("(unset a)", "42"));
  ASSERT_TRUE(RunSuccess("(unset b)", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(unset c)", "Function"));
  ASSERT_TRUE(RunSuccess("(unset d)", "true"));
  ASSERT_TRUE(RunSuccess("(unset e)", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(unset f)", "(+ 2 3)"));

  ASSERT_TRUE(RunFail("a"));
  ASSERT_TRUE(RunFail("b"));
  ASSERT_TRUE(RunFail("c"));
  ASSERT_TRUE(RunFail("d"));
  ASSERT_TRUE(RunFail("e"));
  ASSERT_TRUE(RunFail("f"));

  ASSERT_TRUE(RunFail("(unset a)"));
  ASSERT_TRUE(RunFail("(unset b)"));
  ASSERT_TRUE(RunFail("(unset c)"));
  ASSERT_TRUE(RunFail("(unset d)"));
  ASSERT_TRUE(RunFail("(unset e)"));
  ASSERT_TRUE(RunFail("(unset f)"));

  ASSERT_TRUE(RunFail("a"));
  ASSERT_TRUE(RunFail("b"));
  ASSERT_TRUE(RunFail("c"));
  ASSERT_TRUE(RunFail("d"));
  ASSERT_TRUE(RunFail("e"));
  ASSERT_TRUE(RunFail("f"));

  ASSERT_TRUE(RunSuccess("(set a 42)", "42"));
  ASSERT_TRUE(RunSuccess("(set b \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(set c +)", "Function"));
  ASSERT_TRUE(RunSuccess("(set d true)", "true"));
  ASSERT_TRUE(RunSuccess("(set e (1 2 3))", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(set f (quote (+ 2 3)))", "(+ 2 3)"));

  ASSERT_TRUE(RunSuccess("(set f 42)", "42"));
  ASSERT_TRUE(RunSuccess("(set e \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(set d +)", "Function"));
  ASSERT_TRUE(RunSuccess("(set c true)", "true"));
  ASSERT_TRUE(RunSuccess("(set b (1 2 3))", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(set a (quote (+ 2 3)))", "(+ 2 3)"));

  ASSERT_TRUE(RunSuccess("f", "42"));
  ASSERT_TRUE(RunSuccess("e", "\"foo\""));
  ASSERT_TRUE(RunSuccess("d", "Function"));
  ASSERT_TRUE(RunSuccess("c", "true"));
  ASSERT_TRUE(RunSuccess("b", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("a", "(+ 2 3)"));
}

TEST_F(StdLibInterpreterTest, TestSet) {
  ASSERT_NO_FATAL_FAILURE(TestSetFunctions());
}

TEST_F(StdLibInterpreterTest, TestUnSet) {
  ASSERT_NO_FATAL_FAILURE(TestSetFunctions());
}

class StdLibNumericalTest: public StdLibTest {
  protected:
    decltype(Number::Value) MinValue,
                            MaxValue;
   explicit StdLibNumericalTest():
    MinValue(std::numeric_limits<decltype(MinValue)>::min()),
    MaxValue(std::numeric_limits<decltype(MaxValue)>::max())
  {
  }
  
  void TestBadNumericArgs(const std::string &name) {
    static const char * const values[] = {" ", "\"foo\"", "+", "a"};
    std::string prefix = "(" + name + " ";
    for (auto &value : values) {
      if (name != "+" || value[0] != '"') {
        std::string code = prefix + value + ")";
        ASSERT_TRUE(RunFail(code));
      }
    }
  }

  void TestIdentity(const std::string &name) {
    static const char * const values[] = {"0", "1", "42", "false", "true"};
    std::string prefix = "(" + name + " ";
    for (auto &value : values) {
      std::string code = prefix + value + ")";
      const char *expectedValue = value;
      if (std::strcmp(value, "true") == 0)
        expectedValue = "1";
      else if (std::strcmp(value, "false") == 0)
        expectedValue = "0";
      ASSERT_TRUE(RunSuccess(code, expectedValue));
    }
  }
};

TEST_F(StdLibNumericalTest, TestInc) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("inc"));
  ASSERT_TRUE(RunFail("(inc \"foo\")"));
  ASSERT_TRUE(RunSuccess("(inc -2)", "-1"));
  ASSERT_TRUE(RunSuccess("(inc -1)", "0"));
  ASSERT_TRUE(RunSuccess("(inc 0)", "1"));
  ASSERT_TRUE(RunSuccess("(inc 41)", "42"));

  std::stringstream temp;
  temp << "(inc " << MaxValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), std::to_string(MinValue)));
}

TEST_F(StdLibNumericalTest, TestDec) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("dec"));
  ASSERT_TRUE(RunSuccess("(dec -1)", "-2"));
  ASSERT_TRUE(RunSuccess("(dec 0)", "-1"));
  ASSERT_TRUE(RunSuccess("(dec 1)", "0"));
  ASSERT_TRUE(RunSuccess("(dec 42)", "41"));

  std::stringstream temp;
  temp << "(dec " << MinValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), std::to_string(MaxValue)));
}

TEST_F(StdLibNumericalTest, TestAdd) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("+"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("+"));
  ASSERT_TRUE(RunSuccess("(+ 2 3)", "5"));
  ASSERT_TRUE(RunSuccess("(+ 1 2 3 4 5)", "15"));

  std::stringstream temp;
  temp << "(+ 1 " << MaxValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), std::to_string(MinValue)));
}

TEST_F(StdLibNumericalTest, TestSub) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("-"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("-"));
  ASSERT_TRUE(RunSuccess("(- 5 3)", "2"));
  ASSERT_TRUE(RunSuccess("(- 1 2 3 4 5)", "-13"));

  std::stringstream temp;
  temp << "(- 1 " << MinValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), std::to_string(MaxValue)));
}

TEST_F(StdLibNumericalTest, TestMult) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("*"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("*"));
  ASSERT_TRUE(RunSuccess("(* 2 3)", "6"));
  ASSERT_TRUE(RunSuccess("(* 1 2 3 4 5)", "120"));
}

TEST_F(StdLibNumericalTest, TestDiv) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("/"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("/"));
  ASSERT_TRUE(RunSuccess("(/ 6 2)", "3"));
  ASSERT_TRUE(RunSuccess("(/ 120 5 4 3 2 1)", "1"));
  ASSERT_TRUE(RunSuccess("(/ 1 2)", "0"));
  ASSERT_TRUE(RunFail("(/ 1 0)"));
  ASSERT_TRUE(RunFail("(/ 0 0)"));
}

TEST_F(StdLibNumericalTest, TestMod) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("%"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("%"));
  ASSERT_TRUE(RunSuccess("(% 0 3)", "0"));
  ASSERT_TRUE(RunSuccess("(% 1 3)", "1"));
  ASSERT_TRUE(RunSuccess("(% 2 3)", "2"));
  ASSERT_TRUE(RunSuccess("(% 3 3)", "0"));
  ASSERT_TRUE(RunSuccess("(% 4 3)", "1"));
  ASSERT_TRUE(RunSuccess("(% 9 5 2)", "0"));
  ASSERT_TRUE(RunFail("(% 1 0)"));
  ASSERT_TRUE(RunFail("(% 0 0)"));
  ASSERT_TRUE(RunSuccess("(% -1 3)", "-1"));
  ASSERT_TRUE(RunSuccess("(% 1 -3)", "1"));
  ASSERT_TRUE(RunSuccess("(% -1 -3)", "1"));
}

class StdLibBitwiseTest: public StdLibNumericalTest {
};

TEST_F(StdLibBitwiseTest, TestLeftShift) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("<<"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("<<"));
  ASSERT_TRUE(RunSuccess("(<< 0 0)", "0"));
  ASSERT_TRUE(RunSuccess("(<< 0 1)", "0"));
  ASSERT_TRUE(RunSuccess("(<< 1 0)", "1"));
  ASSERT_TRUE(RunSuccess("(<< 1 1)", "2"));
  ASSERT_TRUE(RunSuccess("(<< 1 2)", "4"));
  ASSERT_TRUE(RunSuccess("(<< 1 3)", "8"));
  ASSERT_TRUE(RunSuccess("(<< 1 30)", "1073741824"));
  ASSERT_TRUE(RunSuccess("(<< 1 31)", "2147483648"));
  ASSERT_TRUE(RunSuccess("(<< 1 2 3 4 5)", "16384"));
}

TEST_F(StdLibBitwiseTest, TestRightShift) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs(">>"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity(">>"));
  ASSERT_TRUE(RunSuccess("(>> 0 0)", "0"));
  ASSERT_TRUE(RunSuccess("(>> 0 1)", "0"));
  ASSERT_TRUE(RunSuccess("(>> 1 0)", "1"));
  ASSERT_TRUE(RunSuccess("(>> 2 1)", "1"));
  ASSERT_TRUE(RunSuccess("(>> 4 2)", "1"));
  ASSERT_TRUE(RunSuccess("(>> 8 3)", "1"));
  ASSERT_TRUE(RunSuccess("(>> 1073741824 30)", "1"));
  ASSERT_TRUE(RunSuccess("(>> 2147483648 31)", "1"));
  ASSERT_TRUE(RunSuccess("(>> 128 2 3)", "4"));
}

TEST_F(StdLibBitwiseTest, BitAnd) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("&"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("&"));
  ASSERT_TRUE(RunSuccess("(& 0 0)", "0"));
  ASSERT_TRUE(RunSuccess("(& 0 1)", "0"));
  ASSERT_TRUE(RunSuccess("(& 1 0)", "0"));
  ASSERT_TRUE(RunSuccess("(& true false)", "0"));
  ASSERT_TRUE(RunSuccess("(& 1 1)", "1"));
  ASSERT_TRUE(RunSuccess("(& 3 0)", "0"));
  ASSERT_TRUE(RunSuccess("(& 3 1)", "1"));
  ASSERT_TRUE(RunSuccess("(& 3 2)", "2"));
  ASSERT_TRUE(RunSuccess("(& 3 3)", "3"));
  ASSERT_TRUE(RunSuccess("(& 3 4)", "0"));
  ASSERT_TRUE(RunSuccess("(& 3 5)", "1"));
  ASSERT_TRUE(RunSuccess("(& 1073741828 4)", "4"));
  ASSERT_TRUE(RunSuccess("(& 9 5 3)", "1"));
}

TEST_F(StdLibBitwiseTest, BitOr) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("|"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("|"));
  ASSERT_TRUE(RunSuccess("(| 0 0)", "0"));
  ASSERT_TRUE(RunSuccess("(| 0 1)", "1"));
  ASSERT_TRUE(RunSuccess("(| true false)", "1"));
  ASSERT_TRUE(RunSuccess("(| 1 0)", "1"));
  ASSERT_TRUE(RunSuccess("(| 1 1)", "1"));
  ASSERT_TRUE(RunSuccess("(| 2 1)", "3"));
  ASSERT_TRUE(RunSuccess("(| 3 0)", "3"));
  ASSERT_TRUE(RunSuccess("(| 3 1)", "3"));
  ASSERT_TRUE(RunSuccess("(| 3 2)", "3"));
  ASSERT_TRUE(RunSuccess("(| 3 3)", "3"));
  ASSERT_TRUE(RunSuccess("(| 3 4)", "7"));
  ASSERT_TRUE(RunSuccess("(| 3 5)", "7"));
  ASSERT_TRUE(RunSuccess("(| 1073741824 4)", "1073741828"));
  ASSERT_TRUE(RunSuccess("(| 9 5 4)", "13"));
}

TEST_F(StdLibBitwiseTest, BitXor) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("^"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("^"));
  ASSERT_TRUE(RunSuccess("(^ 0 0)", "0"));
  ASSERT_TRUE(RunSuccess("(^ 0 1)", "1"));
  ASSERT_TRUE(RunSuccess("(^ true false)", "1"));
  ASSERT_TRUE(RunSuccess("(^ 1 0)", "1"));
  ASSERT_TRUE(RunSuccess("(^ 1 1)", "0"));
  ASSERT_TRUE(RunSuccess("(^ 2 1)", "3"));
  ASSERT_TRUE(RunSuccess("(^ 3 0)", "3"));
  ASSERT_TRUE(RunSuccess("(^ 3 1)", "2"));
  ASSERT_TRUE(RunSuccess("(^ 3 2)", "1"));
  ASSERT_TRUE(RunSuccess("(^ 3 3)", "0"));
  ASSERT_TRUE(RunSuccess("(^ 3 4)", "7"));
  ASSERT_TRUE(RunSuccess("(^ 3 5)", "6"));
  ASSERT_TRUE(RunSuccess("(^ 1073741828 4)", "1073741824"));
  ASSERT_TRUE(RunSuccess("(^ 9 5 4)", "8"));
}

TEST_F(StdLibBitwiseTest, BitNot) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("~"));
  ASSERT_TRUE(RunSuccess("(~ 0)", "-1"));
  ASSERT_TRUE(RunSuccess("(~ 1)", "-2"));
  ASSERT_TRUE(RunSuccess("(~ 2)", "-3"));
  ASSERT_TRUE(RunSuccess("(~ 3)", "-4"));
  ASSERT_TRUE(RunSuccess("(~ 1073741824)", "-1073741825"));
  ASSERT_TRUE(RunSuccess("(~ 2147483648)", "-2147483649"));
}

class StdLibStringTest: public StdLibTest {
};

TEST_F(StdLibStringTest, TestAdd) {
  ASSERT_TRUE(RunFail("(+)"));
  ASSERT_TRUE(RunSuccess("(+ \"\")", "\"\""));
  ASSERT_TRUE(RunSuccess("(+ \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(+ \"\" \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(+ \" \" \"foo\")", "\" foo\""));
  ASSERT_TRUE(RunSuccess("(+ \"foo\" \"\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(+ \"foo\" \" \")", "\"foo \""));
  ASSERT_TRUE(RunSuccess("(+ \"\" \"foo\" \"\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(+ \" \" \"foo\" \" \")", "\" foo \""));
  ASSERT_TRUE(RunSuccess("(+ \"foo\" \"bar\" )", "\"foobar\""));
  ASSERT_TRUE(RunSuccess("(+ \"Hello, \" \"world\" \"!\" )", "\"Hello, world!\""));
  ASSERT_TRUE(RunSuccess("(+ \"\" \"\" \"Hello,\" \" \" \"\" \"world\" \"\" \"!\" \"\" \"\")", "\"Hello, world!\""));
}

TEST_F(StdLibStringTest, TestReverse) {
  ASSERT_TRUE(RunFail("(reverse)"));
  ASSERT_TRUE(RunSuccess("(reverse \"\")", "\"\""));
  ASSERT_TRUE(RunSuccess("(reverse \"a\")", "\"a\""));
  ASSERT_TRUE(RunSuccess("(reverse \"ab\")", "\"ba\""));
  ASSERT_TRUE(RunSuccess("(reverse \"abc\")", "\"cba\""));
  ASSERT_TRUE(RunSuccess("(reverse \"satan oscillate my metallic sonatas\")", "\"satanos cillatem ym etallicso natas\""));
}

class StdLibListTest: public StdLibTest {
};

TEST_F(StdLibListTest, TestAdd) {
  ASSERT_TRUE(RunFail("(+)"));
  ASSERT_TRUE(RunSuccess("(+ () )", "()"));
  ASSERT_TRUE(RunSuccess("(+ (1) )", "(1)"));
  ASSERT_TRUE(RunSuccess("(+ (1 2 3) )", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(+ () () )", "()"));
  ASSERT_TRUE(RunSuccess("(+ () () ())", "()"));
  ASSERT_TRUE(RunSuccess("(+ (1) (2 3) (4 5 6))", "(1 2 3 4 5 6)"));
  ASSERT_TRUE(RunSuccess("(+ () (1) () (2 3 4 5) (6) () ())", "(1 2 3 4 5 6)"));
  ASSERT_TRUE(RunSuccess("(+ () (\"foo\") () (2 \"bar \" 4 false) (true) (42 (\"bar\" () (-23) () \"rab\") 24) )",
                         "(\"foo\" 2 \"bar \" 4 false true 42 (\"bar\" () (-23) () \"rab\") 24)"));
}

TEST_F(StdLibListTest, TestList) {
  ASSERT_TRUE(RunSuccess("(list)", "()"));
  ASSERT_TRUE(RunSuccess("()", "()"));
  ASSERT_TRUE(RunSuccess("(list 1)", "(1)"));
  ASSERT_TRUE(RunSuccess("(1)", "(1)"));
  ASSERT_TRUE(RunSuccess("(list 1 \"foo\" false)", "(1 \"foo\" false)"));
  ASSERT_TRUE(RunSuccess("(1 \"foo\" false)", "(1 \"foo\" false)"));
  ASSERT_TRUE(RunSuccess("(((((((((())))))))))", "(((((((((())))))))))"));
  ASSERT_TRUE(RunSuccess("((((((((((42))))))))))", "((((((((((42))))))))))"));
}

TEST_F(StdLibListTest, TestMap) {
  ASSERT_TRUE(RunFail("(map)"));
  ASSERT_TRUE(RunFail("(map 2)"));
  ASSERT_TRUE(RunFail("(map 2 3)"));
  ASSERT_TRUE(RunFail("(map + 3)"));
  ASSERT_TRUE(RunFail("(map a 3)"));
  ASSERT_TRUE(RunFail("(map inc 3)"));
  ASSERT_TRUE(RunSuccess("(map inc ())", "()"));
  ASSERT_TRUE(RunSuccess("(map inc (1))", "(2)"));
  ASSERT_TRUE(RunSuccess("(map inc (1 2 3))", "(2 3 4)"));
  ASSERT_TRUE(RunFail("(map (fn (x) a) (1 2 3))"));
  ASSERT_TRUE(RunFail("(map (fn 3) (1 2 3))"));
  ASSERT_TRUE(RunFail("(map (fn () 42) (1 2 3))"));
  ASSERT_TRUE(RunSuccess("(map (fn (x) 42) (1 2 3))", "(42 42 42)"));
  ASSERT_TRUE(RunSuccess("(map (fn (x) (* x 10)) (1 2 3))", "(10 20 30)"));
}

TEST_F(StdLibListTest, TestHead) {
  ASSERT_TRUE(RunFail("(head)"));
  ASSERT_TRUE(RunFail("(head 3)"));
  ASSERT_TRUE(RunFail("(head a)"));
  ASSERT_TRUE(RunFail("(head \"foo\")"));
  ASSERT_TRUE(RunFail("(head true)"));
  ASSERT_TRUE(RunSuccess("(head ())", "()"));
  ASSERT_TRUE(RunSuccess("(head (1))", "1"));
  ASSERT_TRUE(RunSuccess("(head (1 2))", "1"));
  ASSERT_TRUE(RunFail("(head (head (1 2)))"));
  ASSERT_TRUE(RunSuccess("(head ((1 2)) )", "(1 2)"));
  ASSERT_TRUE(RunSuccess("(head (head ((1 2)) ))", "1"));
  ASSERT_TRUE(RunFail("(head (head (head ((1 2)) )))"));
}

TEST_F(StdLibListTest, TestTail) {
  ASSERT_TRUE(RunFail("(tail)"));
  ASSERT_TRUE(RunFail("(tail 3)"));
  ASSERT_TRUE(RunFail("(tail a)"));
  ASSERT_TRUE(RunFail("(tail \"foo\")"));
  ASSERT_TRUE(RunFail("(tail true)"));
  ASSERT_TRUE(RunSuccess("(tail ())", "()"));
  ASSERT_TRUE(RunSuccess("(tail (1))", "()"));
  ASSERT_TRUE(RunSuccess("(tail (1 2))", "2"));
  ASSERT_TRUE(RunSuccess("(tail (tail (1 2)))", "()"));
  ASSERT_TRUE(RunSuccess("(tail ((1 2)) )", "()"));
  ASSERT_TRUE(RunSuccess("(tail (tail ((1 2)) ))", "()"));
  ASSERT_TRUE(RunSuccess("(tail (tail (tail ((1 2)) )))", "()"));
}

class StdLibComparisonTest: public StdLibTest {
  protected:
    std::string Prefix;

    StdLibComparisonTest()
    {
    }

    void TestEquality(const char *value1, const char *value2) {
      ASSERT_TRUE(RunFail(Prefix + ")"));
      ASSERT_TRUE(RunSuccess(Prefix + "true)", "true"));
      ASSERT_TRUE(RunSuccess(Prefix + "42)", "true"));
      ASSERT_TRUE(RunSuccess(Prefix + "\"foo\")", "true"));
  
      ASSERT_TRUE(RunSuccess(Prefix + "true false)", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "42 5)", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "\"foo\" \"bar\")", value2));

      ASSERT_TRUE(RunSuccess(Prefix + "true true)", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "42 42)", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "\"foo\" \"foo\")", value1));

      //Implicit Number -> Bool not implemented
      //ASSERT_TRUE(RunSuccess(Prefix + "true 1)", value1));
      //ASSERT_TRUE(RunSuccess(Prefix + "1 true)", value1));
      //ASSERT_TRUE(RunSuccess(Prefix + "true 4)", "false"));
      //ASSERT_TRUE(RunSuccess(Prefix + "4 true)", "false"));
      ASSERT_TRUE(RunFail(Prefix + "true 1)"));
      ASSERT_TRUE(RunFail(Prefix + "1 true)"));
      ASSERT_TRUE(RunFail(Prefix + "true 4)"));
      ASSERT_TRUE(RunFail(Prefix + "4 true)"));

      ASSERT_TRUE(RunSuccess(Prefix + "42 (+ 40 2))", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "\"foo\" (reverse \"oof\"))", value1));

      //Comparisons are currently required to be homogeneous
      //ASSERT_TRUE(RunSuccess(Prefix + "42 \"foo\")", "false"));
      //ASSERT_TRUE(RunSuccess(Prefix + "\"foo\" nil)", "true"));
      ASSERT_TRUE(RunFail(Prefix + "42 \"foo\")"));
      ASSERT_TRUE(RunFail(Prefix + "\"foo\" nil)"));
    }

    void TestInEquality(const char *value1, const char *value2, const char *value3) {
      ASSERT_TRUE(RunFail(Prefix + ")"));
      ASSERT_TRUE(RunSuccess(Prefix + "\"foo\")", "true"));
      ASSERT_TRUE(RunSuccess(Prefix + "2)", "true"));
      ASSERT_TRUE(RunSuccess(Prefix + "true)", "true"));

      ASSERT_TRUE(RunSuccess(Prefix + "1 1)", value3));
      ASSERT_TRUE(RunSuccess(Prefix + "2 1)", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "1 2)", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "5 4 3 2 1)", value2));

      ASSERT_TRUE(RunSuccess(Prefix + "\"apple\" \"apple\")", value3));
      ASSERT_TRUE(RunSuccess(Prefix + "\"apple\" \"banana\")", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "\"banana\" \"apple\")", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "\"apple\" \"banana\" \"cantelope\" \"durian\")", value1));
    }
};

TEST_F(StdLibComparisonTest, TestEq) {
  Prefix = "(= ";
  ASSERT_NO_FATAL_FAILURE(TestEquality("true", "false"));
  ASSERT_TRUE(RunSuccess(Prefix + "6 (+ 2 4) (- 7 1) (* 2 3) (/ 24 6))", "false"));
  ASSERT_TRUE(RunSuccess(Prefix + "6 (+ 2 4) (- 7 1) (* 2 3) (/ 24 4))", "true"));
}

TEST_F(StdLibComparisonTest, TestNe) {
  Prefix = "(!= ";
  ASSERT_NO_FATAL_FAILURE(TestEquality("false", "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "6 (+ 2 4) (- 7 1) (* 2 3) (/ 24 6))", "false"));
  ASSERT_TRUE(RunSuccess(Prefix + "6 (+ 2 4) (- 7 1) (* 2 3) (/ 24 4))", "false"));
  ASSERT_TRUE(RunSuccess(Prefix + "6 (+ 2 5) (- 7 2) (* 2 4) (/ 24 4))", "true"));
}

TEST_F(StdLibComparisonTest, TestLt) {
  Prefix = "(< ";
  ASSERT_NO_FATAL_FAILURE(TestInEquality("true", "false", "false"));
  ASSERT_TRUE(RunSuccess(Prefix + "\"apple\" \"banana\" \"cantelope\" \"cactus\")", "false"));
  ASSERT_TRUE(RunSuccess(Prefix + "5 4 3 2 7)", "false"));
}

TEST_F(StdLibComparisonTest, TestGt) {
  Prefix = "(> ";
  ASSERT_NO_FATAL_FAILURE(TestInEquality("false", "true", "false"));
}

TEST_F(StdLibComparisonTest, TestLte) {
  Prefix = "(<= ";
  ASSERT_NO_FATAL_FAILURE(TestInEquality("true", "false", "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "2 3 4 5 4)", "false"));
  ASSERT_TRUE(RunSuccess(Prefix + "2 3 4 5 5)", "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "2 3 4 5 6)", "true"));
}

TEST_F(StdLibComparisonTest, TestGte) {
  Prefix = "(>= ";
  ASSERT_NO_FATAL_FAILURE(TestInEquality("false", "true", "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "\"apple\" \"banana\" \"cantelope\" \"cactus\")", "false"));
  ASSERT_TRUE(RunSuccess(Prefix + "\"durian\" \"cantelope\" \"banana\" \"apple\")", "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "5 4 3 2 3)", "false"));
  ASSERT_TRUE(RunSuccess(Prefix + "5 4 3 2 2)", "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "5 4 3 2 1)", "true"));
}

class StdLibBranchTest: public StdLibTest {
  protected:
    void TestQuotes();
};

TEST_F(StdLibBranchTest, TestIf) {
  ASSERT_TRUE(RunFail("(if)"));
  ASSERT_TRUE(RunFail("(if true)"));
  ASSERT_TRUE(RunFail("(if true 1)"));
  ASSERT_TRUE(RunSuccess("(if true 1 0)", "1"));
  ASSERT_TRUE(RunSuccess("(if false 1 0)", "0"));
  ASSERT_TRUE(RunSuccess("(if true 1 a)", "1"));
  ASSERT_TRUE(RunSuccess("(if false a 0)", "0"));
  ASSERT_TRUE(RunFail("(if true a 0)"));
  ASSERT_TRUE(RunFail("(if false 0 a)"));
  ASSERT_TRUE(RunSuccess("(if (< 3 4) \"less than\" \"greater than or equal to\")", "\"less than\""));
  ASSERT_TRUE(RunSuccess("(if (< 3 3) \"less than\" (if (> 3 3) \"greater than\" \"equal to\"))", "\"equal to\"")); 
}

TEST_F(StdLibBranchTest, TestLet) {
  ASSERT_TRUE(RunFail("(let)"));
  ASSERT_TRUE(RunFail("(let ("));
  ASSERT_TRUE(RunFail("(let true)"));
  ASSERT_TRUE(RunSuccess("(let () true)", "true"));
  ASSERT_TRUE(RunSuccess("(let () true false)", "false"));
  ASSERT_TRUE(RunFail("(let (()) true)"));
  ASSERT_TRUE(RunFail("(let ((a)) true)"));
  ASSERT_TRUE(RunFail("(let ((3)) true)"));
  ASSERT_TRUE(RunSuccess("(let ((a 3)) true)", "true"));
  ASSERT_TRUE(RunSuccess("(let ((a 3)) a)", "3"));
  ASSERT_TRUE(RunSuccess("(let ((a true)) a)", "true"));
  ASSERT_TRUE(RunSuccess("(let ((a (+ 2 1))) a)", "3"));
  ASSERT_TRUE(RunFail("(let ((a 3 4)) a)"));
  ASSERT_TRUE(RunSuccess("(let ((a 3) (a 2)) a)", "2"));
  ASSERT_TRUE(RunSuccess("(let ((a 3) (b 2)) a)", "3"));
  ASSERT_TRUE(RunSuccess("(let ((a 3) (b 2)) b)", "2"));
  ASSERT_TRUE(RunSuccess("(let ((a 3) (b 2)) (+ a b))", "5"));

  // strings
  ASSERT_TRUE(RunSuccess("(let ((a \"foo\")) a)", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(let ((a \"foo\") (b 3)) b a)", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(let ((a \"foo\") (b 3)) a b)", "3"));

  // functions
  ASSERT_TRUE(RunSuccess("(let ((add +)) (add 3 2))", "5"));

  // lists
  ASSERT_TRUE(RunSuccess("(let ((a (1 2 3))) a)", "(1 2 3)"));
  
  // nesting
  ASSERT_TRUE(RunSuccess("(let ((a 3))"
                         "  (let ((a 2))"
                         "    a))", "2"));
  ASSERT_TRUE(RunSuccess("(let ((a 3))"
                         "  (let ((b 2))"
                         "    a))", "3"));
  ASSERT_TRUE(RunSuccess("(let ((a 3))"
                         "  (let ((b 2))"
                         "    b))", "2"));
  ASSERT_TRUE(RunSuccess("(let ((a 3))"
                         "  (let ((b 2))"
                         "    (+ a b)))", "5"));
  ASSERT_TRUE(RunFail("(let ((a 3))"
                      "  (let ((b 2))"
                      "    b)"
                      "  (+ a b))"));

  ASSERT_TRUE(RunSuccess("(let ((a 3))"
                         "  (let ((b 2))"
                         "    b)"
                         "  a)", "3"));

  ASSERT_TRUE(RunSuccess("(let ((a 3) (b 2))"
                         "  (let ((c (+ a b)))"
                         "    (+ c 4)))", "9"));

  // execution
  ASSERT_TRUE(RunSuccess("(let () (set v1 4) (set v2 63))", "63"));
  ASSERT_TRUE(RunSuccess("v1", "4"));
  ASSERT_TRUE(RunSuccess("v2", "63"));

  // scoping
  ASSERT_TRUE(RunSuccess("(let ((a 2)) (set a 3))", "3"));

  // TODO: This fails. Need to keep track out current scope when doing a (set)
  //ASSERT_TRUE(RunFail("a"));

  ASSERT_TRUE(RunSuccess("(let ((v1 32)) v1)", "32"));
  ASSERT_TRUE(RunSuccess("v1", "4"));
  ASSERT_TRUE(RunSuccess("(let ((v1 32)) (set v1 33) v1)", "33"));
  //ASSERT_TRUE(RunSuccess("v1", "4"));
}

void StdLibBranchTest::TestQuotes() {
  // quotes
  ASSERT_TRUE(RunFail("(quote)"));
  ASSERT_TRUE(RunFail("(quote 2 3)"));
  ASSERT_TRUE(RunSuccess("(quote 4)", "4"));
  ASSERT_TRUE(RunSuccess("(quote \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(quote +)", "+"));
  ASSERT_TRUE(RunSuccess("(quote quote)", "quote"));
  ASSERT_TRUE(RunSuccess("(quote (+ 2 3))", "(+ 2 3)"));
  ASSERT_TRUE(RunSuccess("(quote (1 2 3))", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(quote thisdoesnotexist)", "thisdoesnotexist"));
  ASSERT_TRUE(RunSuccess("(quote (quote (+ 2 3)))", "(+ 2 3)"));
  ASSERT_TRUE(RunSuccess("(quote (quote (+ 2 3)))", "(+ 2 3)"));

  // unquotes
  ASSERT_TRUE(RunFail("(unquote)"));
  ASSERT_TRUE(RunFail("(unquote 2 3)"));
  ASSERT_TRUE(RunSuccess("(unquote (quote 4))", "4"));
  ASSERT_TRUE(RunSuccess("(unquote (quote \"foo\"))", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(unquote (quote +))", "Function"));
  ASSERT_TRUE(RunSuccess("(unquote (quote quote))", "Function"));
  ASSERT_TRUE(RunSuccess("(unquote (quote (+ 2 3)))", "5"));
  ASSERT_TRUE(RunSuccess("(unquote (quote (1 2 3)))", "(1 2 3)"));
  ASSERT_TRUE(RunFail("(unquote (quote thisdoesnotexist))"));
  ASSERT_TRUE(RunSuccess("(unquote (quote (quote (+ 2 3))))", "(+ 2 3)"));
  ASSERT_TRUE(RunSuccess("(unquote (unquote (quote (quote (+ 2 3)))))", "5"));
}

TEST_F(StdLibBranchTest, TestQuoteFn) {
  ASSERT_NO_FATAL_FAILURE(TestQuotes());
}

TEST_F(StdLibBranchTest, TestUnquote) {
  ASSERT_NO_FATAL_FAILURE(TestQuotes());
}

TEST_F(StdLibBranchTest, TestBegin) {
  ASSERT_TRUE(RunFail("(begin)"));
  ASSERT_TRUE(RunSuccess("(begin 42)", "42"));
  ASSERT_TRUE(RunSuccess("(begin \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(begin true)", "true"));
  ASSERT_TRUE(RunSuccess("(begin (set a 2) (set b 3) (+ a b))", "5"));
  ASSERT_TRUE(RunSuccess("(+ a b)", "5"));
  ASSERT_TRUE(RunFail("(begin undefined (set c 5))"));
  ASSERT_TRUE(RunFail("c"));
  ASSERT_TRUE(RunFail("(begin (set c 5) undefined)"));
  ASSERT_TRUE(RunSuccess("c", "5"));
}

TEST_F(StdLibBranchTest, TestLambda) {
}

TEST_F(StdLibBranchTest, TestDef) {
}

TEST_F(StdLibBranchTest, TestApply) {
}
