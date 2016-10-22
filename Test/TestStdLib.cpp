#include <sstream>
#include <string>
#include "gtest/gtest.h"

#include "Controller.h"

using namespace std;

class StdLibTest: public ::testing::Test {
  protected:
    static const vector<const char*> CmdArgs; 

    Controller Controller_;
    stringstream Out;
    int NOutputLines;
    
    explicit StdLibTest():
      Controller_(static_cast<int>(CmdArgs.size()), CmdArgs.data())
    {
      Controller_.SetOutput(Out);
    }

    void Reset() {
      Out.str("");
      Out.clear();
      NOutputLines = 0;
    }

    bool RunSuccess(const string &code, const string &expectedResult) {
      Reset();
      Controller_.Run(code);

      for (auto &ch : Out.str()) {
        if (ch == '\n')
          ++NOutputLines;      
      }

      return Out.str().find(expectedResult) != string::npos;
    }

    bool RunFail(const string &code) {
      return RunSuccess(code, "Error");
    }
};

const vector<const char*> StdLibTest::CmdArgs = {"slisp.exe", "(+ 3 4)", "arg1", "arg2"};

class StdLibEnvironmentTest: public StdLibTest {
};

TEST_F(StdLibEnvironmentTest, TestArgs) {
  ASSERT_TRUE(RunSuccess("sys.args", "(\"arg1\" \"arg2\")"));
}

TEST_F(StdLibEnvironmentTest, TestVersion) {
  ASSERT_TRUE(RunSuccess("sys.version", "Slisp"));
}

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
  ASSERT_TRUE(RunSuccess(args, "+"));
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
  ASSERT_TRUE(RunSuccess("(def myAdd (a b) (+ a b))", "Function"));
  ASSERT_TRUE(RunSuccess("(def myFuncWithNoArgs () 10)", "Function"));

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
  ASSERT_TRUE(RunSuccess("(print 42)", "42"));
  ASSERT_TRUE(RunSuccess("(print \"foo\")", "foo"));
}

TEST_F(StdLibInterpreterTest, TestPrompt) {
  ASSERT_TRUE(RunSuccess("(prompt)\nfoo\n", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(prompt \"name: \")\nJohn\n", "name: "));
  ASSERT_TRUE(RunSuccess("(prompt \"name: \")\nJohn\n", "\"John\""));
}

TEST_F(StdLibInterpreterTest, TestDisplay) {
  ASSERT_TRUE(RunSuccess("(display)", ""));
  ASSERT_TRUE(RunFail("(display a)"));
  ASSERT_TRUE(RunSuccess("(display 42)", "42"));
  ASSERT_EQ(2, NOutputLines);
  ASSERT_TRUE(RunSuccess("(display \"\")", "\"\""));
  ASSERT_TRUE(RunSuccess("(display \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(display +)", "Function"));

  const char *list = "(display (1 2 3))";
  ASSERT_TRUE(RunSuccess(list, "1"));
  ASSERT_EQ(2, NOutputLines);
  ASSERT_TRUE(RunSuccess(list, "2"));
  ASSERT_TRUE(RunSuccess(list, "3"));

  const char *unWrappedList = "(display 1 2 3)";
  ASSERT_TRUE(RunSuccess(unWrappedList, "1"));
  ASSERT_EQ(4, NOutputLines);
  ASSERT_TRUE(RunSuccess(unWrappedList, "2"));
  ASSERT_TRUE(RunSuccess(unWrappedList, "3"));

  const char *variadic = "(display 42 \"foo\" + (1 2 3))"; 
  ASSERT_TRUE(RunSuccess(variadic, "42"));
  ASSERT_EQ(5, NOutputLines);
  ASSERT_TRUE(RunSuccess(variadic, "\"foo\""));
  ASSERT_TRUE(RunSuccess(variadic, "Function"));
  ASSERT_TRUE(RunSuccess(variadic, "1"));
  ASSERT_TRUE(RunSuccess(variadic, "2"));
  ASSERT_TRUE(RunSuccess(variadic, "3"));
}

TEST_F(StdLibInterpreterTest, TestQuit) {
  stringstream in;
  in << "(+ 2 3)" << endl
     << "(+ 4 6)" << endl
     << "(quit)" << endl
     << "(+ 8 12)" << endl;
  Controller_.Run(in);
  ASSERT_NE(Out.str().find("5"), string::npos);
  ASSERT_NE(Out.str().find("10"), string::npos);
  ASSERT_EQ(Out.str().find("20"), string::npos);
}

TEST_F(StdLibInterpreterTest, TestSymbols) {
  ASSERT_TRUE(RunSuccess("(set syms (symbols))", ""));
  ASSERT_TRUE(RunSuccess("(length syms) > 20", "true"));
  ASSERT_TRUE(RunSuccess("syms", "symbols"));
}

TEST_F(StdLibInterpreterTest, TestHelp) {
  ASSERT_TRUE(RunSuccess("(help)", "help"));
  ASSERT_TRUE(RunSuccess("(help)", "+"));
  int helpAllLines = NOutputLines;
  ASSERT_GT(helpAllLines, 20);
  ASSERT_TRUE(RunFail("(help 3)"));
  ASSERT_TRUE(RunFail("(help \"foo\")"));
  ASSERT_TRUE(RunFail("(help true)"));
  ASSERT_TRUE(RunSuccess("(help *)", "*"));
  ASSERT_TRUE(RunSuccess("(help \"*\")", "*"));
  ASSERT_LT(NOutputLines, helpAllLines);
  ASSERT_TRUE(RunSuccess("(help help)", "help"));
}

TEST_F(StdLibInterpreterTest, TestHelpSignatures) {
  ASSERT_TRUE(RunSuccess("(help.signatures *)", "(* .. nums) -> num"));
  ASSERT_TRUE(RunSuccess("(help.signatures \"*\")", "(* .. nums) -> num"));
}

TEST_F(StdLibInterpreterTest, TestHelpDoc) {
  ASSERT_TRUE(RunSuccess("(help.doc *)", "multiply"));
  ASSERT_TRUE(RunSuccess("(help.doc \"*\")", "multiply"));
}

TEST_F(StdLibInterpreterTest, TestHelpExamples) {
  ASSERT_TRUE(RunSuccess("(help.examples *)", "((\"(* 2 3)\" \"6\"))"));
  ASSERT_TRUE(RunSuccess("(help.examples \"*\")", "((\"(* 2 3)\" \"6\"))"));
}

class StdLibIOTest: public StdLibTest {
protected:
  void BasicExistsDeleteTest();
  void BasicReadWriteLinesTest();
};

void StdLibIOTest::BasicExistsDeleteTest() {
  ASSERT_TRUE(RunFail("(file.exists)"));
  ASSERT_TRUE(RunFail("(file.exists 42)"));
  ASSERT_TRUE(RunFail("(file.delete)"));
  ASSERT_TRUE(RunFail("(file.delete 42)"));

  ASSERT_TRUE(RunSuccess("(set filename \"basicExistsDeleteTest.txt\")", ""));
  ASSERT_TRUE(RunSuccess("(file.exists filename)", "false"));
  ASSERT_TRUE(RunSuccess("(file.writelines filename ())", "true"));
  ASSERT_TRUE(RunSuccess("(file.exists filename)", "true"));
  ASSERT_TRUE(RunSuccess("(file.delete filename)", "true"));
  ASSERT_TRUE(RunSuccess("(file.exists filename)", "false"));
  ASSERT_TRUE(RunSuccess("(file.delete filename)", "false"));
}

TEST_F(StdLibIOTest, TestExists) {
  ASSERT_NO_FATAL_FAILURE(BasicExistsDeleteTest());
}

TEST_F(StdLibIOTest, TestDelete) {
  ASSERT_NO_FATAL_FAILURE(BasicExistsDeleteTest());
}

void StdLibIOTest::BasicReadWriteLinesTest() {
  ASSERT_TRUE(RunFail("(readlines)"));
  ASSERT_TRUE(RunFail("(readlines 42)"));

  ASSERT_TRUE(RunSuccess("(set filename \"readLinesTest.txt\")", ""));

  ASSERT_TRUE(RunSuccess("(file.exists filename)", "false"));
  ASSERT_TRUE(RunSuccess("(file.writelines filename ())", "true"));
  ASSERT_TRUE(RunSuccess("(file.exists filename)", "true"));
  ASSERT_TRUE(RunSuccess("(file.readlines filename)", "()"));

  ASSERT_TRUE(RunSuccess("(file.writelines filename (\"first line\"))", "true"));
  ASSERT_TRUE(RunSuccess("(file.readlines filename)", "(\"first line\")"));

  ASSERT_TRUE(RunSuccess("(file.writelines filename (\"first line\", \"second line\"))", "true"));
  ASSERT_TRUE(RunSuccess("(file.readlines filename)", "(\"first line\" \"second line\")"));

  ASSERT_TRUE(RunSuccess("(file.writelines filename ())", "true"));
  ASSERT_TRUE(RunSuccess("(file.readlines filename)", "()"));
}

TEST_F(StdLibIOTest, TestReadLines) {
  ASSERT_NO_FATAL_FAILURE(BasicExistsDeleteTest());
}

TEST_F(StdLibIOTest, TestWriteLines) {
  ASSERT_NO_FATAL_FAILURE(BasicExistsDeleteTest());
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
    MinValue(numeric_limits<decltype(MinValue)>::min()),
    MaxValue(numeric_limits<decltype(MaxValue)>::max())
  {
  }
  
  void TestBadNumericArgs(const string &name) {
    static const char * const values[] = {" ", "\"foo\"", "+", "a"};
    string prefix = "(" + name + " ";
    for (auto &value : values) {
      if (name != "+" || value[0] != '"') {
        string code = prefix + value + ")";
        ASSERT_TRUE(RunFail(code));
      }
    }
  }

  void TestIdentity(const string &name) {
    static const char * const values[] = {"0", "1", "42"};
    string prefix = "(" + name + " ";
    for (auto &value : values) {
      string code = prefix + value + ")";
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

  stringstream temp;
  temp << "(incr " << MaxValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), to_string(MinValue)));
}

TEST_F(StdLibNumericalTest, TestDecr) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("decr"));
  ASSERT_TRUE(RunSuccess("(decr -1)", "-2"));
  ASSERT_TRUE(RunSuccess("(decr 0)", "-1"));
  ASSERT_TRUE(RunSuccess("(decr 1)", "0"));
  ASSERT_TRUE(RunSuccess("(decr 42)", "41"));

  stringstream temp;
  temp << "(decr " << MinValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), to_string(MaxValue)));
}

TEST_F(StdLibNumericalTest, TestAdd) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("+"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("+"));
  ASSERT_TRUE(RunSuccess("(+ 2 3)", "5"));
  ASSERT_TRUE(RunSuccess("(+ 3.14 4.6)", "7.74"));
  ASSERT_TRUE(RunSuccess("(+ 1 2 3 4 5)", "15"));

  stringstream temp;
  temp << "(+ 1 " << MaxValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), to_string(MinValue)));
}

TEST_F(StdLibNumericalTest, TestSub) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("-"));
  ASSERT_NO_FATAL_FAILURE(TestIdentity("-"));
  ASSERT_TRUE(RunSuccess("(- 5 3)", "2"));
  double r = 3.14 - 4.0;
  ASSERT_TRUE(RunSuccess("(- 3.14 4.0)", "-0.859999"));
  ASSERT_TRUE(RunSuccess("(- 1 2 3 4 5)", "-13"));

  stringstream temp;
  temp << "(- 1 " << MinValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str(), to_string(MaxValue)));
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

