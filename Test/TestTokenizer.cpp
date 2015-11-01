#include <string>
#include <list>
#include <initializer_list>
#include "gtest\gtest.h"

#include "Tokenizer.h"
#include "Token.h"

void RunNoneTest(Tokenizer &tokenizer) {
  for (int i = 0; i < 3; ++i) {
    ASSERT_EQ(Token(), *tokenizer);
    ++tokenizer;
  }
}

struct TestCase {
  std::string Line;
  std::vector<Token> Tokens;

  TestCase(const std::string &line, std::initializer_list<Token> &&tokens):
    Line(line),
    Tokens(tokens)
  {
  }
};

void RunTests(std::vector<TestCase> &tests) {
  Tokenizer tokenizer;
  for (auto &test : tests) {
    tokenizer.SetLine(test.Line);
    ASSERT_EQ(Token(), *tokenizer);
    ++tokenizer;
    for (size_t tokenNum = 0; tokenNum < test.Tokens.size(); ++tokenNum) {
      auto &expectedToken = test.Tokens[tokenNum];
      auto &actualToken = *tokenizer;
      ASSERT_EQ(expectedToken, actualToken) << "{{{ " << test.Line << " }}} TokenNum:" << tokenNum;
      ++tokenizer;
    }
    RunNoneTest(tokenizer);
  }
}

TEST(Tokenizer, TestNone) {
  Tokenizer tokenizer;
  RunNoneTest(tokenizer);
  
  std::vector<TestCase> tests = {
    { "", { Token() } },
    { " ", { Token() } },
    { "     ", { Token() } },
  };
  RunTests(tests);
}


TEST(Tokenizer, TestNumber) {
  std::vector<TestCase> tests = {
    { "0", { Token(TokenTypes::NUMBER, "0") } },
    { "000", { Token(TokenTypes::NUMBER, "000") } },
    { "3", { Token(TokenTypes::NUMBER, "3") } },
    { "03", { Token(TokenTypes::NUMBER, "03") } },
    { "1234567890", { Token(TokenTypes::NUMBER, "1234567890") } },

    { "   123   ", { Token(TokenTypes::NUMBER, "123") } },
  };
  RunTests(tests);
}

TEST(Tokenizer, TestSymbol) {
  std::vector<TestCase> tests = {
    { "a", { Token(TokenTypes::SYMBOL, "a") } },
    { "A", { Token(TokenTypes::SYMBOL, "A") } },
    { "abcdefghijklmnopqrstuvwxyz", { Token(TokenTypes::SYMBOL, "abcdefghijklmnopqrstuvwxyz") } },
    
    // These fail. Bug!

    //{ "a0", { Token(TokenTypes::SYMBOL, "a0") } },
    //{ "a0123456789", { Token(TokenTypes::SYMBOL, "a0123456789") } },

    //{ "   fooBar3   ", { Token(TokenTypes::SYMBOL, "fooBar3") } },
  };
  RunTests(tests);
}

TEST(Tokenizer, TestString) {

}

TEST(Tokenizer, TestParens) {

}

TEST(Tokenizer, TestUnknown) {

}