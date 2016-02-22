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
  ASSERT_TRUE(RunSuccess("3.14", "3.14"));
  ASSERT_TRUE(RunSuccess("\"foo\"", "\"foo\""));
  ASSERT_TRUE(RunSuccess("+", "Function"));
  ASSERT_TRUE(RunSuccess("true", "true"));
  ASSERT_TRUE(RunSuccess("false", "false"));
  ASSERT_TRUE(RunSuccess("nil", "()"));

  const char *args = "42 3.14 \"foo\" + (1 2 3)";
  ASSERT_TRUE(RunSuccess(args, "42"));
  ASSERT_TRUE(RunSuccess(args, "3.14"));
  ASSERT_TRUE(RunSuccess(args, "\"foo\""));
  ASSERT_TRUE(RunSuccess(args, "Function"));
  ASSERT_TRUE(RunSuccess(args, "1"));
  ASSERT_TRUE(RunSuccess(args, "2"));
  ASSERT_TRUE(RunSuccess(args, "3"));
}

TEST_F(StdLibDefaultFunctionTest, TestImplicitSexp) {
  ASSERT_TRUE(RunSuccess("not", "Function"));
  ASSERT_TRUE(RunSuccess("not false", "true"));
  ASSERT_TRUE(RunSuccess("+ 2 4", "6"));
  ASSERT_TRUE(RunSuccess("+ 2 4 8", "14"));
}

TEST_F(StdLibDefaultFunctionTest, TestInfix) {
  // Explicit sexp
  ASSERT_TRUE(RunSuccess("(2)", "(2)"));
  ASSERT_TRUE(RunSuccess("(2 +)", "(2 <Function:+>)"));

  ASSERT_TRUE(RunSuccess("(2 + 4)", "6"));
  ASSERT_TRUE(RunSuccess("(2 + 4 + 8)", "14"));
  ASSERT_TRUE(RunSuccess("(2 + 4 * 8)", "34"));
  ASSERT_TRUE(RunSuccess("(2 - 4 * 8)", "-30"));
  ASSERT_TRUE(RunSuccess("(2 help 4 help 8)", "(2 <Function:help> 4 <Function:help> 8)"));

  ASSERT_TRUE(RunSuccess("(a = 3)", "3"));
  ASSERT_TRUE(RunSuccess("(unset a)", "3"));
  ASSERT_TRUE(RunSuccess("(a = 3 + 4)", "7"));
  ASSERT_TRUE(RunSuccess("(unset a)", "7"));
  ASSERT_TRUE(RunSuccess("(a = (3 + 4))", "7"));
  ASSERT_TRUE(RunSuccess("(unset a)", "7"));
  ASSERT_TRUE(RunSuccess("(a = (+ 3 4))", "7"));
  ASSERT_TRUE(RunSuccess("(unset a)", "7"));
  ASSERT_TRUE(RunSuccess("(a = (3 + 4 + 5))", "12"));
  ASSERT_TRUE(RunSuccess("(unset a)", "12"));
  ASSERT_TRUE(RunSuccess("(a = (3 + 4 + 5 * 6))", "37"));
  ASSERT_TRUE(RunSuccess("(unset a)", "37"));
  ASSERT_TRUE(RunSuccess("(a = (3 + 4 + 5 * 6 * 7))", "217"));
  ASSERT_TRUE(RunSuccess("(unset a)", "217"));
  ASSERT_TRUE(RunSuccess("(a = (2 + 3 * 4 / 5))", "4"));
  ASSERT_TRUE(RunSuccess("(unset a)", "4"));
  ASSERT_TRUE(RunSuccess("(a = \"foo\" + \"bar\" + \"baz\")", "foobarbaz"));

  // Implicit sexp
  ASSERT_TRUE(RunSuccess("2", "2"));
  ASSERT_TRUE(RunSuccess("2 +", "(2 <Function:+>)"));

  ASSERT_TRUE(RunSuccess("2 + 4", "6"));
  ASSERT_TRUE(RunSuccess("2 + 4 + 8", "14"));

  ASSERT_TRUE(RunSuccess("2 + 4 * 8", "34"));
  ASSERT_TRUE(RunSuccess("(2 help 4 help 8)", "(2 <Function:help> 4 <Function:help> 8)"));

  ASSERT_TRUE(RunSuccess("a = 3", "3"));
  ASSERT_TRUE(RunSuccess("unset a", "3"));
  ASSERT_TRUE(RunSuccess("a = 3 + 4", "7"));
  ASSERT_TRUE(RunSuccess("unset a)", "7"));
  ASSERT_TRUE(RunSuccess("a = (3 + 4)", "7"));
  ASSERT_TRUE(RunSuccess("unset a)", "7"));
  ASSERT_TRUE(RunSuccess("a = (+ 3 4)", "7"));
  ASSERT_TRUE(RunSuccess("unset a", "7"));
  ASSERT_TRUE(RunSuccess("a = 3 + 4 + 5", "12"));
  ASSERT_TRUE(RunSuccess("unset a", "12"));
  ASSERT_TRUE(RunSuccess("a = 3 + 4 + 5 * 6", "37"));
  ASSERT_TRUE(RunSuccess("unset a", "37"));
  ASSERT_TRUE(RunSuccess("a = 3 + 4 + 5 * 6 * 7", "217"));
  ASSERT_TRUE(RunSuccess("unset a", "217"));
  ASSERT_TRUE(RunSuccess("a = 2 + 3 * 4 / 5", "4"));
  ASSERT_TRUE(RunSuccess("unset a", "4"));
  ASSERT_TRUE(RunSuccess("a = \"foo\" + \"bar\" + \"baz\"", "foobarbaz"));
}