TEST_F(StdLibNumericalTest, TestEven) {
  ASSERT_TRUE(RunFail("(even? \"2\")"));
  ASSERT_TRUE(RunFail("(even? 1.2)"));
  ASSERT_TRUE(RunFail("(even? -2)"));
  ASSERT_TRUE(RunFail("(even? -1)"));
  ASSERT_TRUE(RunSuccess("(even? 0)", "true"));
  ASSERT_TRUE(RunSuccess("(even? 1)", "false"));
  ASSERT_TRUE(RunSuccess("(even? 2)", "true"));
  ASSERT_TRUE(RunSuccess("(even? 3)", "false"));
}

TEST_F(StdLibNumericalTest, TestOdd) {
  ASSERT_TRUE(RunFail("(odd? \"2\")"));
  ASSERT_TRUE(RunFail("(odd? 1.2)"));
  ASSERT_TRUE(RunFail("(odd? -2)"));
  ASSERT_TRUE(RunFail("(odd? -1)"));
  ASSERT_TRUE(RunSuccess("(odd? 0)", "false"));
  ASSERT_TRUE(RunSuccess("(odd? 1)", "true"));
  ASSERT_TRUE(RunSuccess("(odd? 2)", "false"));
  ASSERT_TRUE(RunSuccess("(odd? 3)", "true"));
}

TEST_F(StdLibNumericalTest, TestZero) {
  ASSERT_TRUE(RunFail("(zero? \"2\")"));
  ASSERT_TRUE(RunFail("(zero? 1.2)"));
  ASSERT_TRUE(RunSuccess("(zero? -2)", "false"));
  ASSERT_TRUE(RunSuccess("(zero? -1)", "false"));
  ASSERT_TRUE(RunSuccess("(zero? 0)", "true"));
  ASSERT_TRUE(RunSuccess("(zero? 1)", "false"));
  ASSERT_TRUE(RunSuccess("(zero? 2)", "false"));
  ASSERT_TRUE(RunSuccess("(zero? 3)", "false"));
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
protected:
  void RunAtTest();
  void RunHeadTest();
  void RunLastTest();
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

TEST_F(StdLibStrTest, TestEmpty) {
  ASSERT_TRUE(RunFail("(empty?)"));
  ASSERT_TRUE(RunFail("(empty? 3)"));
  ASSERT_TRUE(RunSuccess("(empty? \"\")", "true"));
  ASSERT_TRUE(RunSuccess("(empty? \"foo bar\")", "false"));
}

TEST_F(StdLibStrTest, TestLength) {
  ASSERT_TRUE(RunFail("(length)"));
  ASSERT_TRUE(RunFail("(length 3)"));
  ASSERT_TRUE(RunSuccess("(length \"\")", "0"));
  ASSERT_TRUE(RunSuccess("(length \"foo\")", "3"));
}

TEST_F(StdLibStrTest, TestForeach) {
  ASSERT_TRUE(RunSuccess("s = \"abc\"", "abc"));
  ASSERT_TRUE(RunFail("(foreach)"));
  ASSERT_TRUE(RunFail("(foreach s)"));
  ASSERT_TRUE(RunFail("(foreach c s)"));

  ASSERT_TRUE(RunSuccess("r = \"\"", ""));
  ASSERT_TRUE(RunSuccess("(foreach c s (r = c + r))", "cba"));
  ASSERT_TRUE(RunSuccess("r", "cba"));

  ASSERT_TRUE(RunSuccess("r = \"\"", ""));
  ASSERT_TRUE(RunSuccess("(foreach c in s (r = c + r))", "cba"));
  ASSERT_TRUE(RunSuccess("r", "cba"));

  ASSERT_TRUE(RunSuccess("r = \"\"", ""));
  ASSERT_TRUE(RunSuccess("(foreach s (fn (c) (display (+ c c))))", "\"aa\"\n\"bb\"\n\"cc\""));

  //ASSERT_TRUE(RunSuccess("(foreach c in s (c = \"a\"))", "aaa"));
  //ASSERT_TRUE(RunSuccess("s", "aaa"));
}

void StdLibStrTest::RunAtTest() {
  ASSERT_TRUE(RunFail("(at)"));
  ASSERT_TRUE(RunFail("(at 4)"));
  ASSERT_TRUE(RunFail("(at \"\")"));
  ASSERT_TRUE(RunFail("(at \"abc\")"));
  ASSERT_TRUE(RunFail("(at \"abc\" 1.4)"));
  ASSERT_TRUE(RunFail("(at \"abc\" \"0\")"));
  ASSERT_TRUE(RunFail("(at \"\" 0)"));
  ASSERT_TRUE(RunFail("(at \"\" 1)"));
  ASSERT_TRUE(RunFail("(at \"\" -1)"));
  ASSERT_TRUE(RunSuccess("(at \"abc\" 0)", "a"));
  ASSERT_TRUE(RunSuccess("(at \"abc\" 1)", "b"));
  ASSERT_TRUE(RunSuccess("(at \"abc\" 2)", "c"));
  ASSERT_TRUE(RunFail("(at \"abc\" 3)"));
  ASSERT_TRUE(RunSuccess("(at \"abc\" -1)", "c"));
  ASSERT_TRUE(RunSuccess("(at \"abc\" -2)", "b"));
  ASSERT_TRUE(RunSuccess("(at \"abc\" -3)", "a"));
  ASSERT_TRUE(RunFail("(at \"abc\" -4)"));
}

TEST_F(StdLibStrTest, TestAt) {
  ASSERT_NO_FATAL_FAILURE(RunAtTest());
}

TEST_F(StdLibStrTest, TestNth) {
  ASSERT_NO_FATAL_FAILURE(RunAtTest());
  ASSERT_TRUE(RunSuccess("(nth \"abc\" 1)", "b"));
}

TEST_F(StdLibStrTest, TestSubStr) {
  ASSERT_TRUE(RunFail("(substr)"));
  ASSERT_TRUE(RunFail("(substr 3)"));
  ASSERT_TRUE(RunFail("(substr \"abc\")"));
  ASSERT_TRUE(RunFail("(substr \"abc\" -1)"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 0 -1)", "abc"));
  ASSERT_TRUE(RunFail("(substr \"abc\" 0 1 0)"));

  ASSERT_TRUE(RunSuccess("(substr \"\" 0)", ""));
  ASSERT_TRUE(RunFail("(substr \"\" 1)"));
  ASSERT_TRUE(RunSuccess("(substr \"\" 0 -1)", ""));

  ASSERT_TRUE(RunSuccess("(substr \"a\" 0)", "a"));
  ASSERT_TRUE(RunSuccess("(substr \"a\" 0 0)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"a\" 0 1)", "a"));
  ASSERT_TRUE(RunSuccess("(substr \"a\" 0 2)", "a"));
  ASSERT_TRUE(RunSuccess("(substr \"a\" 1)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"a\" 1 0)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"a\" 1 1)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"a\" 1 2)", ""));
  ASSERT_TRUE(RunFail("(substr \"a\" 2 0)"));

  ASSERT_TRUE(RunSuccess("(substr \"ab\" 0)", "ab"));
  ASSERT_TRUE(RunSuccess("(substr \"ab\" 0 0)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"ab\" 0 1)", "a"));
  ASSERT_TRUE(RunSuccess("(substr \"ab\" 0 2)", "ab"));
  ASSERT_TRUE(RunSuccess("(substr \"ab\" 0 3)", "ab"));
  ASSERT_TRUE(RunSuccess("(substr \"ab\" 1)", "b"));
  ASSERT_TRUE(RunSuccess("(substr \"ab\" 1 0)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"ab\" 1 1)", "b"));
  ASSERT_TRUE(RunSuccess("(substr \"ab\" 1 2)", "b"));
  ASSERT_TRUE(RunSuccess("(substr \"ab\" 2 0)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"ab\" 2 1)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"ab\" 2 2)", ""));
  ASSERT_TRUE(RunFail("(substr \"ab\" 3 0)"));

  ASSERT_TRUE(RunSuccess("(substr \"abc\" 0)", "abc"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 0 0)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 0 1)", "a"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 0 2)", "ab"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 0 3)", "abc"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 0 4)", "abc"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 1)", "bc"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 1 0)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 1 1)", "b"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 1 2)", "bc"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 1 3)", "bc"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 2)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 2 0)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 2 1)", "c"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 2 2)", "c"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 2 3)", "c"));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 3)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 3 0)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 3 1)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 3 2)", ""));
  ASSERT_TRUE(RunSuccess("(substr \"abc\" 3 3)", ""));
  ASSERT_TRUE(RunFail("(substr \"abc\" 4)"));
}

TEST_F(StdLibStrTest, TestCompare) {
  ASSERT_TRUE(RunFail("(compare)"));
  ASSERT_TRUE(RunFail("(compare 3 3)"));
  ASSERT_TRUE(RunFail("(compare \"aa\")"));
  ASSERT_TRUE(RunFail("(compare \"aa\" \"bb\" \"cc\")"));
  ASSERT_TRUE(RunSuccess("(compare \"\" \"\")", "0"));
  ASSERT_TRUE(RunSuccess("(compare \"a\" \"\")", "1"));
  ASSERT_TRUE(RunSuccess("(compare \"\" \"b\")", "-1"));
  ASSERT_TRUE(RunSuccess("(compare \"a\" \"a\")", "0"));
}

TEST_F(StdLibStrTest, TestContains) {
  ASSERT_TRUE(RunFail("(contains)"));
  ASSERT_TRUE(RunFail("(contains 3 3)"));
  ASSERT_TRUE(RunFail("(contains \"aa\")"));
  ASSERT_TRUE(RunFail("(contains \"aa\" \"bb\" \"cc\")"));
  ASSERT_TRUE(RunSuccess("(contains \"\" \"\")", "true"));
  ASSERT_TRUE(RunSuccess("(contains \"a\" \"\")", "true"));
  ASSERT_TRUE(RunSuccess("(contains \"\" \"a\")", "false"));
  ASSERT_TRUE(RunSuccess("(contains \"a\" \"a\")", "true"));
  ASSERT_TRUE(RunSuccess("(contains \"b\" \"a\")", "false"));
  ASSERT_TRUE(RunSuccess("(contains \"a\" \"b\")", "false"));
  ASSERT_TRUE(RunSuccess("(contains \"ab\" \"b\")", "true"));
  ASSERT_TRUE(RunSuccess("(contains \"a\" \"ab\")", "false"));

  ASSERT_TRUE(RunSuccess("(contains \"abc\" \"a\")", "true"));
  ASSERT_TRUE(RunSuccess("(contains \"abc\" \"b\")", "true"));
  ASSERT_TRUE(RunSuccess("(contains \"abc\" \"c\")", "true"));
  ASSERT_TRUE(RunSuccess("(contains \"abc\" \"ab\")", "true"));
  ASSERT_TRUE(RunSuccess("(contains \"abc\" \"bc\")", "true"));
  ASSERT_TRUE(RunSuccess("(contains \"abc\" \"ac\")", "false"));
  ASSERT_TRUE(RunSuccess("(contains \"abc\" \"abc\")", "true"));
  ASSERT_TRUE(RunSuccess("(contains \"abc\" \"abcd\")", "false"));

  ASSERT_TRUE(RunSuccess("(contains \"abab\" \"ab\")", "true"));
}

