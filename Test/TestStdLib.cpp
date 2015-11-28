#include <sstream>
#include <string>
#include "gtest\gtest.h"

#include "Controller.h"

class StdLibTest: public ::testing::Test {
  protected:
    Controller Controller_;
    std::stringstream Out;
    
    explicit StdLibTest() {
      Controller_.SetOutput(Out);
    }

    bool RunSuccess(const char *code, const char *expectedResult) {
      Out.str("");
      Out.clear();
      Controller_.Run(code);
      return Out.str().find(expectedResult) != std::string::npos;
    }

    bool RunFail(const char *code) {
      return RunSuccess(code, "Error");
    }
};

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
    static const char * const values[] = {" ", "\"foo\"", "+", "true", "a"};
    std::string prefix = "(" + name + " ";
    for (auto &value : values) {
      if (name != "+" || value[0] != '"') {
        std::string code = prefix + value + ")";
        ASSERT_TRUE(RunFail(code.c_str()));
      }
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
  ASSERT_TRUE(RunSuccess(temp.str().c_str(), std::to_string(MinValue).c_str()));
}

TEST_F(StdLibNumericalTest, TestDec) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("dec"));
  ASSERT_TRUE(RunSuccess("(dec -1)", "-2"));
  ASSERT_TRUE(RunSuccess("(dec 0)", "-1"));
  ASSERT_TRUE(RunSuccess("(dec 1)", "0"));
  ASSERT_TRUE(RunSuccess("(dec 42)", "41"));

  std::stringstream temp;
  temp << "(dec " << MinValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str().c_str(), std::to_string(MaxValue).c_str()));
}

TEST_F(StdLibNumericalTest, TestAdd) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("+"));
  ASSERT_TRUE(RunSuccess("(+ 42)", "42"));
  ASSERT_TRUE(RunSuccess("(+ 2 3)", "5"));
  ASSERT_TRUE(RunSuccess("(+ 1 2 3 4 5)", "15"));

  std::stringstream temp;
  temp << "(+ 1 " << MaxValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str().c_str(), std::to_string(MinValue).c_str()));
}

TEST_F(StdLibNumericalTest, TestSub) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("-"));
  ASSERT_TRUE(RunSuccess("(- 42)", "42"));
  ASSERT_TRUE(RunSuccess("(- 5 3)", "2"));
  ASSERT_TRUE(RunSuccess("(- 1 2 3 4 5)", "-13"));

  std::stringstream temp;
  temp << "(- 1 " << MinValue << ")";
  ASSERT_TRUE(RunSuccess(temp.str().c_str(), std::to_string(MaxValue).c_str()));
}

TEST_F(StdLibNumericalTest, TestMult) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("*"));
  ASSERT_TRUE(RunSuccess("(* 2)", "2"));
  ASSERT_TRUE(RunSuccess("(* 2 3)", "6"));
  ASSERT_TRUE(RunSuccess("(* 1 2 3 4 5)", "120"));
}

TEST_F(StdLibNumericalTest, TestDiv) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("/"));
  ASSERT_TRUE(RunSuccess("(/ 6)", "6"));
  ASSERT_TRUE(RunSuccess("(/ 6 2)", "3"));
  ASSERT_TRUE(RunSuccess("(/ 120 5 4 3 2 1)", "1"));
  ASSERT_TRUE(RunSuccess("(/ 1 2)", "0"));
  ASSERT_TRUE(RunFail("(/ 1 0)"));
  ASSERT_TRUE(RunFail("(/ 0 0)"));
}

TEST_F(StdLibNumericalTest, TestMod) {
  ASSERT_NO_FATAL_FAILURE(TestBadNumericArgs("%"));
  ASSERT_TRUE(RunSuccess("(% 1)", "1"));
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