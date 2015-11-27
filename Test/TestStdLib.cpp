#include <iostream>
#include <fstream>
#include <sstream>
#include "gtest\gtest.h"

#include "Interpreter.h"
#include "ConsoleInterface.h"
#include "Tokenizer.h"
#include "Parser.h"

class StdLibTest: public ::testing::Test {
  protected:
    static CommandInterface *CmdInterface;
    static Interpreter *Interpreter_;
    static Tokenizer *Tokenizer_;
    static Parser *Parser_;
    static std::fstream In;
    static std::stringstream Out;
  
    static void SetUpTestCase() {
      In.open("TestStdLib.slisp", std::ios_base::in);
      CmdInterface = new ConsoleInterface(In, Out);
      Interpreter_ = new Interpreter(*CmdInterface);
      Tokenizer_ = new Tokenizer();
      Parser_ = new Parser(*CmdInterface, *Tokenizer_, Interpreter_->GetDefaultSexp());
    }

    static void TearDownTestCase() {
      delete Parser_;
      delete Tokenizer_;
      delete Interpreter_;
      delete CmdInterface;
      In.close();
      Out.clear();
    }
};