TEST_F(StdLibDefaultFunctionTest, TestInfix_InterpretedFunction) {
  ASSERT_TRUE(RunSuccess("def myAdd (a b) (+ a b)", "Function"));
  ASSERT_TRUE(RunSuccess("def myFuncWithNoArgs () 10", "Function"));

  ASSERT_TRUE(RunSuccess("2 + 4", "6"));
  ASSERT_TRUE(RunSuccess("2 myAdd 4", "(2 <Function:myAdd> 4)"));
  ASSERT_TRUE(RunSuccess("infix-register myAdd", "()"));
  ASSERT_TRUE(RunSuccess("2 myAdd 4", "6"));
  ASSERT_TRUE(RunSuccess("2 myAdd (myFuncWithNoArgs)", "12"));

  ASSERT_TRUE(RunSuccess("infix-unregister myAdd", "()"));
  ASSERT_TRUE(RunSuccess("2 myAdd 4", "(2 <Function:myAdd> 4)"));
  ASSERT_TRUE(RunSuccess("2 myAdd (myFuncWithNoArgs)", "(2 <Function:myAdd> 10)"));
  ASSERT_TRUE(RunSuccess("2 + 4", "6"));
  ASSERT_TRUE(RunSuccess("infix-unregister +", "()"));
  ASSERT_TRUE(RunSuccess("2 + 4", "(2 <Function:+> 4)"));
}

TEST_F(StdLibDefaultFunctionTest, TestInfix_Multiline) {
  ASSERT_TRUE(RunSuccess("(3 + 4)", "7"));
  ASSERT_TRUE(RunSuccess("(\n3 + 4)", "7"));
  ASSERT_TRUE(RunSuccess("(3\n+ 4)", "7"));
  ASSERT_TRUE(RunSuccess("(3 +\n4)", "7"));
  ASSERT_TRUE(RunSuccess("(3 + 4\n)", "7"));
  ASSERT_TRUE(RunSuccess("(3\n+\n4)", "7"));
  ASSERT_TRUE(RunSuccess("(\n3\n+\n4\n)", "7"));

  ASSERT_TRUE(RunSuccess("(+ 1 2 3 4 5 6)", "21"));
  ASSERT_TRUE(RunSuccess("(1 + 2 +\n3 + 4 +\n5 + 6)", "21"));
  ASSERT_TRUE(RunSuccess("(1 + 2 +\n(3 + 4) +\n(5 + 6))", "21"));
}

TEST_F(StdLibDefaultFunctionTest, TestInfix_InsideBegin) {
  ASSERT_TRUE(RunSuccess("(begin\n2 + 4\n)", "6"));
  ASSERT_TRUE(RunSuccess("(begin\nx = 42\n)", "42"));
  ASSERT_TRUE(RunSuccess("(begin\na = 2\nb = 3\na + b\n)", "5"));
  ASSERT_TRUE(RunSuccess("(begin\n(a = 2)\n(begin\nb = 3\n(begin\na + b\n)))", "5"));
  ASSERT_TRUE(RunSuccess("(begin\n(2 + 4)\n)", "6"));
  ASSERT_TRUE(RunSuccess("(begin\n(2 + 4))", "6"));
  ASSERT_TRUE(RunSuccess("(begin\n2 + 4)", "6"));
}

