#include <iostream>

#include "ConsoleInterface.h"

using namespace std;

ConsoleInterface::ConsoleInterface()
{
  Init();
  SetInput();
  SetOutput();
}

ConsoleInterface::ConsoleInterface(istream &in, ostream &out)
{
  Init();
  SetInput(in);
  SetOutput(out);
}

void ConsoleInterface::Init() {
  In = nullptr;
  Out = nullptr;
  HasMore_ = true;
  InteractiveMode_ = true;
}

void ConsoleInterface::Reset() {
  HasMore_ = true;
}

bool ConsoleInterface::HasMore() const {
  return HasMore_;
}

bool ConsoleInterface::ReadLine(const string &prefix, string &input) {
  if (InteractiveMode_)
    *Out << prefix;
  getline(*In, input);
  HasMore_ = !In->eof();
  if (!HasMore_)
    In->clear();
  return true;
}

bool ConsoleInterface::WriteOutputLine(const string &output) {
  *Out << output;
  return true;
}

bool ConsoleInterface::WriteError(const string &error) {
  *Out << "Error: " << error << endl;
  return true;
}

std::istream& ConsoleInterface::GetInput() {
  return *In;
}

std::ostream& ConsoleInterface::GetOutput() {
  return *Out;
}

void ConsoleInterface::SetInput() {
  SetInput(cin);
}

void ConsoleInterface::SetInput(istream &in) {
  In = &in;
  Reset();
}

void ConsoleInterface::SetOutput() {
  SetOutput(cout);
}

void ConsoleInterface::SetOutput(ostream &out) {
  Out = &out;
}

void ConsoleInterface::SetInteractiveMode(bool enabled) {
  InteractiveMode_ = enabled;
}

void ConsoleInterface::GetInteractiveMode(bool &enabled) {
  enabled = InteractiveMode_;
}
