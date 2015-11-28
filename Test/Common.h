#pragma once

#include "gtest\gtest.h"

#include "Parser.h"
#include "CommandInterface.h"
#include "Tokenizer.h"

class TestCommandInterface: public CommandInterface {
  public:
    std::string Input;
    std::string Output;
    std::string Error;
    bool Result;
    bool HasMore_;

    TestCommandInterface(): Result(true)                             { Reset(); }
    virtual ~TestCommandInterface() override                         {}
    virtual void Reset()                                             { HasMore_ = true;}
    virtual bool HasMore() const override                            { return HasMore_; }
    virtual bool ReadInputLine(std::string &input) override          {input = Input; HasMore_ = false; return Result; }
    virtual bool ReadContinuedInputLine(std::string &input) override { return ReadInputLine(input); }
    virtual bool WriteOutputLine(const std::string &output) override { Output = output; return Result; }
    virtual bool WriteError(const std::string &error)       override { Error = error; return Result; }
};

class TestTokenizer: public ITokenizer {
  public:
    std::list<Token> Tokens;
    Token CurrToken;

    virtual void SetLine(const std::string& line) override {}

    virtual ITokenizer& operator++() override {
      if (Tokens.empty())
        CurrToken = Token(TokenTypes::NONE, "");
      else {
        CurrToken = Tokens.front();
        Tokens.pop_front();
      }
      return *this;
    }

    virtual Token& operator*() override { return CurrToken; }
};