TEST_F(StdLibStrTest, TestStartsWith) {
  ASSERT_TRUE(RunFail("(startswith)"));
  ASSERT_TRUE(RunFail("(startswith 3 3)"));
  ASSERT_TRUE(RunFail("(startswith \"aa\")"));
  ASSERT_TRUE(RunFail("(startswith \"aa\" \"bb\" \"cc\")"));
  ASSERT_TRUE(RunSuccess("(startswith \"\" \"\")", "true"));
  ASSERT_TRUE(RunSuccess("(startswith \"a\" \"\")", "true"));
  ASSERT_TRUE(RunSuccess("(startswith \"\" \"a\")", "false"));
  ASSERT_TRUE(RunSuccess("(startswith \"a\" \"a\")", "true"));
  ASSERT_TRUE(RunSuccess("(startswith \"b\" \"a\")", "false"));
  ASSERT_TRUE(RunSuccess("(startswith \"a\" \"b\")", "false"));
  ASSERT_TRUE(RunSuccess("(startswith \"ab\" \"b\")", "false"));
  ASSERT_TRUE(RunSuccess("(startswith \"a\" \"ab\")", "false"));

  ASSERT_TRUE(RunSuccess("(startswith \"abc\" \"a\")", "true"));
  ASSERT_TRUE(RunSuccess("(startswith \"abc\" \"b\")", "false"));
  ASSERT_TRUE(RunSuccess("(startswith \"abc\" \"c\")", "false"));
  ASSERT_TRUE(RunSuccess("(startswith \"abc\" \"ab\")", "true"));
  ASSERT_TRUE(RunSuccess("(startswith \"abc\" \"bc\")", "false"));
  ASSERT_TRUE(RunSuccess("(startswith \"abc\" \"ac\")", "false"));
  ASSERT_TRUE(RunSuccess("(startswith \"abc\" \"abc\")", "true"));
  ASSERT_TRUE(RunSuccess("(startswith \"abc\" \"abcd\")", "false"));

  ASSERT_TRUE(RunSuccess("(startswith \"abab\" \"ab\")", "true"));
}

TEST_F(StdLibStrTest, TestEndsWith) {
  ASSERT_TRUE(RunFail("(endswith)"));
  ASSERT_TRUE(RunFail("(endswith 3 3)"));
  ASSERT_TRUE(RunFail("(endswith \"aa\")"));
  ASSERT_TRUE(RunFail("(endswith \"aa\" \"bb\" \"cc\")"));
  ASSERT_TRUE(RunSuccess("(endswith \"\" \"\")", "true"));
  ASSERT_TRUE(RunSuccess("(endswith \"a\" \"\")", "true"));
  ASSERT_TRUE(RunSuccess("(endswith \"\" \"a\")", "false"));
  ASSERT_TRUE(RunSuccess("(endswith \"a\" \"a\")", "true"));
  ASSERT_TRUE(RunSuccess("(endswith \"b\" \"a\")", "false"));
  ASSERT_TRUE(RunSuccess("(endswith \"a\" \"b\")", "false"));
  ASSERT_TRUE(RunSuccess("(endswith \"ab\" \"b\")", "true"));
  ASSERT_TRUE(RunSuccess("(endswith \"a\" \"ab\")", "false"));

  ASSERT_TRUE(RunSuccess("(endswith \"abc\" \"a\")", "false"));
  ASSERT_TRUE(RunSuccess("(endswith \"abc\" \"b\")", "false"));
  ASSERT_TRUE(RunSuccess("(endswith \"abc\" \"c\")", "true"));
  ASSERT_TRUE(RunSuccess("(endswith \"abc\" \"ab\")", "false"));
  ASSERT_TRUE(RunSuccess("(endswith \"abc\" \"bc\")", "true"));
  ASSERT_TRUE(RunSuccess("(endswith \"abc\" \"ac\")", "false"));
  ASSERT_TRUE(RunSuccess("(endswith \"abc\" \"abc\")", "true"));
  ASSERT_TRUE(RunSuccess("(endswith \"abc\" \"abcd\")", "false"));

  ASSERT_TRUE(RunSuccess("(endswith \"abab\" \"ab\")", "true"));
}

TEST_F(StdLibStrTest, TestFind) {
  ASSERT_TRUE(RunFail("(find)"));
  ASSERT_TRUE(RunFail("(find 3 3)"));
  ASSERT_TRUE(RunFail("(find \"aa\")"));
  ASSERT_TRUE(RunFail("(find \"aa\" \"bb\" \"cc\")"));
  ASSERT_TRUE(RunSuccess("(find \"\" \"\")", "0"));
  ASSERT_TRUE(RunSuccess("(find \"a\" \"\")", "0"));
  ASSERT_TRUE(RunSuccess("(find \"\" \"a\")", "-1"));
  ASSERT_TRUE(RunSuccess("(find \"a\" \"a\")", "0"));
  ASSERT_TRUE(RunSuccess("(find \"b\" \"a\")", "-1"));
  ASSERT_TRUE(RunSuccess("(find \"a\" \"b\")", "-1"));
  ASSERT_TRUE(RunSuccess("(find \"ab\" \"b\")", "1"));
  ASSERT_TRUE(RunSuccess("(find \"a\" \"ab\")", "-1"));

  ASSERT_TRUE(RunSuccess("(find \"abc\" \"a\")", "0"));
  ASSERT_TRUE(RunSuccess("(find \"abc\" \"b\")", "1"));
  ASSERT_TRUE(RunSuccess("(find \"abc\" \"c\")", "2"));
  ASSERT_TRUE(RunSuccess("(find \"abc\" \"ab\")", "0"));
  ASSERT_TRUE(RunSuccess("(find \"abc\" \"bc\")", "1"));
  ASSERT_TRUE(RunSuccess("(find \"abc\" \"ac\")", "-1"));
  ASSERT_TRUE(RunSuccess("(find \"abc\" \"abc\")", "0"));
  ASSERT_TRUE(RunSuccess("(find \"abc\" \"abcd\")", "-1"));

  ASSERT_TRUE(RunSuccess("(find \"abab\" \"ab\")", "0"));

  // start
  ASSERT_TRUE(RunSuccess("(find \"abab\" \"ab\" -1)", "-1"));
  ASSERT_TRUE(RunSuccess("(find \"abab\" \"ab\" 0)", "0"));
  ASSERT_TRUE(RunSuccess("(find \"abab\" \"ab\" 1)", "2"));
  ASSERT_TRUE(RunSuccess("(find \"abab\" \"ab\" 2)", "2"));
  ASSERT_TRUE(RunSuccess("(find \"abab\" \"ab\" 3)", "-1"));
  ASSERT_TRUE(RunSuccess("(find \"abab\" \"ab\" 4)", "-1"));
}

TEST_F(StdLibStrTest, TestRFind) {
  ASSERT_TRUE(RunFail("(rfind)"));
  ASSERT_TRUE(RunFail("(rfind 3 3)"));
  ASSERT_TRUE(RunFail("(rfind \"aa\")"));
  ASSERT_TRUE(RunFail("(rfind \"aa\" \"bb\" \"cc\")"));
  ASSERT_TRUE(RunSuccess("(rfind \"\" \"\")", "0"));
  ASSERT_TRUE(RunSuccess("(rfind \"a\" \"\")", "1"));
  ASSERT_TRUE(RunSuccess("(rfind \"\" \"a\")", "-1"));
  ASSERT_TRUE(RunSuccess("(rfind \"a\" \"a\")", "0"));
  ASSERT_TRUE(RunSuccess("(rfind \"b\" \"a\")", "-1"));
  ASSERT_TRUE(RunSuccess("(rfind \"a\" \"b\")", "-1"));
  ASSERT_TRUE(RunSuccess("(rfind \"ab\" \"b\")", "1"));
  ASSERT_TRUE(RunSuccess("(rfind \"a\" \"ab\")", "-1"));

  ASSERT_TRUE(RunSuccess("(rfind \"abc\" \"a\")", "0"));
  ASSERT_TRUE(RunSuccess("(rfind \"abc\" \"b\")", "1"));
  ASSERT_TRUE(RunSuccess("(rfind \"abc\" \"c\")", "2"));
  ASSERT_TRUE(RunSuccess("(rfind \"abc\" \"ab\")", "0"));
  ASSERT_TRUE(RunSuccess("(rfind \"abc\" \"bc\")", "1"));
  ASSERT_TRUE(RunSuccess("(rfind \"abc\" \"ac\")", "-1"));
  ASSERT_TRUE(RunSuccess("(rfind \"abc\" \"abc\")", "0"));
  ASSERT_TRUE(RunSuccess("(rfind \"abc\" \"abcd\")", "-1"));

  ASSERT_TRUE(RunSuccess("(rfind \"abab\" \"ab\")", "2"));

  // start
  ASSERT_TRUE(RunSuccess("(rfind \"abab\" \"ab\" -1)", "2"));
  ASSERT_TRUE(RunSuccess("(rfind \"abab\" \"ab\" 0)", "0"));
  ASSERT_TRUE(RunSuccess("(rfind \"abab\" \"ab\" 1)", "0"));
  ASSERT_TRUE(RunSuccess("(rfind \"abab\" \"ab\" 2)", "2"));
  ASSERT_TRUE(RunSuccess("(rfind \"abab\" \"ab\" 3)", "2"));
  ASSERT_TRUE(RunSuccess("(rfind \"abab\" \"ab\" 4)", "2"));
}

TEST_F(StdLibStrTest, TestReplace) {
  ASSERT_TRUE(RunFail("(replace)"));
  ASSERT_TRUE(RunFail("(replace 3)"));

  ASSERT_TRUE(RunSuccess("(replace \"\" \"\" \"\" 0)", ""));
  ASSERT_TRUE(RunSuccess("(replace \"\" \"\" \"\" 1)", ""));
  ASSERT_TRUE(RunSuccess("(replace \"\" \"\" \"a\" 1)", "a"));
  ASSERT_TRUE(RunSuccess("(replace \"a\" \"\" \"a\" 1)", "a"));
  ASSERT_TRUE(RunSuccess("(replace \"a\" \"\" \"b\" 1)", "a"));
  ASSERT_TRUE(RunSuccess("(replace \"a\" \"a\" \"\" 1)", ""));
  ASSERT_TRUE(RunSuccess("(replace \"ab\" \"a\" \"\" 1)", "b"));
  ASSERT_TRUE(RunSuccess("(replace \"ab\" \"a\" \"bc\" 1)", "bcb"));

  ASSERT_TRUE(RunSuccess("(replace \"abYab\" \"ab\" \"Z\" -1)", "ZYZ"));
  ASSERT_TRUE(RunSuccess("(replace \"abYab\" \"ab\" \"Z\" 0)", "abYab"));
  ASSERT_TRUE(RunSuccess("(replace \"abYab\" \"ab\" \"Z\" 1)", "ZYab"));
  ASSERT_TRUE(RunSuccess("(replace \"abYab\" \"ab\" \"Z\" 2)", "ZYZ"));
  ASSERT_TRUE(RunSuccess("(replace \"abYab\" \"ab\" \"Z\" 3)", "ZYZ"));

  ASSERT_TRUE(RunSuccess("(replace \"abYab\" \"ab\" \"Z\")", "ZYZ"));
  ASSERT_TRUE(RunSuccess("(replace \"abYab\" \"ab\" \"\")", "Y"));

  ASSERT_TRUE(RunSuccess("(replace \"abYab\" \"ab\" \"\")", "Y"));

  ASSERT_TRUE(RunSuccess("(replace \"abab\" \"ab\" \"Z\")", "ZZ"));
  ASSERT_TRUE(RunSuccess("(replace \"abYYab\" \"ab\" \"Z\")", "ZYYZ"));
  ASSERT_TRUE(RunSuccess("(replace \"abab\" \"ab\" \"ab\")", "abab"));
}

