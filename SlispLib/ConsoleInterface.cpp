#include <iostream>

#include "ConsoleInterface.h"

using namespace std;

ConsoleInterface::ConsoleInterface():
  ConsoleInterface(cin, cout)
{
}

ConsoleInterface::ConsoleInterface(istream &in, ostream &out):
  In { in },
  Out { out }
{
}

bool ConsoleInterface::ReadInputLine(string &input) {
  return ReadLine(">>> ", input);
}

bool ConsoleInterface::ReadContinuedInputLine(string &input) {
  return ReadLine("... ", input);
}

bool ConsoleInterface::WriteOutputLine(const string &output) {
  Out << output;
  return true;
}

bool ConsoleInterface::WriteError(const string &error) {
  Out << "Error: " << error << endl;
  return true;
}

bool ConsoleInterface::ReadLine(const string &prefix, string &input) {
  Out << prefix;
  getline(In, input);
  if (In.eof())
    In.clear();
  return true;
}