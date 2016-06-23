#include "gtest\gtest.h"
#include <iostream>
#include <sstream>
#include "ConsoleInterface.h"

using namespace std;

using ReadFn = bool(ConsoleInterface::*)(string &);

void TestSingleRead(ConsoleInterface &console, stringstream &out, ReadFn readFn, const string &expectedStr) {
  string currLine;
  ASSERT_TRUE((console.*readFn)(currLine));
  ASSERT_EQ(expectedStr, currLine);
  ASSERT_GT(out.str().length(), (size_t)0);
  out.str("");
}

void TestReadFn(ReadFn readFn) {
  stringstream in,
                    out;
  ConsoleInterface  console(in, out);

  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));
  
  in << "this is line number 1" << endl
     << "Line #2" << endl
     << endl;
  ASSERT_FALSE(console.HasMore());
  console.Reset();
  ASSERT_TRUE(console.HasMore());
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, "this is line number 1"));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, "Line #2"));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));
  ASSERT_FALSE(console.HasMore());

  in << "Another first line" << endl;
  ASSERT_FALSE(console.HasMore());
  console.Reset();
  ASSERT_TRUE(console.HasMore());
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, "Another first line"));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));
  ASSERT_FALSE(console.HasMore());

  in << "Incomplete line";
  ASSERT_FALSE(console.HasMore());
  console.Reset();
  ASSERT_TRUE(console.HasMore());
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, "Incomplete line"));
  ASSERT_NO_FATAL_FAILURE(TestSingleRead(console, out, readFn, ""));
  ASSERT_FALSE(console.HasMore());
}

TEST(ConsoleInterface, TestReadInputLine) {
  TestReadFn(&ConsoleInterface::ReadInputLine);
}

TEST(ConsoleInterface, TestReadContinuedInputLine) {
  TestReadFn(&ConsoleInterface::ReadContinuedInputLine);
}

using WriteFn = bool(ConsoleInterface::*)(const string &);
void TestWriteFn(WriteFn writeFn) {
  stringstream in,
                    out;
  ConsoleInterface  console(in, out);
  string       lineToOutput = "";
  ASSERT_TRUE((console.*writeFn)(lineToOutput));

  out.str("");
  lineToOutput = "this is an output line";
  ASSERT_TRUE((console.*writeFn)(lineToOutput));
  ASSERT_NE(out.str().find(lineToOutput), string::npos);

  out.str("");
  lineToOutput = "this is a different output line";
  ASSERT_TRUE((console.*writeFn)(lineToOutput));
  ASSERT_NE(out.str().find(lineToOutput), string::npos);

  out.str("");
  lineToOutput = "";
  ASSERT_TRUE((console.*writeFn)(lineToOutput));

  out.str("");
  lineToOutput = "multiline output\nline with\nthree lines";
  ASSERT_TRUE((console.*writeFn)(lineToOutput));
  ASSERT_NE(out.str().find(lineToOutput), string::npos);
}

TEST(ConsoleInterface, TestWriteOutputLine) {
  TestWriteFn(&ConsoleInterface::WriteOutputLine);
}

TEST(ConsoleInterface, TestWriteError) {
  TestWriteFn(&ConsoleInterface::WriteError);
}

TEST(ConsoleInterface, TestSetInputOutput) {
  stringstream in1,
                    out1,
                    in2,
                    out2;
  string       line;
  ConsoleInterface  console(in1, out1);
  in1 << "in1" << endl;

  ASSERT_TRUE(console.ReadInputLine(line));
  ASSERT_NE(line.find("in1"), string::npos);
  ASSERT_TRUE(console.WriteOutputLine("out1"));
  ASSERT_NE(out1.str().find("out1"), string::npos);
  ASSERT_TRUE(console.ReadInputLine(line));
  ASSERT_TRUE(line.empty());
  ASSERT_FALSE(console.HasMore());

  console.SetInput(in2);
  ASSERT_TRUE(console.HasMore());
  console.SetOutput(out2);
  in2 << "in2" << endl;
  ASSERT_TRUE(console.ReadInputLine(line));
  ASSERT_NE(line.find("in2"), string::npos);
  ASSERT_TRUE(console.WriteOutputLine("out2"));
  ASSERT_NE(out2.str().find("out2"), string::npos);
  ASSERT_TRUE(console.ReadInputLine(line));
  ASSERT_TRUE(line.empty());
  ASSERT_FALSE(console.HasMore());
}
