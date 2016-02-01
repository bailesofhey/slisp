#include <sstream>
#include <fstream>
#include <string>
#include "gtest\gtest.h"

#include "Controller.h"

TEST(TestController, TestRunStreams) {
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

TEST(TestController, TestRunString) {
  std::stringstream out;
  Controller controller;
  controller.SetOutput(out);
  controller.Run("(+ 2 3)");
  ASSERT_NE(out.str().find("5"), std::string::npos);

  controller.Run("(+ 4 6)");
  ASSERT_NE(out.str().find("10"), std::string::npos);
}

TEST(TestController, TestRunFile) {
  std::stringstream out;
  Controller controller;
  controller.SetOutput(out);
  controller.RunFile("..\\..\\Test\\Files\\TestRunFile1.slisp");
  ASSERT_NE(out.str().find("5"), std::string::npos);

  controller.RunFile("..\\..\\Test\\Files\\TestRunFile2.slisp");
  ASSERT_NE(out.str().find("10"), std::string::npos);
}

TEST(TestController, TestOutputStream) {
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

TEST(TestController, TestOutputFile) {
  Controller controller;
  ASSERT_NO_FATAL_FAILURE(TestOutFile(controller, "TestOutputFile1.txt", "(+ 2 3)", "5"));
  ASSERT_NO_FATAL_FAILURE(TestOutFile(controller, "TestOutputFile2.txt", "(+ 4 6)", "10"));
}