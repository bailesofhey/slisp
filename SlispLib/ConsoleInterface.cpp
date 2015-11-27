#include <iostream>

#include "ConsoleInterface.h"

using namespace std;

ConsoleInterface::ConsoleInterface()
{
  SetInput();
  SetOutput();
}

ConsoleInterface::ConsoleInterface(istream &in, ostream &out)
{
  SetInput(in);
  SetOutput(out);
}

void ConsoleInterface::Reset() {
  HasMore_ = true;
}

bool ConsoleInterface::HasMore() const {
  return HasMore_;
}

bool ConsoleInterface::ReadInputLine(string &input) {
  return ReadLine(">>> ", input);
}

bool ConsoleInterface::ReadContinuedInputLine(string &input) {
  return ReadLine("... ", input);
}

bool ConsoleInterface::WriteOutputLine(const string &output) {
  *Out << output;
  return true;
}

bool ConsoleInterface::WriteError(const string &error) {
  *Out << "Error: " << error << endl;
  return true;
}

void ConsoleInterface::SetInput() {
  SetInput(cin);
}

void ConsoleInterface::SetInput(std::istream &in) {
  In = &in;
  Reset();
}

void ConsoleInterface::SetOutput() {
  SetOutput(cout);
}

void ConsoleInterface::SetOutput(std::ostream &out) {
  Out = &out;
}

bool ConsoleInterface::ReadLine(const string &prefix, string &input) {
  *Out << prefix;
  getline(*In, input);
  HasMore_ = !In->eof();
  if (!HasMore_)
    In->clear();
  return true;
}