class StdLibInterpreterTest: public StdLibTest {
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

class StdLibAssignmentTest: public StdLibTest {
  protected:
    void TestSetFunctions();
};

void StdLibAssignmentTest::TestSetFunctions() {
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
  ASSERT_TRUE(RunFail("(set a 42 43)"));
  ASSERT_TRUE(RunSuccess("(set b \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(set c +)", "Function"));
  ASSERT_TRUE(RunSuccess("(set d true)", "true"));
  ASSERT_TRUE(RunSuccess("(set e (1 2 3))", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(set f (quote (+ 2 3)))", "(+ 2 3)"));
  ASSERT_TRUE(RunSuccess("(set g 3.14)", "3.14"));

  ASSERT_TRUE(RunSuccess("a", "42"));
  ASSERT_TRUE(RunSuccess("b", "\"foo\""));
  ASSERT_TRUE(RunSuccess("c", "Function"));
  ASSERT_TRUE(RunSuccess("d", "true"));
  ASSERT_TRUE(RunSuccess("e", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("f", "(+ 2 3)"));
  ASSERT_TRUE(RunSuccess("g", "3.14"));

  ASSERT_TRUE(RunSuccess("(unset a)", "42"));
  ASSERT_TRUE(RunSuccess("(unset b)", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(unset c)", "Function"));
  ASSERT_TRUE(RunSuccess("(unset d)", "true"));
  ASSERT_TRUE(RunSuccess("(unset e)", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(unset f)", "(+ 2 3)"));
  ASSERT_TRUE(RunSuccess("(unset g)", "3.14"));

  ASSERT_TRUE(RunFail("a"));
  ASSERT_TRUE(RunFail("b"));
  ASSERT_TRUE(RunFail("c"));
  ASSERT_TRUE(RunFail("d"));
  ASSERT_TRUE(RunFail("e"));
  ASSERT_TRUE(RunFail("f"));
  ASSERT_TRUE(RunFail("g"));

  ASSERT_TRUE(RunFail("(unset a)"));
  ASSERT_TRUE(RunFail("(unset b)"));
  ASSERT_TRUE(RunFail("(unset c)"));
  ASSERT_TRUE(RunFail("(unset d)"));
  ASSERT_TRUE(RunFail("(unset e)"));
  ASSERT_TRUE(RunFail("(unset f)"));
  ASSERT_TRUE(RunFail("(unset g)"));

  ASSERT_TRUE(RunFail("a"));
  ASSERT_TRUE(RunFail("b"));
  ASSERT_TRUE(RunFail("c"));
  ASSERT_TRUE(RunFail("d"));
  ASSERT_TRUE(RunFail("e"));
  ASSERT_TRUE(RunFail("f"));
  ASSERT_TRUE(RunFail("g"));

  ASSERT_TRUE(RunSuccess("(set a 42)", "42"));
  ASSERT_TRUE(RunSuccess("(set b \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(set c +)", "Function"));
  ASSERT_TRUE(RunSuccess("(set d true)", "true"));
  ASSERT_TRUE(RunSuccess("(set e (1 2 3))", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(set f (quote (+ 2 3)))", "(+ 2 3)"));
  ASSERT_TRUE(RunSuccess("(set g 3.14)", "3.14"));
}

TEST_F(StdLibAssignmentTest, TestSet) {
  ASSERT_NO_FATAL_FAILURE(TestSetFunctions());
}

TEST_F(StdLibAssignmentTest, TestSetOperator) {
  ASSERT_TRUE(RunFail("n"));
  ASSERT_TRUE(RunSuccess("(= n 42)", "42"));
  ASSERT_TRUE(RunSuccess("n", "42"));
  ASSERT_TRUE(RunSuccess("(= n 102)", "102"));
  ASSERT_TRUE(RunSuccess("n", "102"));
  ASSERT_TRUE(RunSuccess("(unset n)", "102"));
  ASSERT_TRUE(RunFail("n"));

  // infix set
  ASSERT_TRUE(RunSuccess("n = 42", "42"));
  ASSERT_TRUE(RunSuccess("n", "42"));
  ASSERT_TRUE(RunSuccess("n = 102", "102"));
  ASSERT_TRUE(RunSuccess("n", "102"));
}

TEST_F(StdLibAssignmentTest, TestUnSet) {
  ASSERT_NO_FATAL_FAILURE(TestSetFunctions());
}

TEST_F(StdLibAssignmentTest, TestSetWithOpSingleArg) {
  ASSERT_TRUE(RunSuccess("a = 42", "42"));
  ASSERT_TRUE(RunSuccess("a +=", "(42 <Function:+=>)"));
  ASSERT_TRUE(RunFail("a += \"foo\""));
  ASSERT_TRUE(RunSuccess("a += 10", "52"));
  ASSERT_TRUE(RunSuccess("a -= 10", "42"));
  ASSERT_TRUE(RunSuccess("a *= 10", "420"));
  ASSERT_TRUE(RunSuccess("a /= 10", "42"));
  ASSERT_TRUE(RunSuccess("a %= 10", "2"));
  ASSERT_TRUE(RunSuccess("a <<= 10", "2048"));
  ASSERT_TRUE(RunSuccess("a >>= 10", "2"));
  ASSERT_TRUE(RunSuccess("a |= 8", "10"));
  ASSERT_TRUE(RunSuccess("a &= 8", "8"));
  ASSERT_TRUE(RunSuccess("a ^= 15", "7"));
}

TEST_F(StdLibAssignmentTest, TestSetWithOpIncrementDecrement) {
  ASSERT_TRUE(RunSuccess("a = 42", "42"));
  ASSERT_TRUE(RunSuccess("++ a", "43"));
  ASSERT_TRUE(RunSuccess("-- a", "42"));
}

class StdLibNumericalTest: public StdLibTest {
  protected:
    decltype(Int::Value) MinValue,
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
    static const char * const values[] = {"0", "1", "42"};
    std::string prefix = "(" + name + " ";
    for (auto &value : values) {
      std::string code = prefix + value + ")";
      const char *expectedValue = value;
      ASSERT_TRUE(RunSuccess(code, expectedValue));
    }
  }
};

TEST_F(StdLibNumericalTest, TestIncr) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("incr"));
  ASSERT_TRUE(RunFail("(incr \"foo\")"));
  ASSERT_TRUE(RunSuccess("(incr -2)", "-1"));
  ASSERT_TRUE(RunSuccess("(incr -1)", "0"));
  ASSERT_TRUE(RunSuccess("(incr 0)", "1"));
  ASSERT_TRUE(RunSuccess("(incr 41)", "42"));

  std::stringstream temp;
  temp << "(incr " << MaxValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), std::to_string(MinValue)));
}

TEST_F(StdLibNumericalTest, TestDecr) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("decr"));
  ASSERT_TRUE(RunSuccess("(decr -1)", "-2"));
  ASSERT_TRUE(RunSuccess("(decr 0)", "-1"));
  ASSERT_TRUE(RunSuccess("(decr 1)", "0"));
  ASSERT_TRUE(RunSuccess("(decr 42)", "41"));

  std::stringstream temp;
  temp << "(decr " << MinValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), std::to_string(MaxValue)));
}

TEST_F(StdLibNumericalTest, TestAdd) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("+"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("+"));
  ASSERT_TRUE(RunSuccess("(+ 2 3)", "5"));
  ASSERT_TRUE(RunSuccess("(+ 3.14 4.6)", "7.74"));
  ASSERT_TRUE(RunSuccess("(+ 1 2 3 4 5)", "15"));

  std::stringstream temp;
  temp << "(+ 1 " << MaxValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), std::to_string(MinValue)));
}

TEST_F(StdLibNumericalTest, TestSub) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("-"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("-"));
  ASSERT_TRUE(RunSuccess("(- 5 3)", "2"));
  double r = 3.14 - 4.0;
  ASSERT_TRUE(RunSuccess("(- 3.14 4.0)", "-0.859999"));
  ASSERT_TRUE(RunSuccess("(- 1 2 3 4 5)", "-13"));

  std::stringstream temp;
  temp << "(- 1 " << MinValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), std::to_string(MaxValue)));
}

TEST_F(StdLibNumericalTest, TestMult) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("*"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("*"));
  ASSERT_TRUE(RunSuccess("(* 2 3)", "6"));
  ASSERT_TRUE(RunSuccess("(* 3.14 2.0)", "6.28"));
  ASSERT_TRUE(RunSuccess("(* 1 2 3 4 5)", "120"));
}

TEST_F(StdLibNumericalTest, TestDiv) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("/"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("/"));
  ASSERT_TRUE(RunSuccess("(/ 6 2)", "3"));
  ASSERT_TRUE(RunSuccess("(/ 6.28 2.0)", "3.14"));
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

TEST_F(StdLibNumericalTest, TestHex) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("hex"));
  ASSERT_TRUE(RunSuccess("(hex 0)", "\"0x0\""));
  ASSERT_TRUE(RunSuccess("(hex 254)", "\"0xfe\""));
}