TEST_F(StdLibStrTest, TestSplit) {
  ASSERT_TRUE(RunFail("(split)"));
  ASSERT_TRUE(RunFail("(split 3)"));
  ASSERT_TRUE(RunFail("(split \"foo\")"));
  ASSERT_TRUE(RunFail("(split \"foo\" 3)"));

  ASSERT_TRUE(RunSuccess("(split \"\" \"\")", "()"));
  ASSERT_TRUE(RunSuccess("(split \"a\" \"\")", "(\"a\")"));
  ASSERT_TRUE(RunSuccess("(split \"\" \"a\")", "()"));

  ASSERT_TRUE(RunSuccess("(split \"a\" \":\")", "(\"a\")"));
  ASSERT_TRUE(RunSuccess("(split \":\" \":\")", "()"));
  ASSERT_TRUE(RunSuccess("(split \"a:\" \":\")", "(\"a\")"));
  ASSERT_TRUE(RunSuccess("(split \":a\" \":\")", "(\"a\")"));
  ASSERT_TRUE(RunSuccess("(split \"a:b\" \":\")", "(\"a\" \"b\")"));
  ASSERT_TRUE(RunSuccess("(split \":a:b:\" \":\")", "(\"a\" \"b\")"));
  ASSERT_TRUE(RunSuccess("(split \":::a::b::::\" \":\")", "(\"a\" \"b\")"));

  ASSERT_TRUE(RunSuccess("(split \"a\" \":\") false", "(\"a\")"));
  ASSERT_TRUE(RunSuccess("(split \":\" \":\" false)", "(\"\" \"\")"));
  ASSERT_TRUE(RunSuccess("(split \"a:\" \":\" false)", "(\"a\" \"\")"));
  ASSERT_TRUE(RunSuccess("(split \":a\" \":\" false)", "(\"\" \"a\")"));
  ASSERT_TRUE(RunSuccess("(split \"a:b\" \":\" false)", "(\"a\" \"b\")"));
  ASSERT_TRUE(RunSuccess("(split \":a:b:\" \":\" false)", "(\"\" \"a\" \"b\" \"\")"));
  ASSERT_TRUE(RunSuccess("(split \":::a::b::::\" \":\" false)", "(\"\" \"\" \"\" \"a\" \"\" \"b\" \"\" \"\" \"\" \"\")"));
}

TEST_F(StdLibStrTest, TestJoin) {
  ASSERT_TRUE(RunFail("(join)"));
  ASSERT_TRUE(RunFail("(join 3)"));
  ASSERT_TRUE(RunFail("(join \"foo\")"));
  ASSERT_TRUE(RunFail("(join \"foo\" 3)"));

  ASSERT_TRUE(RunSuccess("(join () \"\")", "\"\""));
  ASSERT_TRUE(RunSuccess("(join (\"a\") \"\")", "\"a\""));
  ASSERT_TRUE(RunSuccess("(join () \"a\")", "\"\""));

  ASSERT_TRUE(RunSuccess("(join (\"a\") \":\")", "\"a\""));
  ASSERT_TRUE(RunSuccess("(join () \":\")", "\"\""));
  ASSERT_TRUE(RunSuccess("(join (\"a\" \"b\") \":\")", "\"a:b\""));

  ASSERT_TRUE(RunSuccess("(join (\"a\") \":\") false", "\"a\""));
  ASSERT_TRUE(RunSuccess("(join (\"\" \"\") \":\" false)", "\":\""));
  ASSERT_TRUE(RunSuccess("(join (\"a\" \"\") \":\" false)", "\"a:\""));
  ASSERT_TRUE(RunSuccess("(join (\"\" \"a\") \":\" false)", "\":a\""));
  ASSERT_TRUE(RunSuccess("(join (\"a\" \"b\") \":\" false)", "\"a:b\""));
  ASSERT_TRUE(RunSuccess("(join (\"\" \"a\" \"b\" \"\") \":\" false)", "\":a:b:\""));
  ASSERT_TRUE(RunSuccess("(join (\"\" \"\" \"\" \"a\" \"\" \"b\" \"\" \"\" \"\" \"\") \":\" false)", "\":::a::b::::\""));

  ASSERT_TRUE(RunFail("(join (1) \":\")"));
}

