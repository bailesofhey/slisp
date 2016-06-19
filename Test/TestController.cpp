#include <sstream>
#include <fstream>
#include <string>
#include <initializer_list>
#include "gtest\gtest.h"

#include "Controller.h"

using namespace std;

struct ArgTest {
  vector<const char *> CmdArgs;
  int ExpectedFlags;
  string ExpectedProgramName;
  string ExpectedRun;
  vector<string> ExpectedScriptArgs;
};

vector<ArgTest> ArgTests {
  { {}, ControllerArgs::REPL, "", "", {} },
  { {"slisp"}, ControllerArgs::REPL, "slisp", "", {} },

  { {"slisp", "-badArg"}, ControllerArgs::Error, "slisp", "", {} },

  { {"slisp", "-h"}, ControllerArgs::Help, "slisp", "", {} },
  { {"slisp", "--help"}, ControllerArgs::Help, "slisp", "", {} },
  { {"slisp", "/?"}, ControllerArgs::Help, "slisp", "", {} },

  { {"slisp", "(+ 3 4)"}, ControllerArgs::RunCode, "slisp", "(+ 3 4)", {} },
  { {"slisp", "-i", "(+ 3 4)"}, ControllerArgs::RunCode | ControllerArgs::REPL, "slisp", "(+ 3 4)", {} },
  { {"slisp", "(+ 3 4)", "-i"}, ControllerArgs::RunCode, "slisp", "(+ 3 4)", {"-i"} },

  { {"slisp", ".slisp"}, ControllerArgs::RunFile, "slisp", ".slisp", {} },
  { {"slisp", "script.slisp"}, ControllerArgs::RunFile, "slisp", "script.slisp", {} },
  { {"slisp", "-i", "script.slisp"}, ControllerArgs::RunFile | ControllerArgs::REPL, "slisp", "script.slisp", {} },
  { {"slisp", "script.slisp", "-i"}, ControllerArgs::RunFile, "slisp", "script.slisp", {"-i"} },
  { {"slisp", "script.slisp", "arg1"}, ControllerArgs::RunFile, "slisp", "script.slisp", {"arg1"} },
  { {"slisp", "script.slisp", "arg1", "arg2"}, ControllerArgs::RunFile, "slisp", "script.slisp", {"arg1", "arg2"} },

  { {"slisp", "-i", "arg1", "arg2"}, ControllerArgs::RunCode | ControllerArgs::REPL, "slisp", "arg1", {"arg2"} },
  { {"slisp", "arg1", "arg2"}, ControllerArgs::RunCode, "slisp", "arg1", {"arg2"} },
};

void ParseTest(const ArgTest &test) {
  ControllerArgs args(static_cast<int>(test.CmdArgs.size()), test.CmdArgs.data());
  EXPECT_EQ(test.ExpectedFlags, args.Flags);
  EXPECT_EQ(test.ExpectedRun, args.Run);
  EXPECT_EQ(test.ExpectedProgramName, args.ProgramName);
  EXPECT_EQ(test.ExpectedScriptArgs, args.ScriptArgs);
}

TEST(ControllerArgs, TestParse) {
  for (int i = 0; i < ArgTests.size(); ++i)
    EXPECT_NO_FATAL_FAILURE(ParseTest(ArgTests[i])) << "Test #" << i;
}

class ControllerTest: public testing::Test {
protected:
  Environment& GetEnvironment(Controller &controller) {
    return controller.Interpreter_.GetEnvironment();
  }
};

TEST_F(ControllerTest, TestRunStreams) {
  std::stringstream in,
                    out;
  Controller controller;
  controller.SetOutput(out);
  in << "(+ 2 3)" << std::endl;
  controller.Run(in);
  ASSERT_NE(out.str().find("5"), std::string::npos);

  in << "(+ 4 6)" << std::endl;
  controller.Run(in);
  ASSERT_NE(out.str().find("10"), std::string::npos);
}

TEST_F(ControllerTest, TestRunString) {
  std::stringstream out;
  Controller controller;
  controller.SetOutput(out);
  controller.Run("(+ 2 3)");
  ASSERT_NE(out.str().find("5"), std::string::npos);

  controller.Run("(+ 4 6)");
  ASSERT_NE(out.str().find("10"), std::string::npos);
}

TEST_F(ControllerTest, TestRunFile) {
  std::stringstream out;
  Controller controller;
  controller.SetOutput(out);
  ASSERT_TRUE(controller.RunFile("..\\..\\Test\\Files\\TestRunFile1.slisp"));
  ASSERT_NE(out.str().find("5"), std::string::npos);

  ASSERT_TRUE(controller.RunFile("..\\..\\Test\\Files\\TestRunFile2.slisp"));
  ASSERT_NE(out.str().find("10"), std::string::npos);

  ASSERT_FALSE(controller.RunFile("ThisFileDoesNotExist.slisp"));
}