TEST_F(StdLibNumericalTest, TestBin) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("bin"));
  ASSERT_TRUE(RunSuccess("(bin 0)", "\"0b0\""));
  ASSERT_TRUE(RunSuccess("(bin 13)", "\"0b1101\""));
}

TEST_F(StdLibNumericalTest, TestDec) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("dec"));
  ASSERT_TRUE(RunSuccess("(dec 0)", "\"0\""));
  ASSERT_TRUE(RunSuccess("(dec 72)", "\"72\""));
}

TEST_F(StdLibNumericalTest, TestAbs) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("abs"));
  ASSERT_TRUE(RunSuccess("(abs 123)", "123"));
  ASSERT_TRUE(RunSuccess("(abs -123)", "123"));
  ASSERT_TRUE(RunSuccess("(abs 1.23)", "1.23"));
  ASSERT_TRUE(RunSuccess("(abs -1.23)", "1.23"));
  ASSERT_TRUE(RunSuccess("(abs 0)", "0"));
  ASSERT_TRUE(RunSuccess("(abs -0)", "0"));
  ASSERT_TRUE(RunSuccess("(abs 0.0)", "0"));
  ASSERT_TRUE(RunSuccess("(abs -0.0)", "0"));
}

TEST_F(StdLibNumericalTest, TestMin) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("min"));
  ASSERT_TRUE(RunSuccess("(min 23)", "23"));
  ASSERT_TRUE(RunFail("(min 23 true)"));
  ASSERT_TRUE(RunFail("(min 23 \"foo\")"));
  ASSERT_TRUE(RunFail("(min 23 3.14)"));
  ASSERT_TRUE(RunSuccess("(min 23 (int 3.14))", "3"));
  ASSERT_TRUE(RunSuccess("(min 23 34)", "23"));
  ASSERT_TRUE(RunSuccess("(min 23 34 12)", "12"));

  ASSERT_TRUE(RunSuccess("(min 2.3)", "2.29999"));
  ASSERT_TRUE(RunFail("(min 2.3 true)"));
  ASSERT_TRUE(RunFail("(min 2.3 \"foo\")"));
  ASSERT_TRUE(RunFail("(min 2.3 1)"));
  ASSERT_TRUE(RunSuccess("(min 2.3 (float 1))", "1"));
  ASSERT_TRUE(RunSuccess("(min 2.3 3.4)", "2.29999"));
  ASSERT_TRUE(RunSuccess("(min 2.3 3.4 1.2)", "1.2"));
}

TEST_F(StdLibNumericalTest, TestMax) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("max"));
  //get min tests to pass first
} 

TEST_F(StdLibNumericalTest, TestPow) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("pow"));
  ASSERT_TRUE(RunFail("(pow 0)"));
  ASSERT_TRUE(RunFail("(pow 0 false)"));
  ASSERT_TRUE(RunFail("(pow 0 \"foo\")"));
  ASSERT_TRUE(RunSuccess("(pow 0 0)", "1"));
  ASSERT_TRUE(RunSuccess("(pow 34 0)", "1"));
  ASSERT_TRUE(RunSuccess("(pow 0 1)", "0"));
  ASSERT_TRUE(RunSuccess("(pow 1 1)", "1"));
  ASSERT_TRUE(RunSuccess("(pow 1 34)", "1"));
  ASSERT_TRUE(RunSuccess("(pow 2 4)", "16"));
  ASSERT_TRUE(RunFail("(pow 2 4.2)"));
  ASSERT_TRUE(RunSuccess("(pow 2 (int 4.2))", "16"));
  //TODO: overflow tests

  ASSERT_TRUE(RunSuccess("(pow (64.0 / 81.0) (1.0 / 2.0))", ".888888888888888"));
}

TEST_F(StdLibNumericalTest, TestExp) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("exp"));
  ASSERT_TRUE(RunSuccess("(exp 2.0)", "7.38"));
}

TEST_F(StdLibNumericalTest, TestLog) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("log"));
  ASSERT_TRUE(RunSuccess("(log 2.0)", "0.69"));
}

TEST_F(StdLibNumericalTest, TestCeil) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("ceil"));
  ASSERT_TRUE(RunSuccess("(ceil 2.1)", "3"));
}

TEST_F(StdLibNumericalTest, TestFloor) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("floor"));
  ASSERT_TRUE(RunSuccess("(floor 2.9)", "2"));
}

TEST_F(StdLibNumericalTest, TestRound) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("round"));
  ASSERT_TRUE(RunSuccess("(round 2.5)", "3"));
  ASSERT_TRUE(RunSuccess("(round 2.4)", "2"));
}

TEST_F(StdLibNumericalTest, TestCos) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("cos"));
  ASSERT_TRUE(RunSuccess("(cos 2.0)", "-0.41"));
}

TEST_F(StdLibNumericalTest, TestSin) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("sin"));
  ASSERT_TRUE(RunSuccess("(sin 2.0)", "0.90"));
}

TEST_F(StdLibNumericalTest, TestTan) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("tan"));
  ASSERT_TRUE(RunSuccess("(tan 2.0)", "-2.18"));
}

TEST_F(StdLibNumericalTest, TestACos) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("acos"));
  ASSERT_TRUE(RunSuccess("(acos 0.5)", "1.04"));
}

TEST_F(StdLibNumericalTest, TestASin) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("asin"));
  ASSERT_TRUE(RunSuccess("(asin 0.5)", "0.52"));
}

TEST_F(StdLibNumericalTest, TestATan) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("atan"));
  ASSERT_TRUE(RunSuccess("(atan 0.5)", "0.46"));
}

TEST_F(StdLibNumericalTest, TestATan2) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("atan2"));
  ASSERT_TRUE(RunSuccess("(atan2 0.4 0.6)", "0.58"));
}

TEST_F(StdLibNumericalTest, TestCosh) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("cosh"));
  ASSERT_TRUE(RunSuccess("(cosh 0.5)", "1.12"));
}