TEST_F(StdLibStrTest, TestFormat) {
  ASSERT_TRUE(RunFail("(format)"));
  ASSERT_TRUE(RunFail("(format 3)"));

  ASSERT_TRUE(RunSuccess("(format \"\")", "\"\""));
  ASSERT_TRUE(RunSuccess("(format \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunFail("(format \"{\")"));
  ASSERT_TRUE(RunSuccess("(format \"{{\")", "\"{\""));
  ASSERT_TRUE(RunFail("(format \"}\")"));
  ASSERT_TRUE(RunSuccess("(format \"}}\")", "\"}\""));
  ASSERT_TRUE(RunSuccess("(format \"foo}}\")", "\"foo}\""));
  ASSERT_TRUE(RunSuccess("(format \"foo}}bar\")", "\"foo}bar\""));
  ASSERT_TRUE(RunSuccess("(format \"{{}}\")", "\"{}\""));
  ASSERT_TRUE(RunSuccess("(format \"{}\" \"bar\")", "\"bar\""));
  ASSERT_TRUE(RunSuccess("(format \"{{}}\" \"bar\")", "\"{}\""));
  ASSERT_TRUE(RunSuccess("(format \"{{{}}}\" \"bar\")", "\"{bar}\""));

  ASSERT_TRUE(RunSuccess("(format \"{{this should be enclosed in curlies}}\")", "\"{this should be enclosed in curlies}\""));
  ASSERT_TRUE(RunSuccess("(format \"hello, {} {}!\" \"john\" \"doe\")", "\"hello, john doe!\""));

  ASSERT_TRUE(RunSuccess("(format \"hello, {{{} {}}}!\" \"john\" \"doe\")", "\"hello, {john doe}!\""));


  ASSERT_TRUE(RunSuccess("(format \"hello, {0} {1}!\" \"john\" \"doe\")", "\"hello, john doe!\""));
  ASSERT_TRUE(RunSuccess("firstName = \"john\"", "john"));
  ASSERT_TRUE(RunSuccess("lastName = \"doe\"", "doe"));
  ASSERT_TRUE(RunSuccess("(format \"hello, {firstName} {lastName}!\")", "\"hello, john doe!\""));
}

void StdLibStrTest::RunHeadTest() {
  ASSERT_TRUE(RunFail("(head)"));
  ASSERT_TRUE(RunFail("(head 4)"));
  ASSERT_TRUE(RunSuccess("(head \"\")", ""));
  ASSERT_TRUE(RunSuccess("(head \"a\")", "a"));
  ASSERT_TRUE(RunSuccess("(head \"abc\")", "a"));
}

TEST_F(StdLibStrTest, TestHead) {
  ASSERT_NO_FATAL_FAILURE(RunHeadTest());
}

TEST_F(StdLibStrTest, TestFirst) {
  ASSERT_NO_FATAL_FAILURE(RunHeadTest());
  ASSERT_TRUE(RunSuccess("(first \"abc\")", "a"));
}

TEST_F(StdLibStrTest, TestFront) {
  ASSERT_NO_FATAL_FAILURE(RunHeadTest());
  ASSERT_TRUE(RunSuccess("(front \"abc\")", "a"));
}

TEST_F(StdLibStrTest, TestTail) {
  ASSERT_TRUE(RunFail("(tail)"));
  ASSERT_TRUE(RunFail("(tail 4)"));
  ASSERT_TRUE(RunSuccess("(tail \"\")", ""));
  ASSERT_TRUE(RunSuccess("(tail \"a\")", ""));
  ASSERT_TRUE(RunSuccess("(tail \"abc\")", "bc"));
}

void StdLibStrTest::RunLastTest() {
  ASSERT_TRUE(RunFail("(last)"));
  ASSERT_TRUE(RunFail("(last 4)"));
  ASSERT_TRUE(RunSuccess("(last \"\")", ""));
  ASSERT_TRUE(RunSuccess("(last \"a\")", "a"));
  ASSERT_TRUE(RunSuccess("(last \"abc\")", "c"));
}

TEST_F(StdLibStrTest, TestLast) {
  ASSERT_NO_FATAL_FAILURE(RunLastTest());
}

TEST_F(StdLibStrTest, TestBack) {
  ASSERT_NO_FATAL_FAILURE(RunLastTest());
  ASSERT_TRUE(RunSuccess("(back \"abc\")", "c"));
}

TEST_F(StdLibStrTest, TestTrim) {
  ASSERT_TRUE(RunFail("(trim)"));
  ASSERT_TRUE(RunFail("(trim 2)"));
  ASSERT_TRUE(RunFail("(trim (1))"));
  ASSERT_TRUE(RunSuccess("(trim \"\")", ""));
  ASSERT_TRUE(RunSuccess("(trim \"a\")", "a"));
  ASSERT_TRUE(RunSuccess("(trim \" \")", ""));
  ASSERT_TRUE(RunSuccess("(trim \"a \")", "a"));
  ASSERT_TRUE(RunSuccess("(trim \" a\")", "a"));
  ASSERT_TRUE(RunSuccess("(trim \"a b\")", "a b"));
  ASSERT_TRUE(RunSuccess("(trim \"   a b c   \")", "a b"));
}

TEST_F(StdLibStrTest, TestUpper) {
  ASSERT_TRUE(RunFail("(upper)"));
  ASSERT_TRUE(RunFail("(upper 2)"));
  ASSERT_TRUE(RunFail("(upper (1))"));
  ASSERT_TRUE(RunSuccess("(upper \"\")", ""));
  ASSERT_TRUE(RunSuccess("(upper \"a\")", "A"));
  ASSERT_TRUE(RunSuccess("(upper \"z\")", "Z"));
  ASSERT_TRUE(RunSuccess("(upper \"A\")", "A"));
  ASSERT_TRUE(RunSuccess("(upper \"1\")", "1"));
  ASSERT_TRUE(RunSuccess("(upper \"1a2b3C4,\")", "1A2B3C4"));
}

TEST_F(StdLibStrTest, TestLower) {
  ASSERT_TRUE(RunFail("(lower)"));
  ASSERT_TRUE(RunFail("(lower 2)"));
  ASSERT_TRUE(RunFail("(lower (1))"));
  ASSERT_TRUE(RunSuccess("(lower \"\")", ""));
  ASSERT_TRUE(RunSuccess("(lower \"A\")", "a"));
  ASSERT_TRUE(RunSuccess("(lower \"Z\")", "z"));
  ASSERT_TRUE(RunSuccess("(lower \"a\")", "a"));
  ASSERT_TRUE(RunSuccess("(lower \"1\")", "1"));
  ASSERT_TRUE(RunSuccess("(lower \"1A2B3c4,\")", "1a2b3c4"));
}

class StdLibListTest: public StdLibTest {
protected:
  void RunAtTest() {
    ASSERT_TRUE(RunFail("(at (10 20 30))"));
    ASSERT_TRUE(RunFail("(at (10 20 30) 1.5)"));
    ASSERT_TRUE(RunFail("(at () 0)"));
    ASSERT_TRUE(RunFail("(at () 1)"));
    ASSERT_TRUE(RunSuccess("(at (10 20 30) 0)", "10"));
    ASSERT_TRUE(RunSuccess("(at (10 20 30) 1)", "20"));
    ASSERT_TRUE(RunSuccess("(at (10 20 30) 2)", "30"));
    ASSERT_TRUE(RunFail("(at (10 20 30) 3)"));
    ASSERT_TRUE(RunSuccess("(at (10 20 30) -1)", "30"));
    ASSERT_TRUE(RunSuccess("(at (10 20 30) -2)", "20"));
    ASSERT_TRUE(RunSuccess("(at (10 20 30) -3)", "10"));
    ASSERT_TRUE(RunFail("(at (10 20 30) -4)"));
  }
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
  ASSERT_TRUE(RunSuccess("(\"a\" (format \"\"))", "(\"a\" \"\")"));
  ASSERT_TRUE(RunSuccess("(list \"a\" (format \"\"))", "(\"a\" \"\")"));
  ASSERT_TRUE(RunSuccess("'(\"a\" (format \"\"))", "(\"a\" (format \"\"))"));
  ASSERT_TRUE(RunSuccess("'(list \"a\" (format \"\"))", "(list \"a\" (format \"\"))"));
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

TEST_F(StdLibListTest, TestFilter) {
  ASSERT_TRUE(RunFail("(filter)"));
  ASSERT_TRUE(RunFail("(filter even?)"));
  ASSERT_TRUE(RunFail("(filter even? 42)"));
  ASSERT_TRUE(RunFail("(filter even? (\"foo\"))"));
  ASSERT_TRUE(RunSuccess("(filter even? ())", "()"));
  ASSERT_TRUE(RunSuccess("(filter even? (0))", "(0)"));
  ASSERT_TRUE(RunSuccess("(filter even? (1))", "()"));
  ASSERT_TRUE(RunSuccess("(filter even? (0 1 2 3))", "(0 2)"));
  ASSERT_TRUE(RunFail("(filter even? (0 1 \"foo\" 3))"));
  ASSERT_TRUE(RunSuccess("(filter even? (0 .. 4))", "(0 2 4)"));
  ASSERT_TRUE(RunFail("(filter (fn () true) (0 .. 4))"));
  ASSERT_TRUE(RunSuccess("(filter (fn (a) true) (0 .. 4))", "(0 1 2 3 4)"));
  ASSERT_TRUE(RunSuccess("(filter (fn (a) false) (0 .. 4))", "()"));
  ASSERT_TRUE(RunSuccess("(filter (fn (a) (a == 3)) (0 .. 4))", "(3)"));
  ASSERT_TRUE(RunFail("(filter (fn (a b) true) (0 .. 4))"));
}

TEST_F(StdLibListTest, TestReduce) {
  ASSERT_TRUE(RunFail("(reduce)"));
  ASSERT_TRUE(RunFail("(reduce +))"));
  ASSERT_TRUE(RunFail("(reduce + ())"));
  ASSERT_TRUE(RunSuccess("(reduce - (\"foo\"))", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(reduce + (1))", "1"));
  ASSERT_TRUE(RunSuccess("(reduce + (1 2))", "3"));
  ASSERT_TRUE(RunSuccess("(reduce + (1 2 3))", "6"));
  ASSERT_TRUE(RunFail("(reduce + (1 2 \"foo\"))"));
  ASSERT_TRUE(RunSuccess("(reduce (fn (a b) (a + b)) (1 2 3))", "6"));
  ASSERT_TRUE(RunSuccess("(reduce + (1 .. 100))", "5050"));
}

TEST_F(StdLibListTest, TestZip) {
  ASSERT_TRUE(RunFail("(zip)"));
  ASSERT_TRUE(RunFail("(zip +)"));

  // 1 list
  ASSERT_TRUE(RunSuccess("(zip (1 2))", "((1) (2))"));
  ASSERT_TRUE(RunSuccess("(zip + (1 2))", "(1 2)"));
  ASSERT_TRUE(RunSuccess("(zip (fn (a) (+ a 2)) (1 2))", "(3 4)"));
  ASSERT_TRUE(RunFail("(zip (fn (a b) (+ a b)) (1 2))"));

  // 2 lists
  ASSERT_TRUE(RunSuccess("(zip (1 2) (3 4))", "((1 3) (2 4))"));
  ASSERT_TRUE(RunSuccess("(zip + (1 2) (3 4))", "(4 6)"));
  ASSERT_TRUE(RunFail("(zip + (1 2) (3 \"foo\"))"));
  ASSERT_TRUE(RunFail("(zip + (1 2) (3))"));
  ASSERT_TRUE(RunSuccess("(zip (fn (a b) (+ a b)) (1 2) (3 4))", "(4 6)"));
  ASSERT_TRUE(RunFail("(zip (fn () 2) (1 2) (3 4))"));
  ASSERT_TRUE(RunFail("(zip (fn (a) (+ a 2)) (1 2) (3 4))"));
  ASSERT_TRUE(RunFail("(zip (fn (a b c) (+ a b c)) (1 2) (3 4))"));
  ASSERT_TRUE(RunSuccess("(zip + (1 .. 2) (3 .. 4))", "(4 6)"));

  // 3 lists
  ASSERT_TRUE(RunSuccess("(zip (fn (a b c) (+ a b c)) (1 2) (3 4) (4 5))", "(8 11)"));
}

TEST_F(StdLibListTest, TestAny) {
  ASSERT_TRUE(RunFail("(any)"));
  ASSERT_TRUE(RunFail("(any even?)"));
  ASSERT_TRUE(RunFail("(any even? ())"));
  ASSERT_TRUE(RunSuccess("(any even? (1))", "false"));
  ASSERT_TRUE(RunSuccess("(any even? (1 2))", "true"));
  ASSERT_TRUE(RunSuccess("(any even? (1 3))", "false"));
  ASSERT_TRUE(RunSuccess("(any even? (2 4))", "true"));
  ASSERT_TRUE(RunSuccess("(any even? '(2 \"this should not get passed to even?\"))", "true"));
  ASSERT_TRUE(RunSuccess("(any even? '(2 thisShouldNotGetEvaluated))", "true"));
  ASSERT_TRUE(RunFail("(any even? '(1 thisWillGetEvaluated))"));
}

TEST_F(StdLibListTest, TestAll) {
  ASSERT_TRUE(RunFail("(all)"));
  ASSERT_TRUE(RunFail("(all even?)"));
  ASSERT_TRUE(RunFail("(all even? ())"));
  ASSERT_TRUE(RunSuccess("(all even? (1))", "false"));
  ASSERT_TRUE(RunSuccess("(all even? (1 2))", "false"));
  ASSERT_TRUE(RunSuccess("(all even? (1 3))", "false"));
  ASSERT_TRUE(RunSuccess("(all even? (2 4))", "true"));
  ASSERT_TRUE(RunFail("(all even? '(2 \"this will get passed to even?\"))"));
  ASSERT_TRUE(RunFail("(all even? '(2 thisWillGetEvaluated))"));
  ASSERT_TRUE(RunSuccess("(all even? '(1 thisShouldNotGetEvaluated))", "false"));
  ASSERT_TRUE(RunFail("(all even? (1 thisWillGetEvaluated))"));
}

TEST_F(StdLibListTest, TestTake) {
  ASSERT_TRUE(RunFail("(take)"));

  // Numeric form
  ASSERT_TRUE(RunFail("(take 1)"));
  ASSERT_TRUE(RunFail("(take -1 ())"));
  ASSERT_TRUE(RunSuccess("(take 0 ())", "()"));
  ASSERT_TRUE(RunSuccess("(take 1 ())", "()"));
  ASSERT_TRUE(RunSuccess("(take 1 (42))", "(42)"));
  ASSERT_TRUE(RunSuccess("(take 1 (42 57))", "(42)"));
  ASSERT_TRUE(RunSuccess("(take 2 (42 57))", "(42 57)"));
  ASSERT_TRUE(RunSuccess("(take 3 (42 57))", "(42 57)"));
  ASSERT_TRUE(RunSuccess("(take 2 (42 57 62))", "(42 57)"));

  // Predicate form
  ASSERT_TRUE(RunFail("(take even?)"));
  ASSERT_TRUE(RunSuccess("(take + ())", "()"));
  ASSERT_TRUE(RunFail("(take + (1))"));
  ASSERT_TRUE(RunSuccess("(take even? ())", "()"));
  ASSERT_TRUE(RunSuccess("(take even? (1))", "()"));
  ASSERT_TRUE(RunSuccess("(take even? (1 2))", "()"));
  ASSERT_TRUE(RunSuccess("(take even? (2))", "(2)"));
  ASSERT_TRUE(RunSuccess("(take even? (2 1))", "(2)"));
  ASSERT_TRUE(RunSuccess("(take even? (2 1 4))", "(2)"));
  ASSERT_TRUE(RunSuccess("(take even? (2 4 1 6))", "(2 4)"));
}

TEST_F(StdLibListTest, TestSkip) {
  ASSERT_TRUE(RunFail("(skip)"));

  // Numeric form
  ASSERT_TRUE(RunFail("(skip 1)"));
  ASSERT_TRUE(RunFail("(skip -1 ())"));
  ASSERT_TRUE(RunSuccess("(skip 0 ())", "()"));
  ASSERT_TRUE(RunSuccess("(skip 0 (1))", "(1)"));
  ASSERT_TRUE(RunSuccess("(skip 1 ())", "()"));
  ASSERT_TRUE(RunSuccess("(skip 1 (42))", "()"));
  ASSERT_TRUE(RunSuccess("(skip 1 (42 57))", "(57)"));
  ASSERT_TRUE(RunSuccess("(skip 2 (42 57))", "()"));
  ASSERT_TRUE(RunSuccess("(skip 3 (42 57))", "()"));
  ASSERT_TRUE(RunSuccess("(skip 2 (42 57 62))", "(62)"));
  ASSERT_TRUE(RunSuccess("(skip 1 (42 57 62))", "(57 62)"));

  // Predicate form
  ASSERT_TRUE(RunFail("(skip even?)"));
  ASSERT_TRUE(RunSuccess("(skip + ())", "()"));
  ASSERT_TRUE(RunFail("(skip + (1))"));
  ASSERT_TRUE(RunSuccess("(skip even? ())", "()"));
  ASSERT_TRUE(RunSuccess("(skip even? (1))", "(1)"));
  ASSERT_TRUE(RunSuccess("(skip even? (1 2))", "(1 2)"));
  ASSERT_TRUE(RunSuccess("(skip even? (2))", "()"));
  ASSERT_TRUE(RunSuccess("(skip even? (2 1))", "(1)"));
  ASSERT_TRUE(RunSuccess("(skip even? (2 1 4))", "(1 4)"));
  ASSERT_TRUE(RunSuccess("(skip even? (2 4 1 6))", "(1 6)"));
}

TEST_F(StdLibListTest, TestHead) {
  ASSERT_TRUE(RunFail("(head)"));
  ASSERT_TRUE(RunFail("(head 3)"));
  ASSERT_TRUE(RunFail("(head a)"));
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

TEST_F(StdLibListTest, TestLast) {
  ASSERT_TRUE(RunFail("(last)"));
  ASSERT_TRUE(RunFail("(last 4)"));
  ASSERT_TRUE(RunSuccess("(last ())", "()"));
  ASSERT_TRUE(RunSuccess("(last (1))", "1"));
  ASSERT_TRUE(RunSuccess("(last (1 2 3))", "3"));
}

TEST_F(StdLibListTest, TestCons) {
  ASSERT_TRUE(RunFail("(cons)"));
  ASSERT_TRUE(RunFail("(cons 1)"));
  ASSERT_TRUE(RunFail("(cons (1))"));

  // Improper lists not supported
  ASSERT_TRUE(RunFail("(cons 1 2)")); 
  ASSERT_TRUE(RunFail("(cons (1) 2)"));
  ASSERT_TRUE(RunFail("(cons (1 2) 3)"));

  ASSERT_TRUE(RunSuccess("(cons 1 nil)", "(1)"));
  ASSERT_TRUE(RunSuccess("(cons 1 ())", "(1)"));
  ASSERT_TRUE(RunSuccess("(cons 1 (2))", "(1 2)"));
  ASSERT_TRUE(RunSuccess("(cons (1) (2))", "((1) 2)"));
  ASSERT_TRUE(RunSuccess("(cons 1 (2 3))", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(cons 'a '(b c))", "(a b c)")); // #118
}

TEST_F(StdLibListTest, TestPushFront) {
  ASSERT_TRUE(RunFail("(push-front)"));
  ASSERT_TRUE(RunFail("(push-front 1)"));
  ASSERT_TRUE(RunFail("(push-front (1))"));
  ASSERT_TRUE(RunFail("(push-front 1 nil)"));
  ASSERT_TRUE(RunSuccess("(push-front nil 1)", "(1)"));
  ASSERT_TRUE(RunSuccess("(push-front () 1)", "(1)"));
  ASSERT_TRUE(RunSuccess("(push-front (2) 1)", "(1 2)"));
  ASSERT_TRUE(RunSuccess("(push-front (2) nil)", "(() 2)"));
  ASSERT_TRUE(RunSuccess("(push-front (2) (1))", "((1) 2)"));
  ASSERT_TRUE(RunSuccess("(push-front nil nil)", "(())"));
}

TEST_F(StdLibListTest, TestPushFrontBang) {
  ASSERT_TRUE(RunFail("(push-front!)"));
  ASSERT_TRUE(RunFail("(push-front! 1)"));
  ASSERT_TRUE(RunFail("(push-front! (1))"));
  ASSERT_TRUE(RunFail("(push-front! 1 nil)"));
  ASSERT_TRUE(RunFail("(push-front! nil 1)"));

  ASSERT_TRUE(RunSuccess("(set a 42)", "42"));
  ASSERT_TRUE(RunFail("(push-front! a 2)"));
  ASSERT_TRUE(RunSuccess("(set a \"foo\")", "foo"));
  ASSERT_TRUE(RunFail("(push-front! a 2)"));
  ASSERT_TRUE(RunSuccess("(set a '42)", "42"));
  ASSERT_TRUE(RunFail("(push-front! a 2)"));
  ASSERT_TRUE(RunSuccess("(set a 'blah)", "blah"));
  ASSERT_TRUE(RunFail("(push-front! a 2)"));
  ASSERT_TRUE(RunFail("(push-front! '(1) 2)"));

  ASSERT_TRUE(RunSuccess("(set a nil)", "()"));
  ASSERT_TRUE(RunSuccess("(push-front! a 2)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(2)"));

  ASSERT_TRUE(RunSuccess("(push-front! a 1)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(1 2)"));

  ASSERT_TRUE(RunSuccess("(set a (2))", "(2)"));
  ASSERT_TRUE(RunSuccess("(push-front! a nil)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(() 2)"));

  ASSERT_TRUE(RunSuccess("(set a (2))", "(2)"));
  ASSERT_TRUE(RunSuccess("(push-front! a (1))", "()"));
  ASSERT_TRUE(RunSuccess("a", "((1) 2)"));

  ASSERT_TRUE(RunSuccess("(set a nil)", "()"));
  ASSERT_TRUE(RunSuccess("(push-front! a nil)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(())"));
}

TEST_F(StdLibListTest, TestPushBack) {
  ASSERT_TRUE(RunFail("(push-back)"));
  ASSERT_TRUE(RunFail("(push-back 1)"));
  ASSERT_TRUE(RunFail("(push-back (1))"));
  ASSERT_TRUE(RunSuccess("(push-back nil 1)", "(1)"));
  ASSERT_TRUE(RunSuccess("(push-back () 1)", "(1)"));
  ASSERT_TRUE(RunSuccess("(push-back (2) 1)", "(2 1)"));
  ASSERT_TRUE(RunSuccess("(push-back (2) nil)", "(2 ())"));
  ASSERT_TRUE(RunSuccess("(push-back (2) (1))", "(2 (1))"));
  ASSERT_TRUE(RunSuccess("(push-back nil nil)", "(())"));
}

TEST_F(StdLibListTest, TestPushBackBang) {
  ASSERT_TRUE(RunFail("(push-back!)"));
  ASSERT_TRUE(RunFail("(push-back! 1)"));
  ASSERT_TRUE(RunFail("(push-back! (1))"));
  ASSERT_TRUE(RunFail("(push-back! 1 nil)"));
  ASSERT_TRUE(RunFail("(push-back! nil 1)"));

  ASSERT_TRUE(RunSuccess("(set a 42)", "42"));
  ASSERT_TRUE(RunFail("(push-back! a 2)"));
  ASSERT_TRUE(RunSuccess("(set a \"foo\")", "foo"));
  ASSERT_TRUE(RunFail("(push-back! a 2)"));
  ASSERT_TRUE(RunSuccess("(set a '42)", "42"));
  ASSERT_TRUE(RunFail("(push-back! a 2)"));
  ASSERT_TRUE(RunSuccess("(set a 'blah)", "blah"));
  ASSERT_TRUE(RunFail("(push-back! a 2)"));
  ASSERT_TRUE(RunFail("(push-back! '(1) 2)"));

  ASSERT_TRUE(RunSuccess("(set a nil)", "()"));
  ASSERT_TRUE(RunSuccess("(push-back! a 2)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(2)"));

  ASSERT_TRUE(RunSuccess("(push-back! a 1)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(2 1)"));

  ASSERT_TRUE(RunSuccess("(set a (2))", "(2)"));
  ASSERT_TRUE(RunSuccess("(push-back! a nil)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(2 ())"));

  ASSERT_TRUE(RunSuccess("(set a (2))", "(2)"));
  ASSERT_TRUE(RunSuccess("(push-back! a (1))", "()"));
  ASSERT_TRUE(RunSuccess("a", "(2 (1))"));

  ASSERT_TRUE(RunSuccess("(set a nil)", "()"));
  ASSERT_TRUE(RunSuccess("(push-back! a nil)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(())"));
}

TEST_F(StdLibListTest, TestPopFront) {
  ASSERT_TRUE(RunFail("(pop-front)"));
  ASSERT_TRUE(RunFail("(pop-front 1)"));
  ASSERT_TRUE(RunSuccess("(pop-front () )", "()"));
  ASSERT_TRUE(RunSuccess("(pop-front (1) )", "()"));
  ASSERT_TRUE(RunSuccess("(pop-front (1 2) )", "(2)"));
  ASSERT_TRUE(RunSuccess("(pop-front (1 2 3) )", "(2 3)"));
  ASSERT_TRUE(RunSuccess("(pop-front (nil) )", "()"));
  ASSERT_TRUE(RunSuccess("(pop-front (nil nil) )", "(())"));
  ASSERT_TRUE(RunSuccess("(pop-front ((1)) )", "()"));
  ASSERT_TRUE(RunSuccess("(pop-front ((1 2)) )", "()"));
  ASSERT_TRUE(RunSuccess("(pop-front ((1) (2)) )", "((2))"));
}

TEST_F(StdLibListTest, TestPopFrontBang) {
  ASSERT_TRUE(RunFail("(pop-front!)"));
  ASSERT_TRUE(RunFail("(pop-front! 1)"));
  ASSERT_TRUE(RunFail("(pop-front! a)"));
  ASSERT_TRUE(RunFail("(pop-front! nil)"));

  ASSERT_TRUE(RunSuccess("(set a 42)", "42"));
  ASSERT_TRUE(RunFail("(pop-front! a)"));
  ASSERT_TRUE(RunSuccess("(set a \"foo\")", "foo"));
  ASSERT_TRUE(RunFail("(pop-front! a)"));
  ASSERT_TRUE(RunSuccess("(set a '42)", "42"));
  ASSERT_TRUE(RunFail("(pop-front! a)"));
  ASSERT_TRUE(RunSuccess("(set a 'blah)", "blah"));
  ASSERT_TRUE(RunFail("(pop-front! a)"));
  ASSERT_TRUE(RunFail("(pop-front! '(1))"));

  ASSERT_TRUE(RunSuccess("(set a nil)", "()"));
  ASSERT_TRUE(RunSuccess("(pop-front! a )", "()"));
  ASSERT_TRUE(RunSuccess("a", "()"));

  ASSERT_TRUE(RunSuccess("(set a (1))", "(1)"));
  ASSERT_TRUE(RunSuccess("(pop-front! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "()"));

  ASSERT_TRUE(RunSuccess("(set a (1 2))", "(1 2)"));
  ASSERT_TRUE(RunSuccess("(pop-front! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(2)"));

  ASSERT_TRUE(RunSuccess("(set a (1 2 3))", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(pop-front! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(2 3)"));

  ASSERT_TRUE(RunSuccess("(set a (nil))", "(())"));
  ASSERT_TRUE(RunSuccess("(pop-front! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "()"));

  ASSERT_TRUE(RunSuccess("(set a (nil nil))", "(() ())"));
  ASSERT_TRUE(RunSuccess("(pop-front! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(())"));

  ASSERT_TRUE(RunSuccess("(set a ((1)) )", "((1))"));
  ASSERT_TRUE(RunSuccess("(pop-front! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "()"));

  ASSERT_TRUE(RunSuccess("(set a ((1 2)) )", "((1 2))"));
  ASSERT_TRUE(RunSuccess("(pop-front! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "()"));

  ASSERT_TRUE(RunSuccess("(set a ((1) (2)) )", "((1) (2))"));
  ASSERT_TRUE(RunSuccess("(pop-front! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "((2))"));
}

TEST_F(StdLibListTest, TestPopBack) {
  ASSERT_TRUE(RunFail("(pop-back)"));
  ASSERT_TRUE(RunFail("(pop-back 1)"));
  ASSERT_TRUE(RunSuccess("(pop-back () )", "()"));
  ASSERT_TRUE(RunSuccess("(pop-back (1) )", "()"));
  ASSERT_TRUE(RunSuccess("(pop-back (1 2) )", "(1)"));
  ASSERT_TRUE(RunSuccess("(pop-back (1 2 3) )", "(1 2)"));
  ASSERT_TRUE(RunSuccess("(pop-back (nil) )", "()"));
  ASSERT_TRUE(RunSuccess("(pop-back (nil nil) )", "(())"));
  ASSERT_TRUE(RunSuccess("(pop-back ((1)) )", "()"));
  ASSERT_TRUE(RunSuccess("(pop-back ((1 2)) )", "()"));
  ASSERT_TRUE(RunSuccess("(pop-back ((1) (2)) )", "((1))"));
}

TEST_F(StdLibListTest, TestPopBackBang) {
  ASSERT_TRUE(RunFail("(pop-back!)"));
  ASSERT_TRUE(RunFail("(pop-back! 1)"));
  ASSERT_TRUE(RunFail("(pop-back! a)"));
  ASSERT_TRUE(RunFail("(pop-back! nil)"));

  ASSERT_TRUE(RunSuccess("(set a 42)", "42"));
  ASSERT_TRUE(RunFail("(pop-back! a)"));
  ASSERT_TRUE(RunSuccess("(set a \"foo\")", "foo"));
  ASSERT_TRUE(RunFail("(pop-back! a)"));
  ASSERT_TRUE(RunSuccess("(set a '42)", "42"));
  ASSERT_TRUE(RunFail("(pop-back! a)"));
  ASSERT_TRUE(RunSuccess("(set a 'blah)", "blah"));
  ASSERT_TRUE(RunFail("(pop-back! a)"));
  ASSERT_TRUE(RunFail("(pop-back! '(1))"));

  ASSERT_TRUE(RunSuccess("(set a nil)", "()"));
  ASSERT_TRUE(RunSuccess("(pop-back! a )", "()"));
  ASSERT_TRUE(RunSuccess("a", "()"));

  ASSERT_TRUE(RunSuccess("(set a (1))", "(1)"));
  ASSERT_TRUE(RunSuccess("(pop-back! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "()"));

  ASSERT_TRUE(RunSuccess("(set a (1 2))", "(1 2)"));
  ASSERT_TRUE(RunSuccess("(pop-back! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(1)"));

  ASSERT_TRUE(RunSuccess("(set a (1 2 3))", "(1 2 3)"));
  ASSERT_TRUE(RunSuccess("(pop-back! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(1 2)"));

  ASSERT_TRUE(RunSuccess("(set a (nil))", "(())"));
  ASSERT_TRUE(RunSuccess("(pop-back! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "()"));

  ASSERT_TRUE(RunSuccess("(set a (nil nil))", "(() ())"));
  ASSERT_TRUE(RunSuccess("(pop-back! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "(())"));

  ASSERT_TRUE(RunSuccess("(set a ((1)) )", "((1))"));
  ASSERT_TRUE(RunSuccess("(pop-back! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "()"));

  ASSERT_TRUE(RunSuccess("(set a ((1 2)) )", "((1 2))"));
  ASSERT_TRUE(RunSuccess("(pop-back! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "()"));

  ASSERT_TRUE(RunSuccess("(set a ((1) (2)) )", "((1) (2))"));
  ASSERT_TRUE(RunSuccess("(pop-back! a)", "()"));
  ASSERT_TRUE(RunSuccess("a", "((1))"));
}

TEST_F(StdLibListTest, TestEmpty) {
  ASSERT_TRUE(RunFail("(empty?)"));
  ASSERT_TRUE(RunFail("(empty? 3)"));
  ASSERT_TRUE(RunSuccess("(empty? ())", "true"));
  ASSERT_TRUE(RunSuccess("(empty? nil)", "true"));
  ASSERT_TRUE(RunSuccess("(empty? (1 2))", "false"));
}

TEST_F(StdLibListTest, TestLength) {
  ASSERT_TRUE(RunFail("(length)"));
  ASSERT_TRUE(RunFail("(length 3)"));
  ASSERT_TRUE(RunSuccess("(length ())", "0"));
  ASSERT_TRUE(RunSuccess("(length nil)", "0"));
  ASSERT_TRUE(RunSuccess("(length (23 42))", "2"));
}

TEST_F(StdLibListTest, TestRange) {
  ASSERT_TRUE(RunFail("(range)"));
  ASSERT_TRUE(RunFail("(range 1)"));
  ASSERT_TRUE(RunFail("(range 1 true)"));
  ASSERT_TRUE(RunSuccess("(range 1 1)", "(1)"));
  ASSERT_TRUE(RunSuccess("(range 1 3)", "(1 2 3)"));
  ASSERT_TRUE(RunFail("(range 3 1)"));
  ASSERT_TRUE(RunSuccess("(range -3 -1)", "(-3 -2 -1)"));

  // using step
  ASSERT_TRUE(RunFail("(range 1 4 0)"));
  ASSERT_TRUE(RunSuccess("(range 1 4 1)", "(1 2 3 4)"));
  ASSERT_TRUE(RunSuccess("(range 1 4 2)", "(1 3)"));
  ASSERT_TRUE(RunFail("(range 4 1 1)"));
  ASSERT_TRUE(RunSuccess("(range 4 1 -1)", "(4 3 2 1)"));

  ASSERT_TRUE(RunSuccess("(range -4 -1 1)", "(-4 -3 -2 -1)"));
  ASSERT_TRUE(RunSuccess("(range -4 -1 2)", "(-4 -2)"));
  ASSERT_TRUE(RunFail("(range -1 -4 1)"));
  ASSERT_TRUE(RunSuccess("(range -1 -4 -1)", "(-1 -2 -3 -4)"));

  ASSERT_TRUE(RunSuccess("(range 1 4 100)", "(1)"));
}

TEST_F(StdLibListTest, TestForeach) {
  ASSERT_TRUE(RunSuccess("lst = (2 3 4)", "(2 3 4)"));
  ASSERT_TRUE(RunFail("(foreach)"));
  ASSERT_TRUE(RunFail("(foreach lst)"));
  ASSERT_TRUE(RunFail("(foreach e lst)"));
  ASSERT_TRUE(RunFail("(foreach e 3 (e *= 2))"));
  ASSERT_TRUE(RunFail("(foreach e false (e *= 2))"));


  ASSERT_TRUE(RunSuccess("tsl = ()", ""));
  ASSERT_TRUE(RunSuccess("(foreach e lst (tsl = (e) + tsl))", "(4 3 2)"));
  ASSERT_TRUE(RunSuccess("tsl = ()", ""));
  ASSERT_TRUE(RunSuccess("(foreach e in lst (tsl = (e) + tsl))", "(4 3 2)"));

  ASSERT_TRUE(RunSuccess("(foreach e in lst (e = (2 * e)))", "8"));
  ASSERT_TRUE(RunSuccess("lst", "(4 6 8)"));

  ASSERT_TRUE(RunSuccess("(foreach e in (1 2 3) (e *= 2))", "6"));

  ASSERT_TRUE(RunSuccess("(foreach lst (fn (e) (e * 2)))", "16"));
  ASSERT_TRUE(RunSuccess("lst", "(4 6 8)"));
  ASSERT_TRUE(RunSuccess("(foreach lst (fn (e) (e *= 2)))", "16"));
  ASSERT_TRUE(RunSuccess("lst", "(8 12 16)"));

  ASSERT_TRUE(RunSuccess("sum = 0", "0"));
  ASSERT_TRUE(RunSuccess("(foreach i in (range 1 100) (sum += i))", "5050"));

  ASSERT_TRUE(RunSuccess("(for e : lst (e += 10))", "26"));
  ASSERT_TRUE(RunSuccess("lst", "(18 22 26)"));
}

TEST_F(StdLibListTest, TestReverse) {
  ASSERT_TRUE(RunFail("(reverse (2) (3))"));
  ASSERT_TRUE(RunSuccess("(reverse ())", "()"));
  ASSERT_TRUE(RunSuccess("(reverse (1))", "(1)"));
  ASSERT_TRUE(RunSuccess("(reverse (1 2))", "(2 1)"));
  ASSERT_TRUE(RunSuccess("(reverse (1 2 3))", "(3 2 1)"));
  ASSERT_TRUE(RunSuccess("(reverse (1 2 3 4))", "(4 3 2 1)"));
  ASSERT_TRUE(RunSuccess("(reverse (1 2 3 4 5))", "(5 4 3 2 1)"));
}

TEST_F(StdLibListTest, TestAt) {
  ASSERT_NO_FATAL_FAILURE(RunAtTest());
}

TEST_F(StdLibListTest, TestNth) {
  ASSERT_NO_FATAL_FAILURE(RunAtTest());
  ASSERT_TRUE(RunSuccess("(nth (10 20 30) 1)", "20"));
}

class StdLibLogicalTest: public StdLibTest {
  protected:
    string Prefix;
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
  string evalCode = Prefix + "(< n 100) (>= n 10) (!= n 43))";
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
  
  string code = "(not (< n 100))";
  ASSERT_TRUE(RunSuccess("(set n 42)", "42"));
  ASSERT_TRUE(RunSuccess(code, "false"));
  ASSERT_TRUE(RunSuccess("(set n 420)", "420"));
  ASSERT_TRUE(RunSuccess(code, "true"));
}

class StdLibComparisonTest: public StdLibTest {
  protected:
    string Prefix;

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
};

TEST_F(StdLibBranchTest, TestCond) {
  ASSERT_TRUE(RunFail("(cond)"));
  ASSERT_TRUE(RunFail("(cond 3)"));
  ASSERT_TRUE(RunFail("(cond ())"));
  ASSERT_TRUE(RunFail("(cond (true))"));
  ASSERT_TRUE(RunFail("(cond (3))"));
  ASSERT_TRUE(RunFail("(cond (3 true))"));
  ASSERT_TRUE(RunSuccess("(cond (true 3))", "3"));
  ASSERT_TRUE(RunFail("(cond (true 3 4))"));
  ASSERT_TRUE(RunFail("(cond 3 4)"));
  ASSERT_TRUE(RunFail("(cond () ())"));
  ASSERT_TRUE(RunSuccess("(cond (false 3))", "()"));
  ASSERT_TRUE(RunSuccess("(cond (true 3) (true 4))", "3"));

  ASSERT_TRUE(RunFail("(cond (foo 3))"));
  ASSERT_TRUE(RunFail("(cond (true foo))"));
  ASSERT_TRUE(RunSuccess("foo = true", "true"));
  ASSERT_TRUE(RunSuccess("(cond (foo 3))", "3"));
  ASSERT_TRUE(RunSuccess("(cond (true foo))", "true"));

  const char * code = "(cond "
                      "  ((n > 0) \"greater than zero\") "
                      "  ((n < 0) \"less than zero\") "
                      "  (true    \"equal to zero\")) ";
  ASSERT_TRUE(RunSuccess("n = 2", "2"));
  ASSERT_TRUE(RunSuccess(code, "greater"));
  ASSERT_TRUE(RunSuccess("n = -3", "-3"));
  ASSERT_TRUE(RunSuccess(code, "less"));
  ASSERT_TRUE(RunSuccess("n = 0", "0"));
  ASSERT_TRUE(RunSuccess(code, "equal"));
  ASSERT_TRUE(RunSuccess("n = \"foobar\"", "\"foobar\""));
  ASSERT_TRUE(RunFail(code));
}

TEST_F(StdLibBranchTest, TestSwitch) {
  ASSERT_TRUE(RunFail("(switch)"));
  ASSERT_TRUE(RunFail("(switch 2)"));
  ASSERT_TRUE(RunFail("(switch 2 ())"));
  ASSERT_TRUE(RunSuccess("(switch 2 (1 \"one\") (\"other\"))", "other"));
  ASSERT_TRUE(RunSuccess("(switch 2 (case 1 \"one\") (default \"other\"))", "other"));
  ASSERT_TRUE(RunSuccess("(switch 2 (1 \"one\") (2 \"two\") (\"other\"))", "two"));
  ASSERT_TRUE(RunSuccess("(switch 2 (case 1 \"one\") (case 2 \"two\") (default \"other\"))", "two"));
  ASSERT_TRUE(RunSuccess("(switch 3 (1 \"one\") (2 \"two\") (\"other\"))", "other"));

  ASSERT_TRUE(RunFail("(switch 3 (noncasesymbol 3 \"three\") (default 3))"));

  // Make sure varExpr only gets evaluated once
  ASSERT_TRUE(RunSuccess("n = 1", "1"));
  ASSERT_TRUE(RunSuccess("(switch (n += 1) (2 \"two\") (\"other\"))", "two"));
  
  const char *code1 = "(switch (type x)            "
                      "  (int \"x is an int\")     "
                      "  (float \"x is a float\")  "
                      "  (\"x is not a number\"))  ";
  const char *code2 = "(switch (type x)                    "
                      "  (case int \"x is an int\")        "
                      "  (case float \"x is a float\")     "
                      "  (default \"x is not a number\"))  ";
  ASSERT_TRUE(RunFail(code1));
  ASSERT_TRUE(RunFail(code2));
  ASSERT_TRUE(RunSuccess("x = 4", "4"));
  ASSERT_TRUE(RunSuccess(code1, "is an int"));
  ASSERT_TRUE(RunSuccess(code2, "is an int"));
  ASSERT_TRUE(RunSuccess("x = 3.14", "3.14"));
  ASSERT_TRUE(RunSuccess(code1, "is a float"));
  ASSERT_TRUE(RunSuccess(code2, "is a float"));
  ASSERT_TRUE(RunSuccess("x = \"foo\"", "foo"));
  ASSERT_TRUE(RunSuccess(code1, "is not a number"));
  ASSERT_TRUE(RunSuccess(code2, "is not a number"));
}

TEST_F(StdLibBranchTest, TestIf) {
  ASSERT_TRUE(RunFail("(if)"));
  ASSERT_TRUE(RunFail("(if true)"));
  ASSERT_TRUE(RunSuccess("(if true 1)", "1"));
  ASSERT_TRUE(RunSuccess("(if false 1)", "()"));
  ASSERT_TRUE(RunSuccess("(if true 1 0)", "1"));
  ASSERT_TRUE(RunSuccess("(if false 1 0)", "0"));
  ASSERT_TRUE(RunSuccess("(if true 1 a)", "1"));
  ASSERT_TRUE(RunSuccess("(if false a 0)", "0"));
  ASSERT_TRUE(RunFail("(if true a 0)"));
  ASSERT_TRUE(RunFail("(if false 0 a)"));
  ASSERT_TRUE(RunSuccess("(if (< 3 4) \"less than\" \"greater than or equal to\")", "\"less than\""));
  ASSERT_TRUE(RunSuccess("(if (< 3 3) \"less than\" (if (> 3 3) \"greater than\" \"equal to\"))", "\"equal to\"")); 
}

TEST_F(StdLibBranchTest, TestWhile) {
  ASSERT_TRUE(RunFail("(while)"));
  ASSERT_TRUE(RunFail("(while (false))"));
  ASSERT_TRUE(RunSuccess("(while false 3)", "()"));
  ASSERT_TRUE(RunFail("(while foo 3)"));
  ASSERT_TRUE(RunFail("(while true foo)"));

  ASSERT_TRUE(RunSuccess("a = 0", "0"));
  ASSERT_TRUE(RunSuccess("(while (a == 0) (++ a) 3 4 5)", "5"));

  ASSERT_TRUE(RunSuccess("a = 0", "0"));
  ASSERT_TRUE(RunFail("(while (a < 10)        "
                      "  (++ a)               "
                      "  (if (a == 9) foobar) "
                      "  5)                   "));

  ASSERT_TRUE(RunSuccess("a = 0", "0"));
  ASSERT_TRUE(RunSuccess("b = 0", "0"));
  ASSERT_TRUE(RunSuccess("(while ((+= a 2) < 12) 3 (++ b) 5)", "5"));
  ASSERT_TRUE(RunSuccess("b", "5"));

  const char *code = "(while (i < 10) "
                     "  (++ i))       ";
  ASSERT_TRUE(RunSuccess("i = 1", "1"));
  ASSERT_TRUE(RunSuccess(code, "10"));
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

  // eval func in let
  ASSERT_TRUE(RunSuccess("(let ((l (length \"abc\"))) l)", "3"));
}

TEST_F(StdLibBranchTest, TestQuoteFn) {
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

  // Quote specific tests
  ASSERT_TRUE(RunSuccess("(* 4 (quote 5))", "20"));
  ASSERT_TRUE(RunSuccess("(* (quote 4) (quote 5))", "20"));
  ASSERT_TRUE(RunSuccess("(set q (quote foo))", "foo"));
  ASSERT_TRUE(RunFail("(* 4 q)"));
  ASSERT_TRUE(RunSuccess("(set foo 5)", "5"));
  ASSERT_TRUE(RunFail("(* 4 q)")); //#118

  // These don't actually call invoke quote func
  ASSERT_TRUE(RunSuccess("'42", "42"));
  ASSERT_TRUE(RunSuccess("'3.14", "3.14"));
  ASSERT_TRUE(RunSuccess("'true", "true"));
  ASSERT_TRUE(RunSuccess("'\"qux\"", "qux"));
  ASSERT_TRUE(RunSuccess("'foo", "foo"));
  ASSERT_TRUE(RunSuccess("'(+ 1 2)", "(+ 1 2)"));
}

TEST_F(StdLibBranchTest, TestUnquote) {
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

  ASSERT_TRUE(RunFail("(unquote 'foo)"));
  ASSERT_TRUE(RunSuccess("(unquote ''foo)", "foo"));
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
  ASSERT_TRUE(RunSuccess("(str 3.14)", "\"3.1400000000000001\""));

  ASSERT_TRUE(RunSuccess("(str \"\")", "\"\""));
  ASSERT_TRUE(RunSuccess("(str \"foo\")", "\"foo\""));
  ASSERT_TRUE(RunSuccess("(str \"0\")", "\"0\""));
  ASSERT_TRUE(RunSuccess("(str \"42\")", "\"42\""));
  ASSERT_TRUE(RunSuccess("(str \"3.14\")", "\"3.14\""));

  ASSERT_TRUE(RunSuccess("(str \"-42\")", "\"-42\""));
  ASSERT_TRUE(RunSuccess("(str \"0xfe\")", "\"0xfe\""));
}

TEST_F(StdLibOperatorsTest, TestSymbol) {
  ASSERT_TRUE(RunFail("(symbol)"));
  ASSERT_TRUE(RunFail("(symbol 3 4)"));

  ASSERT_TRUE(RunFail("(symbol true)"));
  ASSERT_TRUE(RunFail("(symbol false)"));

  ASSERT_TRUE(RunFail("(symbol 0)"));
  ASSERT_TRUE(RunFail("(symbol 1)"));
  ASSERT_TRUE(RunFail("(symbol 3)"));
  ASSERT_TRUE(RunFail("(symbol 3.14)"));

  ASSERT_TRUE(RunFail("(symbol \"\")"));
  ASSERT_TRUE(RunSuccess("(symbol \"foo\")", "foo"));
  ASSERT_TRUE(RunFail("(symbol \"foo bar\")"));
  ASSERT_TRUE(RunFail("(symbol \"1foo\")"));
  ASSERT_TRUE(RunSuccess("(symbol \"foo1\")", "foo1"));
  ASSERT_TRUE(RunFail("(symbol \"0\")"));
  ASSERT_TRUE(RunFail("(symbol \"42\")"));
  ASSERT_TRUE(RunFail("(symbol \"3.14\")"));

  ASSERT_TRUE(RunFail("(symbol \"-42\")"));
  ASSERT_TRUE(RunFail("(symbol \"0xfe\")"));
}

TEST_F(StdLibOperatorsTest, TestType) {
  ASSERT_TRUE(RunSuccess("(type 3)", "int"));
  ASSERT_TRUE(RunSuccess("(type 3.14)", "float"));
  ASSERT_TRUE(RunSuccess("(type true)", "bool"));
  ASSERT_TRUE(RunSuccess("(type false)", "bool"));
  ASSERT_TRUE(RunSuccess("(type \"foo\")", "str"));
  ASSERT_TRUE(RunSuccess("(type ())", "list"));
  ASSERT_TRUE(RunSuccess("(type (1))", "list"));
  ASSERT_TRUE(RunSuccess("(type (1 2))", "list"));
  ASSERT_TRUE(RunSuccess("(type +)", "fn"));

  ASSERT_TRUE(RunSuccess("a = 3.14", "3.14"));
  ASSERT_TRUE(RunSuccess("(type a)", "float"));

  ASSERT_TRUE(RunSuccess("(type 0xa)", "int"));
  ASSERT_TRUE(RunSuccess("(type 0xe)", "int")); //#104
}

TEST_F(StdLibOperatorsTest, TestTypeQ) {
  ASSERT_TRUE(RunSuccess("(bool? true)", "true"));
  ASSERT_TRUE(RunSuccess("(bool? 3)", "false"));
  ASSERT_TRUE(RunSuccess("(int? 3)", "true"));
  ASSERT_TRUE(RunSuccess("(int? 3.14)", "false"));
  ASSERT_TRUE(RunSuccess("(float? 3.14)", "true"));
  ASSERT_TRUE(RunSuccess("(float? \"foo\")", "false"));
  ASSERT_TRUE(RunSuccess("(str? \"foo\")", "true"));
  ASSERT_TRUE(RunSuccess("(str? (1 2))", "false"));
  ASSERT_TRUE(RunSuccess("(list? (1 2))", "true"));
  ASSERT_TRUE(RunSuccess("(list? +)", "false"));
  ASSERT_TRUE(RunSuccess("(fn? +)", "true"));
  ASSERT_TRUE(RunSuccess("(fn? false)", "false"));
  ASSERT_TRUE(RunSuccess("(atom? 3)", "true"));
  ASSERT_TRUE(RunSuccess("(atom? (3))", "false"));

  ASSERT_TRUE(RunSuccess("a = 3.14", "3.14"));
  ASSERT_TRUE(RunSuccess("(float? a)", "true"));

  // #119
  ASSERT_TRUE(RunSuccess("(symbol? +)", "true"));
  ASSERT_TRUE(RunFail("(symbol? 3.14)"));
  ASSERT_TRUE(RunSuccess("(symbol? true)", "true"));
  ASSERT_TRUE(RunSuccess("(symbol? a)", "true"));
  ASSERT_TRUE(RunSuccess("(symbol? b)", "false"));
}
