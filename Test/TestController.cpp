#include <sstream>
#include <fstream>
#include <string>
#include "gtest\gtest.h"

#include "Controller.h"

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
  controller.RunFile("..\\..\\Test\\Files\\TestRunFile1.slisp");
  ASSERT_NE(out.str().find("5"), std::string::npos);

  controller.RunFile("..\\..\\Test\\Files\\TestRunFile2.slisp");
  ASSERT_NE(out.str().find("10"), std::string::npos);
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

TEST_F(ControllerTest, TestEnvironment) {
  {
    Controller controller(0, nullptr);
    auto &env = GetEnvironment(controller);
    ASSERT_TRUE(env.ProcessArgs().empty());
    ASSERT_TRUE(env.SlispArgs().empty());
  }
  {
    char *args[] = {"Slisp.exe"};
    const size_t size = 1;
    Controller controller(size, args);
    auto &env = GetEnvironment(controller);
    ASSERT_EQ(size, env.ProcessArgs().size());
    ASSERT_EQ(size, env.SlispArgs().size());
  }
  {
    char *args[] = {"Slisp.exe", "arg1"};
    const size_t size = 2;
    Controller controller(size, args);
    auto &env = GetEnvironment(controller);
    ASSERT_EQ(size, env.ProcessArgs().size());
    ASSERT_EQ(size, env.SlispArgs().size());
  }
  {
    char *args[] = {"Slisp.exe", "arg1", "arg2"};
    const size_t size = 3;
    Controller controller(size, args);
    auto &env = GetEnvironment(controller);
    ASSERT_EQ(size, env.ProcessArgs().size());
    ASSERT_EQ(size, env.SlispArgs().size());
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