TEST_F(StdLibNumericalTest, TestSinh) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("sinh"));
  ASSERT_TRUE(RunSuccess("(sinh 0.5)", "0.52"));
}

TEST_F(StdLibNumericalTest, TestTanh) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("tanh"));
  ASSERT_TRUE(RunSuccess("(tanh 0.5)", "0.46"));
}

TEST_F(StdLibNumericalTest, TestACosh) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("acosh"));
  ASSERT_TRUE(RunSuccess("(acosh 1.5)", "0.96"));
}

TEST_F(StdLibNumericalTest, TestASinh) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("asinh"));
  ASSERT_TRUE(RunSuccess("(asinh 1.5)", "1.19"));
}

TEST_F(StdLibNumericalTest, TestATanh) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("atanh"));
  ASSERT_TRUE(RunSuccess("(atanh 0.5)", "0.54"));
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

class StdLibStrTest: public StdLibTest {
};

TEST_F(StdLibStrTest, TestAdd) {
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

TEST_F(StdLibStrTest, TestReverse) {
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
  ASSERT_TRUE(RunSuccess("(map incr ())", "()"));
  ASSERT_TRUE(RunSuccess("(map incr (1))", "(2)"));
  ASSERT_TRUE(RunSuccess("(map incr (1 2 3))", "(2 3 4)"));
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
  ASSERT_TRUE(RunSuccess("(car (1 2))", "1"));
  ASSERT_TRUE(RunSuccess("(first (1 2))", "1"));
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
  ASSERT_TRUE(RunSuccess("(tail (1 2))", "(2)"));
  ASSERT_TRUE(RunSuccess("(cdr (1 2))", "(2)"));
  ASSERT_TRUE(RunSuccess("(rest (1 2))", "(2)"));
  ASSERT_TRUE(RunSuccess("(tail (tail (1 2)))", "()"));
  ASSERT_TRUE(RunSuccess("(tail ((1 2)) )", "()"));
  ASSERT_TRUE(RunSuccess("(tail (tail ((1 2)) ))", "()"));
  ASSERT_TRUE(RunSuccess("(tail (tail (tail ((1 2)) )))", "()"));
}

TEST_F(StdLibListTest, TestCons) {
  ASSERT_TRUE(RunFail("(cons)"));
  ASSERT_TRUE(RunFail("(cons 1)"));
  ASSERT_TRUE(RunFail("(cons (1))"));
  ASSERT_TRUE(RunSuccess("(cons 1 2)", "(1 2)")); // Improper lists not supported
  ASSERT_TRUE(RunSuccess("(cons (1) 2)", "(1 2)"));
  ASSERT_TRUE(RunSuccess("(cons 1 (2))", "(1 2)"));
  ASSERT_TRUE(RunSuccess("(cons (1) (2))", "(1 2)"));
  ASSERT_TRUE(RunSuccess("(cons 1 (2 3))", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(cons (1 2) 3)", "(1 2 3)"));
}

class StdLibLogicalTest: public StdLibTest {
  protected:
    std::string Prefix;
    void TestAndOr(bool isAnd);
};

void StdLibLogicalTest::TestAndOr(bool isAnd) {
  ASSERT_TRUE(RunFail(Prefix + ")"));

  ASSERT_TRUE(RunFail(Prefix + "false)"));
  ASSERT_TRUE(RunFail(Prefix + "true)"));

  ASSERT_TRUE(RunSuccess(Prefix + "false false)", isAnd ? "false" : "false"));
  ASSERT_TRUE(RunSuccess(Prefix + "true false)",  isAnd ? "false" : "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "false true)",  isAnd ? "false" : "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "true true)",   isAnd ? "true"  : "true"));

  ASSERT_TRUE(RunSuccess(Prefix + "false false false)", isAnd ? "false" : "false"));
  ASSERT_TRUE(RunSuccess(Prefix + "false true false)",  isAnd ? "false" : "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "false false true)",  isAnd ? "false" : "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "false true true)",   isAnd ? "false" : "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "true false false)",  isAnd ? "false" : "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "true true false)",   isAnd ? "false" : "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "true false true)",   isAnd ? "false" : "true"));
  ASSERT_TRUE(RunSuccess(Prefix + "true true true)",    isAnd ? "true"  : "true"));

  // Other types
  ASSERT_TRUE(RunFail(Prefix + "3 2)"));
  ASSERT_TRUE(RunFail(Prefix + "\"foo\" \"bar\")"));

  // Evaluation
  std::string evalCode = Prefix + "(< n 100) (>= n 10) (!= n 43))";
  ASSERT_TRUE(RunSuccess("(set n 42)", "42"));
  ASSERT_TRUE(RunSuccess(evalCode, isAnd ? "true" : "true"));
  ASSERT_TRUE(RunSuccess("(set n 9)", "9"));
  ASSERT_TRUE(RunSuccess(evalCode, isAnd ? "false" : "true"));
  ASSERT_TRUE(RunSuccess("(set n 101)", "101"));
  ASSERT_TRUE(RunSuccess(evalCode, isAnd ? "false" : "true"));
  ASSERT_TRUE(RunSuccess("(set n 10)", "10"));
  ASSERT_TRUE(RunSuccess(evalCode, isAnd ? "true" : "true"));
  ASSERT_TRUE(RunSuccess("(set n 43)", "43"));
  ASSERT_TRUE(RunSuccess(evalCode, isAnd ? "false" : "true")); 

  // Short circuit
  if (isAnd) {
    ASSERT_TRUE(RunFail(Prefix + "true thisisnotdefined)"));
    ASSERT_TRUE(RunSuccess(Prefix + "false thisisnotdefined)", "false"));
    ASSERT_TRUE(RunSuccess(Prefix + "false (+ 2 5))", "false"));
  }
  else {
    ASSERT_TRUE(RunSuccess(Prefix + "true thisisnotdefined)", "true"));
    ASSERT_TRUE(RunFail(Prefix + "false thisisnotdefined)"));
    ASSERT_TRUE(RunFail(Prefix + "false (+ 2 5))"));
  }
}

TEST_F(StdLibLogicalTest, TestAnd) {
  Prefix = "(and ";
  ASSERT_NO_FATAL_FAILURE(TestAndOr(true));
}

TEST_F(StdLibLogicalTest, TestOr) {
  Prefix = "(or ";
  ASSERT_NO_FATAL_FAILURE(TestAndOr(false));
}

TEST_F(StdLibLogicalTest, TestNot) {
  ASSERT_TRUE(RunFail("(not)"));
  ASSERT_TRUE(RunFail("(not 42)"));
  ASSERT_TRUE(RunFail("(not \"foo\")"));
  ASSERT_TRUE(RunFail("(not (1 2 3))"));
  ASSERT_TRUE(RunSuccess("(not false)", "true"));
  ASSERT_TRUE(RunSuccess("(not true)", "false"));
  
  std::string code = "(not (< n 100))";
  ASSERT_TRUE(RunSuccess("(set n 42)", "42"));
  ASSERT_TRUE(RunSuccess(code, "false"));
  ASSERT_TRUE(RunSuccess("(set n 420)", "420"));
  ASSERT_TRUE(RunSuccess(code, "true"));
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
      ASSERT_TRUE(RunSuccess(Prefix + "3.14)", "true"));
      ASSERT_TRUE(RunSuccess(Prefix + "\"foo\")", "true"));
  
      ASSERT_TRUE(RunSuccess(Prefix + "true false)", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "42 5)", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "3.14 3.15)", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "\"foo\" \"bar\")", value2));

      ASSERT_TRUE(RunSuccess(Prefix + "true true)", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "42 42)", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "3.14 3.14)", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "\"foo\" \"foo\")", value1));

      // Lists
      ASSERT_TRUE(RunSuccess(Prefix + "nil nil)", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "() nil)", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "nil ())", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "(1) nil)", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "nil (1))", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "(1) (1))", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "(1) (2))", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "(1 2 \"foo\") (1 2))", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "(1 2) (1 2 \"foo\"))", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "(1 2 \"foo\") (1 2 4))", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "(1 2 4) (1 2 \"foo\"))", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "(1 2 \"foo\") (1 2 \"foo\"))", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "(1 2 (3 4)) (1 2 (3 4)))", value1));

      //Implicit Number -> Bool not implemented
      //ASSERT_TRUE(RunSuccess(Prefix + "true 1)", value1));
      //ASSERT_TRUE(RunSuccess(Prefix + "1 true)", value1));
      //ASSERT_TRUE(RunSuccess(Prefix + "true 4)", "false"));
      //ASSERT_TRUE(RunSuccess(Prefix + "4 true)", "false"));

      ASSERT_TRUE(RunSuccess(Prefix + "42 (+ 40 2))", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "\"foo\" (reverse \"oof\"))", value1));

      //Comparisons are currently required to be homogeneous
      //ASSERT_TRUE(RunSuccess(Prefix + "42 \"foo\")", "false"));
      //ASSERT_TRUE(RunSuccess(Prefix + "\"foo\" nil)", "true"));
      ASSERT_TRUE(RunSuccess(Prefix + "42 \"foo\")", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "\"foo\" nil)", value2));
    }

    void TestInEquality(const char *value1, const char *value2, const char *value3) {
      ASSERT_TRUE(RunFail(Prefix + ")"));
      ASSERT_TRUE(RunSuccess(Prefix + "\"foo\")", "true"));
      ASSERT_TRUE(RunSuccess(Prefix + "2)", "true"));

      ASSERT_TRUE(RunSuccess(Prefix + "1 1)", value3));
      ASSERT_TRUE(RunSuccess(Prefix + "2 1)", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "1 2)", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "5 4 3 2 1)", value2));

      ASSERT_TRUE(RunSuccess(Prefix + "1.1 1.1)", value3));
      ASSERT_TRUE(RunSuccess(Prefix + "2.2 1.1)", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "1.1 2.2)", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "5.5 4.4 3.3 2.2 1.1)", value2));

      ASSERT_TRUE(RunSuccess(Prefix + "\"apple\" \"apple\")", value3));
      ASSERT_TRUE(RunSuccess(Prefix + "\"apple\" \"banana\")", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "\"banana\" \"apple\")", value2));
      ASSERT_TRUE(RunSuccess(Prefix + "\"apple\" \"banana\" \"cantelope\" \"durian\")", value1));

      ASSERT_TRUE(RunSuccess("(set n 42)", "42"));
      ASSERT_TRUE(RunSuccess(Prefix + "n 42)", value3));
      ASSERT_TRUE(RunSuccess(Prefix + "42 n)", value3));
      ASSERT_TRUE(RunSuccess(Prefix + "n 43)", value1));
      ASSERT_TRUE(RunSuccess(Prefix + "43 n)", value2));
    }
};

