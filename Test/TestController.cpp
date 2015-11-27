#include <sstream>
#include "gtest\gtest.h"

#include "Controller.h"

TEST(TestController, TestStreams) {
  std::stringstream in,
                    out;
  Controller controller;
  in << "(+ 2 3)" << std::endl;
  controller.SetOutput(out);
  controller.Run(in);
  ASSERT_NE(out.str().find("5"), std::string::npos);
}