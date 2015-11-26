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

std::unique_ptr<ITokenizer> CreateTokenizer() {
  return std::unique_ptr<ITokenizer>(new Tokenizer);
}

int main(int argc, char **argv) {
  auto cmdInterface = CreateCommandInterface();
  auto tokenizer = CreateTokenizer();
  Interpreter interpreter(*cmdInterface);
  Parser parser { *cmdInterface, *tokenizer, interpreter.GetDefaultSexp() };

  StdLib lib;
  lib.Load(interpreter);

  while (!interpreter.StopRequested()) {
    if (parser.Parse()) {
      auto &exprTree = parser.ExpressionTree();
      if (exprTree) {
        ExpressionPtr root { exprTree.release() };
        if (!interpreter.Evaluate(root)) {
          auto errors = interpreter.GetErrors();
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