TEST_F(StdLibComparisonTest, TestEq) {
  Prefix = "(== ";
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
  ASSERT_TRUE(RunFail("(let ((a 3) (a 2)) a)")); // #20
  ASSERT_TRUE(RunSuccess("(let ((a 3) (b 2)) a)", "3"));
  ASSERT_TRUE(RunSuccess("(let ((a 3) (b 2)) b)", "2"));
  ASSERT_TRUE(RunSuccess("(let ((a 3) (b 2)) (+ a b))", "5"));

  // floats
  ASSERT_TRUE(RunSuccess("(let ((p 3.14)) p)", "3.14"));
  ASSERT_TRUE(RunSuccess("(let ((p 3.14) (q 2.7)) q p)", "3.14"));
  ASSERT_TRUE(RunSuccess("(let ((p 3.14) (q 2.7)) p q)", "2.7"));

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

  ASSERT_TRUE(RunFail("a")); // #20 

  ASSERT_TRUE(RunSuccess("(let ((v1 32)) v1)", "32"));
  ASSERT_TRUE(RunSuccess("v1", "4"));
  ASSERT_TRUE(RunSuccess("(let ((v1 32)) (set v1 33) v1)", "33"));

  ASSERT_TRUE(RunSuccess("v1", "4")); // #20
}

