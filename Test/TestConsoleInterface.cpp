#include "gtest\gtest.h"
#include <iostream>
#include <sstream>
#include "ConsoleInterface.h"

using ReadFn = bool(ConsoleInterface::*)(std::string &);

void TestSingleRead(ConsoleInterface &console, std::stringstream &out, ReadFn readFn, const std::string &expectedStr) {
  std::string currLine;
  ASSERT_TRUE((console.*readFn)(currLine));
  ASSERT_EQ(expectedStr, currLine);
  ASSERT_GT(out.str().length(), (size_t)0);
  out.str("");
}

void TestReadFn(ReadFn readFn) {
  std::stringstream in,
                    out;
  ConsoleInterface  console(in, out);

  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));
  
  in << "this is line number 1" << std::endl
     << "Line #2" << std::endl
     << std::endl;
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, "this is line number 1"));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, "Line #2"));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));

  in << "Another first line" << std::endl;
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, "Another first line"));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));

  in << "Incomplete line";
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, "Incomplete line"));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));
}

TEST(ConsoleInterface, TestReadInputLine) {
  TestReadFn(&ConsoleInterface::ReadInputLine);
}

TEST(ConsoleInterface, TestReadContinuedInputLine) {
  TestReadFn(&ConsoleInterface::ReadContinuedInputLine);
}

using WriteFn = bool(ConsoleInterface::*)(const std::string &);
void TestWriteFn(WriteFn writeFn) {
  std::stringstream in,
                    out;
  ConsoleInterface  console(in, out);
  std::string       lineToOutput = "";
  ASSERT_TRUE((console.*writeFn)(lineToOutput));

  out.str("");
  lineToOutput = "this is an output line";
  ASSERT_TRUE((console.*writeFn)(lineToOutput));
  ASSERT_NE(out.str().find(lineToOutput), std::string::npos);

  out.str("");
  lineToOutput = "this is a different output line";
  ASSERT_TRUE((console.*writeFn)(lineToOutput));
  ASSERT_NE(out.str().find(lineToOutput), std::string::npos);

  out.str("");
  lineToOutput = "";
  ASSERT_TRUE((console.*writeFn)(lineToOutput));

  out.str("");
  lineToOutput = "multiline output\nline with\nthree lines";
  ASSERT_TRUE((console.*writeFn)(lineToOutput));
  ASSERT_NE(out.str().find(lineToOutput), std::string::npos);
}

TEST(ConsoleInterface, TestWriteOutputLine) {
  TestWriteFn(&ConsoleInterface::WriteOutputLine);
}

TEST(ConsoleInterface, TestWriteError) {
  TestWriteFn(&ConsoleInterface::WriteError);
}
