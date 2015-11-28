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

  TestCase(const std::string &line, std::vector<Token> &tokens):
    Line(line),
    Tokens(tokens)
  {
  }
};

void RunTest(Tokenizer &tokenizer, const TestCase &test) {
  tokenizer.SetLine(test.Line);
  ASSERT_EQ(Token(), *tokenizer) << "Leading tokens";
  ++tokenizer;
  for (size_t tokenNum = 0; tokenNum < test.Tokens.size(); ++tokenNum) {
    auto &expectedToken = test.Tokens[tokenNum];
    auto &actualToken = *tokenizer;
    ASSERT_EQ(expectedToken, actualToken) << "TokenNum:" << tokenNum;
    ++tokenizer;
  }
  ASSERT_NO_FATAL_FAILURE(RunNoneTest(tokenizer)) << "Trailing tokens";
}

void RunTests(std::initializer_list<TestCase> &&tests) {
  Tokenizer tokenizer;
  for (const auto &test : tests)
    ASSERT_NO_FATAL_FAILURE(RunTest(tokenizer, test)) << "{{{ " << test.Line << " }}}";
}

TEST(Tokenizer, TestNone) {
  Tokenizer tokenizer;
  ASSERT_NO_FATAL_FAILURE(RunNoneTest(tokenizer));
  RunTests({
    { "", { Token() } },
    { " ", { Token() } },
    { "     ", { Token() } },
  });
}

TEST(Tokenizer, TestNumber) {
  RunTests({
    { "0", { Token(TokenTypes::NUMBER, "0") } },
    { "000", { Token(TokenTypes::NUMBER, "000") } },
    { "3", { Token(TokenTypes::NUMBER, "3") } },
    { "03", { Token(TokenTypes::NUMBER, "03") } },
    { "1234567890", { Token(TokenTypes::NUMBER, "1234567890") } },
    { "-42", { Token(TokenTypes::NUMBER, "-42") } },

    { "   123   ", { Token(TokenTypes::NUMBER, "123") } },
    { " 1 23  345 ", { Token(TokenTypes::NUMBER, "1"), Token(TokenTypes::NUMBER, "23"), Token(TokenTypes::NUMBER, "345") } },
  });
}

TEST(Tokenizer, TestSymbol) {
  RunTests({
    { "a", { Token(TokenTypes::SYMBOL, "a") } },
    { "A", { Token(TokenTypes::SYMBOL, "A") } },
    { "abcdefghijklmnopqrstuvwxyz", { Token(TokenTypes::SYMBOL, "abcdefghijklmnopqrstuvwxyz") } },
    
    { "~", { Token(TokenTypes::SYMBOL, "~") } },
    { "`", { Token(TokenTypes::SYMBOL, "`") } },
    { "!", { Token(TokenTypes::SYMBOL, "!") } },
    { "@", { Token(TokenTypes::SYMBOL, "@") } },
    { "#", { Token(TokenTypes::SYMBOL, "#") } },
    { "$", { Token(TokenTypes::SYMBOL, "$") } },
    { "%", { Token(TokenTypes::SYMBOL, "%") } },
    { "^", { Token(TokenTypes::SYMBOL, "^") } },
    { "&", { Token(TokenTypes::SYMBOL, "&") } },
    { "*", { Token(TokenTypes::SYMBOL, "*") } },
    { "-", { Token(TokenTypes::SYMBOL, "-") } },
    { "_", { Token(TokenTypes::SYMBOL, "_") } },
    { "+", { Token(TokenTypes::SYMBOL, "+") } },
    { "=", { Token(TokenTypes::SYMBOL, "=") } },
    { "{", { Token(TokenTypes::SYMBOL, "{") } },
    { "[", { Token(TokenTypes::SYMBOL, "[") } },
    { "}", { Token(TokenTypes::SYMBOL, "}") } },
    { "|", { Token(TokenTypes::SYMBOL, "|") } },
    { "\\", { Token(TokenTypes::SYMBOL, "\\") } },
    { ":", { Token(TokenTypes::SYMBOL, ":") } },
    { ";", { Token(TokenTypes::SYMBOL, ";") } },
    { "'", { Token(TokenTypes::SYMBOL, "'") } },
    { "<", { Token(TokenTypes::SYMBOL, "<") } },
    { ",", { Token(TokenTypes::SYMBOL, ",") } },
    { ">", { Token(TokenTypes::SYMBOL, ">") } },
    { ".", { Token(TokenTypes::SYMBOL, ".") } },
    { "?", { Token(TokenTypes::SYMBOL, "?") } },
    { "/", { Token(TokenTypes::SYMBOL, "/") } },

    { "a0", { Token(TokenTypes::SYMBOL, "a0") } },
    { "a0123456789", { Token(TokenTypes::SYMBOL, "a0123456789") } },
    { "   fooBar3   ", { Token(TokenTypes::SYMBOL, "fooBar3") } },

    { " foo  BAR  $baraaa**z", {
      Token(TokenTypes::SYMBOL, "foo"),
      Token(TokenTypes::SYMBOL, "BAR"),
      Token(TokenTypes::SYMBOL, "$baraaa**z"),
    }},

  });
}