void StdLibBranchTest::TestQuotes() {
  // quotes
  ASSERT_TRUE(RunFail("(quote)"));
  ASSERT_TRUE(RunFail("(quote 2 3)"));
  ASSERT_TRUE(RunSuccess("(quote 4)", "4"));
  ASSERT_TRUE(RunSuccess("(quote 3.14)", "3.14"));
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
  ASSERT_TRUE(RunSuccess("(unquote (quote 3.14))", "3.14"));
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
  ASSERT_TRUE(RunFail("(lambda)"));
  ASSERT_TRUE(RunFail("(lambda true)"));
  ASSERT_TRUE(RunFail("(lambda 42)"));
  ASSERT_TRUE(RunFail("(lambda \"foo\")"));
  ASSERT_TRUE(RunFail("(lambda x)"));
  ASSERT_TRUE(RunFail("(lambda (1 2 3))"));
  ASSERT_TRUE(RunFail("(lambda (+ 2 3))"));

  ASSERT_TRUE(RunFail("(lambda ())"));
  ASSERT_TRUE(RunFail("(lambda (true))"));
  ASSERT_TRUE(RunFail("(lambda (42))"));
  ASSERT_TRUE(RunFail("(lambda (\"foo\"))"));
  ASSERT_TRUE(RunFail("(lambda (x))"));
  ASSERT_TRUE(RunFail("(lambda ((1 2 3)))"));
  ASSERT_TRUE(RunFail("(lambda ((+ 2 3)))"));

  ASSERT_TRUE(RunSuccess("(lambda () true)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda () 42)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda () 3.14)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda () \"foo\")", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda () x)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda () (1 2 3))", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda () (+ 2 3))", "Function"));

  ASSERT_TRUE(RunSuccess("(lambda (x) true)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x) 42)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x) 3.14)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x) \"foo\")", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x) x)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x) (1 2 3))", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x) (+ 2 3))", "Function"));

  ASSERT_TRUE(RunSuccess("(lambda (x y z) true)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x y z) 42)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x y z) 3.14)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x y z) \"foo\")", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x y z) x)", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x y z) (1 2 3))", "Function"));
  ASSERT_TRUE(RunSuccess("(lambda (x y z) (+ 2 3))", "Function"));

  // execution
  ASSERT_TRUE(RunSuccess("((lambda () (* 2 10)) )", "20"));
  ASSERT_TRUE(RunSuccess("((lambda (x) (* x 10)) 5)", "50"));
  ASSERT_TRUE(RunFail("((lambda (x) (* x 10)) )"));
  ASSERT_TRUE(RunFail("((lambda (x) (* x 10)) 5 6)"));
  ASSERT_TRUE(RunSuccess("((lambda (x y) (* x y)) 5 6)", "30"));
  ASSERT_TRUE(RunSuccess("((lambda (xy) (+ xy xy)) (5 6))", "(5 6 5 6)"));
  ASSERT_TRUE(RunSuccess("((lambda (xy) (list xy xy)) (5 6))", "((5 6) (5 6))")); // #21 (list) doesn't evaluate its arguments

  // fn
  ASSERT_TRUE(RunSuccess("((fn (x y) (* x y)) 5 6)", "30"));
}

TEST_F(StdLibBranchTest, TestDef) {
  ASSERT_TRUE(RunFail("(def)"));
  ASSERT_TRUE(RunFail("(def f)"));
  ASSERT_TRUE(RunFail("(def foo () )"));
  ASSERT_TRUE(RunSuccess("(def foo () true)", "Function"));
  ASSERT_TRUE(RunSuccess("(def foo () 42)", "Function"));
  ASSERT_TRUE(RunSuccess("(def foo () 3.14)", "Function"));
  ASSERT_TRUE(RunSuccess("(def add (a b) (+ a b))", "Function"));
  ASSERT_TRUE(RunSuccess("add", "Function"));
  ASSERT_TRUE(RunFail("(add)"));
  ASSERT_TRUE(RunFail("(add 2)"));
  ASSERT_TRUE(RunSuccess("(add 2 3)", "5"));
  ASSERT_TRUE(RunFail("(add 2 3 4)"));
  ASSERT_TRUE(RunSuccess("(add \"foo\" \"bar\")", "foobar"));
  ASSERT_TRUE(RunSuccess("(add (1 2) (3 4 5))", "(1 2 3 4 5)"));
  ASSERT_TRUE(RunFail("(add 3 \"foo\")"));
  ASSERT_TRUE(RunFail("(add + -)"));
}

TEST_F(StdLibBranchTest, TestApply) {
  ASSERT_TRUE(RunFail("(apply)"));
  ASSERT_TRUE(RunFail("(apply +)"));
  ASSERT_TRUE(RunFail("(apply + 1)"));
  ASSERT_TRUE(RunFail("(apply + \"foo\")"));
  ASSERT_TRUE(RunFail("(apply + false)"));
  ASSERT_TRUE(RunFail("(apply + (+ 1 2))"));
  ASSERT_TRUE(RunFail("(apply + 1 2)"));
  ASSERT_TRUE(RunFail("(apply + ())"));
  ASSERT_TRUE(RunSuccess("(apply + (1))", "1"));
  ASSERT_TRUE(RunSuccess("(apply + (1 2))", "3"));
  ASSERT_TRUE(RunSuccess("(apply + (1 2 3))", "6"));

  ASSERT_TRUE(RunFail("(apply + (1 2 a))"));
  ASSERT_TRUE(RunFail("(apply + (1 2 \"foo\"))"));
  ASSERT_TRUE(RunFail("(apply + (1 2 (3 4)))"));

  ASSERT_TRUE(RunSuccess("(apply + (\"Hello\" \", wo\" \"rld!\"))", "\"Hello, world!\""));
  ASSERT_TRUE(RunFail("(apply (fn (a b) (+ a b)) ())"));
  ASSERT_TRUE(RunFail("(apply (fn (a b) (+ a b)) (1))"));
  ASSERT_TRUE(RunSuccess("(apply (fn (a b) (+ a b)) (1 2))", "3"));
  ASSERT_TRUE(RunFail("(apply (fn (a b) (+ a b)) (1 2 3))"));
}

