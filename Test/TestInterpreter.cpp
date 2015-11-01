#include "gtest\gtest.h"
#include "Interpreter.h"

TEST(Interpreter, TestSimple) {
  Interpreter interpreter;
  auto defaultSexp = interpreter.GetDefaultSexp();
  ASSERT_NE("", defaultSexp);
}