TEST(Tokenizer, TestString) {
  RunTests({
    { "\"\"", { Token(TokenTypes::STRING, "") } },
    { "\"Hello, world!\"", { Token(TokenTypes::STRING, "Hello, world!") } },
    { "\" \"", { Token(TokenTypes::STRING, " ") } },

    { "   \"   \"   ", { Token(TokenTypes::STRING, "   ") } },
    { "   \" a  \"   ", { Token(TokenTypes::STRING, " a  ") } },

    { " \" \" \" \"   \"a\"   \"  a\" \"a   \" \"  a b \"  ", {
      Token(TokenTypes::STRING, " "),
      Token(TokenTypes::STRING, " "),
      Token(TokenTypes::STRING, "a"),
      Token(TokenTypes::STRING, "  a"),
      Token(TokenTypes::STRING, "a   "),
      Token(TokenTypes::STRING, "  a b "),
    }},

    // TODO: need to support escape characters: \" \\ \n \r \t
  });
}

TEST(Tokenizer, TestParens) {
  RunTests({
    { "(", { Token(TokenTypes::PARENOPEN, "(") } },
    { "  (   ", { Token(TokenTypes::PARENOPEN, "(") } },

    { ")", { Token(TokenTypes::PARENCLOSE, ")") } },
    { "  )   ", { Token(TokenTypes::PARENCLOSE, ")") } },

    { "()", { Token(TokenTypes::PARENOPEN, "("), Token(TokenTypes::PARENCLOSE, ")") } },
    { " ( ())) )  () ", {
      Token(TokenTypes::PARENOPEN, "("),
      Token(TokenTypes::PARENOPEN, "("),
      Token(TokenTypes::PARENCLOSE, ")"),
      Token(TokenTypes::PARENCLOSE, ")"),
      Token(TokenTypes::PARENCLOSE, ")"),
      Token(TokenTypes::PARENCLOSE, ")"),
      Token(TokenTypes::PARENOPEN, "("),
      Token(TokenTypes::PARENCLOSE, ")"),
    }},

  });
}

TEST(Tokenizer, TestUnknown) {
  RunTests({
    { "0a", { Token(TokenTypes::UNKNOWN, "0a") } },
    { "3*", { Token(TokenTypes::UNKNOWN, "3*") } },
  });
}

TEST(Tokenizer, TestSimpleSexp) {
  std::vector<Token> tokens = {
    Token(TokenTypes::PARENOPEN, "("),
    Token(TokenTypes::SYMBOL, "+"),
    Token(TokenTypes::NUMBER, "2"),
    Token(TokenTypes::NUMBER, "3"),
    Token(TokenTypes::PARENCLOSE, ")"),
  };
  RunTests({
    { "(+ 2 3)", tokens},
    { "  (  +   2   3  )   ", tokens },
  });
}

TEST(Tokenizer, TestComplex) {
  RunTests({
    { "(if (< a 0) \"less than\" (+ \"greater than\" \"or equal to\") ) ", {
      Token(TokenTypes::PARENOPEN, "("),
        Token(TokenTypes::SYMBOL, "if"),
          Token(TokenTypes::PARENOPEN, "("),
            Token(TokenTypes::SYMBOL, "<"),
            Token(TokenTypes::SYMBOL, "a"),
            Token(TokenTypes::NUMBER, "0"),
          Token(TokenTypes::PARENCLOSE, ")"),
          Token(TokenTypes::STRING, "less than"),
          Token(TokenTypes::PARENOPEN, "("),
            Token(TokenTypes::SYMBOL, "+"),
            Token(TokenTypes::STRING, "greater than"),
            Token(TokenTypes::STRING, "or equal to"),
          Token(TokenTypes::PARENCLOSE, ")"),
      Token(TokenTypes::PARENCLOSE, ")"),
    }},
  });
}

TEST(Tokenizer, TestSetLine) {
  Tokenizer tokenizer;
  tokenizer.SetLine("(+ 2 3)");
  ++tokenizer;
  ASSERT_EQ(Token(TokenTypes::PARENOPEN, "("), *tokenizer);
  ++tokenizer;
  ASSERT_EQ(Token(TokenTypes::SYMBOL, "+"), *tokenizer);
  tokenizer.SetLine("\"foo\"");
  ++tokenizer;
  ASSERT_EQ(Token(TokenTypes::STRING, "foo"), *tokenizer);
  ++tokenizer;
  RunNoneTest(tokenizer);
}