class StdLibOperatorsTest: public StdLibTest {
};

TEST_F(StdLibOperatorsTest, TestBool) {
  ASSERT_TRUE(RunFail("(bool)"));
  ASSERT_TRUE(RunFail("(bool 3 4)"));

  ASSERT_TRUE(RunSuccess("(bool true)", "true"));
  ASSERT_TRUE(RunSuccess("(bool false)", "false"));

  ASSERT_TRUE(RunSuccess("(bool 0)", "false"));
  ASSERT_TRUE(RunSuccess("(bool 1)", "true"));
  ASSERT_TRUE(RunSuccess("(bool 3)", "true"));
  ASSERT_TRUE(RunSuccess("(bool 3.14)", "true"));

  ASSERT_TRUE(RunSuccess("(bool \"\")", "false"));
  ASSERT_TRUE(RunSuccess("(bool \"foo\")", "true"));
  ASSERT_TRUE(RunSuccess("(bool \"0\")", "true"));
  ASSERT_TRUE(RunSuccess("(bool \"42\")", "true"));
  ASSERT_TRUE(RunSuccess("(bool \"3.14\")", "true"));
}

TEST_F(StdLibOperatorsTest, TestInt) {
  ASSERT_TRUE(RunFail("(int)"));
  ASSERT_TRUE(RunFail("(int 3 4)"));

  ASSERT_TRUE(RunSuccess("(int true)", "1"));
  ASSERT_TRUE(RunSuccess("(int false)", "0"));

  ASSERT_TRUE(RunSuccess("(int 0)", "0"));
  ASSERT_TRUE(RunSuccess("(int 1)", "1"));
  ASSERT_TRUE(RunSuccess("(int 3)", "3"));
  ASSERT_TRUE(RunSuccess("(int 3.14)", "3"));

  ASSERT_TRUE(RunSuccess("(int \"\")", "0"));
  ASSERT_TRUE(RunSuccess("(int \"foo\")", "0"));
  ASSERT_TRUE(RunSuccess("(int \"0\")", "0"));
  ASSERT_TRUE(RunSuccess("(int \"42\")", "42"));
  ASSERT_TRUE(RunSuccess("(int \"3.14\")", "3"));

  ASSERT_TRUE(RunSuccess("(int \"-42\")", "-42"));
  ASSERT_TRUE(RunSuccess("(int \"0xfe\")", "254"));
}

TEST_F(StdLibOperatorsTest, TestFloat) {
  ASSERT_TRUE(RunFail("(float)"));
  ASSERT_TRUE(RunFail("(float 3.14 4.5)"));

  ASSERT_TRUE(RunSuccess("(float true)", "1"));
  ASSERT_TRUE(RunSuccess("(float false)", "0"));

  ASSERT_TRUE(RunSuccess("(float 0)", "0"));
  ASSERT_TRUE(RunSuccess("(float 1)", "1"));
  ASSERT_TRUE(RunSuccess("(float 3)", "3"));

  ASSERT_TRUE(RunSuccess("(float \"\")", "0"));
  ASSERT_TRUE(RunSuccess("(float \"foo\")", "0"));
  ASSERT_TRUE(RunSuccess("(float \"0\")", "0"));
  ASSERT_TRUE(RunSuccess("(float \"42\")", "42"));

  ASSERT_TRUE(RunSuccess("(float \"-42\")", "-42"));
  ASSERT_TRUE(RunSuccess("(float \"0xfe\")", "254"));

  // float specific
  ASSERT_TRUE(RunSuccess("(float 3.14)", "3.14"));
  ASSERT_TRUE(RunSuccess("(float \"3.14\")", "3.14"));
}

TEST_F(StdLibOperatorsTest, TestStr) {
  ASSERT_TRUE(RunFail("(str)"));
  ASSERT_TRUE(RunFail("(str 3 4)"));

  ASSERT_TRUE(RunSuccess("(str true)", "\"true\""));
  ASSERT_TRUE(RunSuccess("(str false)", "\"false\""));

  ASSERT_TRUE(RunSuccess("(str 0)", "\"0\""));
  ASSERT_TRUE(RunSuccess("(str 1)", "\"1\""));
  ASSERT_TRUE(RunSuccess("(str 3)", "\"3\""));
  ASSERT_TRUE(RunSuccess("(str 3.14)", "\"3.14\""));

  ASSERT_TRUE(RunSuccess("(str \"\")", "\"\""));
  ASSERT_TRUE(RunSuccess("(str \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(str \"0\")", "\"0\""));
  ASSERT_TRUE(RunSuccess("(str \"42\")", "\"42\""));
  ASSERT_TRUE(RunSuccess("(str \"3.14\")", "\"3.14\""));

  ASSERT_TRUE(RunSuccess("(str \"-42\")", "\"-42\""));
  ASSERT_TRUE(RunSuccess("(str \"0xfe\")", "\"0xfe\""));
}

TEST_F(StdLibOperatorsTest, TestType) {
  ASSERT_TRUE(RunSuccess("(type 3)", "int"));
  ASSERT_TRUE(RunSuccess("(type 3.14)", "float"));
  ASSERT_TRUE(RunSuccess("(type true)", "bool"));
  ASSERT_TRUE(RunSuccess("(type false)", "bool"));
  ASSERT_TRUE(RunSuccess("(type \"foo\")", "str"));

  ASSERT_TRUE(RunSuccess("a = 3.14", "3.14"));
  ASSERT_TRUE(RunSuccess("(type a)", "float"));
}