TEST_F(ControllerTest, TestOutputStream) {
  std::stringstream out1,
                    out2;
  Controller controller;
  controller.SetOutput(out1);
  controller.Run("(+ 2 3)");
  ASSERT_NE(out1.str().find("5"), std::string::npos);
  controller.SetOutput(out2);
  controller.Run("(+ 4 6)");
  ASSERT_NE(out2.str().find("10"), std::string::npos);
}

TEST_F(ControllerTest, TestRunArgs_Help) {
  vector<const char*> args { "slisp.exe", "-help" };
  std::stringstream out;
  Controller controller(static_cast<int>(args.size()), args.data());
  controller.SetOutput(out);
  controller.Run();
  ASSERT_NE(out.str().find("usage"), std::string::npos);
}

TEST_F(ControllerTest, TestRunArgs_Error) {
  vector<const char*> args { "slisp.exe", "-badArg" };
  std::stringstream out;
  Controller controller(static_cast<int>(args.size()), args.data());
  controller.SetOutput(out);
  controller.Run();
  ASSERT_NE(out.str().find("usage"), std::string::npos);
}

TEST_F(ControllerTest, TestRunArgs_Code) {
  {
    vector<const char*> args { "slisp.exe", "(+ 2 3)"};
    std::stringstream out;
    Controller controller(static_cast<int>(args.size()), args.data());
    controller.SetOutput(out);
    controller.Run();
    ASSERT_NE(out.str().find("5"), std::string::npos);
  }
  {
    vector<const char*> args { "slisp.exe", "(length sys.args)", "2", "3", "4"};
    std::stringstream out;
    Controller controller(static_cast<int>(args.size()), args.data());
    controller.SetOutput(out);
    controller.Run();
    ASSERT_NE(out.str().find("3"), std::string::npos);
  }
}

TEST_F(ControllerTest, TestRunArgs_File) {
  {
    vector<const char*> args { "slisp.exe", "..\\..\\Test\\Files\\TestRunFile1.slisp"};
    std::stringstream out;
    Controller controller(static_cast<int>(args.size()), args.data());
    controller.SetOutput(out);
    controller.Run();
    ASSERT_NE(out.str().find("5"), std::string::npos);
  }
  {
    vector<const char*> args { "slisp.exe", "..\\..\\Test\\Files\\TestRunArgs.slisp", "2", "3", "4"};
    std::stringstream out;
    Controller controller(static_cast<int>(args.size()), args.data());
    controller.SetOutput(out);
    controller.Run();
    ASSERT_NE(out.str().find("3"), std::string::npos);
  }
  {
    vector<const char*> args { "slisp.exe", "ThisFileDoesNotExist.slisp"};
    std::stringstream out;
    Controller controller(static_cast<int>(args.size()), args.data());
    controller.SetOutput(out);
    controller.Run();
    ASSERT_NE(out.str().find("ThisFileDoesNotExist"), std::string::npos);
  }
}

TEST_F(ControllerTest, TestEnvironment) {
  {
    const size_t size = 0;
    Controller controller(0, nullptr);
    auto &env = GetEnvironment(controller);
    ASSERT_EQ("", env.Program);
    ASSERT_EQ("", env.Script);
    ASSERT_EQ(size, env.Args.size());
  }
  {
    vector<const char*> args = {"slisp", "script.slisp", "arg1", "arg2"};
    const size_t size = 2;
    Controller controller(static_cast<int>(args.size()), args.data());
    auto &env = GetEnvironment(controller);
    ASSERT_EQ("slisp", env.Program);
    ASSERT_EQ("script.slisp", env.Script);
    ASSERT_EQ(size, env.Args.size());
  }
}

void TestOutFile(Controller &controller, const char *outFileName, const char *code, const char *expectedOutput) {
  std::fstream outFile;
  std::string outLine;
  ASSERT_TRUE(controller.SetOutputFile(outFileName));
  controller.Run(code);
  controller.SetOutput();
  outFile.open(outFileName, std::ios_base::in);
  ASSERT_TRUE(outFile.is_open());
  std::getline(outFile, outLine);
  ASSERT_NE(outLine.find(expectedOutput), std::string::npos);
}

TEST_F(ControllerTest, TestOutputFile) {
  Controller controller;
  ASSERT_NO_FATAL_FAILURE(TestOutFile(controller, "TestOutputFile1.txt", "(+ 2 3)", "5"));
  ASSERT_NO_FATAL_FAILURE(TestOutFile(controller, "TestOutputFile2.txt", "(+ 4 6)", "10"));
}
