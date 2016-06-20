#include "CommandInterface.h"

using namespace std;

CommandInterface::~CommandInterface() {
}

bool CommandInterface::ReadInputLine(string &input) {
  return ReadLine(">>> ", input);
}

bool CommandInterface::ReadContinuedInputLine(string &input) {
  return ReadLine("... ", input);
}

void CommandInterface::SetInteractiveMode(bool enabled) {
}

void CommandInterface::GetInteractiveMode(bool &enabled) {
}