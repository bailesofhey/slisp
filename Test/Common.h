#include "gtest\gtest.h"

#include "Parser.h"
#include "CommandInterface.h"
#include "Tokenizer.h"

class TestCommandInterface: public CommandInterface {
  public:
    std::string Input;  
    bool Result;

    TestCommandInterface(): Result(true)                    {}
    virtual ~TestCommandInterface()                         {}
    virtual bool ReadInputLine(std::string &input)          { input = Input; return Result; }
    virtual bool ReadContinuedInputLine(std::string &input) { return ReadInputLine(input); }
    virtual bool WriteOutputLine(const std::string &output) { return Result; }
    virtual bool WriteError(const std::string &error)       { return Result; }
};

class TestTokenizer: public ITokenizer {
  public:
    std::list<Token> Tokens;
    Token CurrToken;

    virtual void SetLine(const std::string& line) {}

    virtual ITokenizer& operator++() {
      if (Tokens.empty())
        CurrToken = Token(TokenTypes::NONE, "");
      else {
        CurrToken = Tokens.front();
        Tokens.pop_front();
      }
      return *this;
    }

    virtual Token& operator*() { return CurrToken; }
};