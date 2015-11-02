#include <iostream>
#include <sstream>
#include <functional>

#include "Tokenizer.h"
#include "Parser.h"
#include "ConsoleInterface.h"
#include "Expression.h"
#include "Interpreter.h"
#include "StdLib\StdLib.h"

using std::cout;
using std::cin;
using std::endl;

//=============================================================================

std::unique_ptr<CommandInterface> CreateCommandInterface() {
  return std::unique_ptr<CommandInterface>(new ConsoleInterface);
}

int main(int argc, char **argv) {
  Interpreter interpreter;

  StdLib lib;
  lib.Load(interpreter);

  while (!interpreter.StopRequested()) {
    auto cmdInterface = CreateCommandInterface();
    Parser parser { *cmdInterface, interpreter.GetDefaultSexp() };
    if (parser.Parse()) {
      auto &exprTree = parser.ExpressionTree();
      if (exprTree) {
        ExpressionPtr root { exprTree.release() };
        if (!interpreter.Evaluate(root)) {
          auto errors = interpreter.GetErrors();
          //errors.pop_back();
          //errors.pop_back();
          for (auto &error : errors)
            cout << error.Where << ": " << error.What << endl;
        }
        interpreter.ClearErrors();
      }
      else
        cout << "Parse Error: No Expression Tree" << endl;
    }
    else
      cout << "Parse Error: " << parser.Error() << endl;
  }

  return 0;
}