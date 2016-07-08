#include <string>
#include <list>
#include <initializer_list>
#include "gtest\gtest.h"

#include "Tokenizer.h"
#include "Token.h"

using namespace std;

void RunNoneTest(Tokenizer &tokenizer) {
  for (int i = 0; i < 3; ++i) {
    ASSERT_EQ(Token(), *tokenizer);
    ++tokenizer;
  }
}

struct TestCase {
  string Line;
  vector<Token> Tokens;

  TestCase(const string &line, initializer_list<Token> &&tokens):
    Line(line),
    Tokens(tokens)
  {
  }

  TestCase(const string &line, vector<Token> &tokens):
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

void RunTests(initializer_list<TestCase> &&tests) {
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

    // comments
    { "# this is a comment", { Token() } },
    { "# this is a \n# multiline\n# comment!", { Token() } },
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

    { "0x0", { Token(TokenTypes::NUMBER, "0x0") } },
    { "0x9", { Token(TokenTypes::NUMBER, "0x9") } },
    { "0xa", { Token(TokenTypes::NUMBER, "0xa") } },
    { "0xA", { Token(TokenTypes::NUMBER, "0xa") } },
    { "0x0123456789ABCDEF", { Token(TokenTypes::NUMBER, "0x0123456789abcdef") } },

    { "0.0", { Token(TokenTypes::NUMBER, "0.0") } },
    { "3.14", { Token(TokenTypes::NUMBER, "3.14") } },
    { "3.14159265359", { Token(TokenTypes::NUMBER, "3.14159265359") } },
    { "-3.14", { Token(TokenTypes::NUMBER, "-3.14") } },
    { "3.14e1", { Token(TokenTypes::NUMBER, "3.14e1") } },
    { "3.14e23", { Token(TokenTypes::NUMBER, "3.14e23") } },
    { "-3.14e23", { Token(TokenTypes::NUMBER, "-3.14e23") } },
    { "-3.14e-23", { Token(TokenTypes::NUMBER, "-3.14e-23") } },
    { "3e2", { Token(TokenTypes::NUMBER, "3e2") } },
    { "3e-2", { Token(TokenTypes::NUMBER, "3e-2") } },

    { "   123   ", { Token(TokenTypes::NUMBER, "123") } },
    { " 1 23  345 ", { Token(TokenTypes::NUMBER, "1"), Token(TokenTypes::NUMBER, "23"), Token(TokenTypes::NUMBER, "345") } },

    { "123 # this is tokenize as a number", { Token(TokenTypes::NUMBER, "123") } },
  });
}

TEST(Tokenizer, TestSymbol) {
  RunTests({
    { "a", { Token(TokenTypes::SYMBOL, "a") } },
    { "x", { Token(TokenTypes::SYMBOL, "x") } },
    { "A", { Token(TokenTypes::SYMBOL, "A") } },
    { "abcdefghijklmnopqrstuvwxyz", { Token(TokenTypes::SYMBOL, "abcdefghijklmnopqrstuvwxyz") } },
    
    { "~", { Token(TokenTypes::SYMBOL, "~") } },
    { "`", { Token(TokenTypes::SYMBOL, "`") } },
    { "!", { Token(TokenTypes::SYMBOL, "!") } },
    { "@", { Token(TokenTypes::SYMBOL, "@") } },
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

    { "foobar # this is tokenize as a symbol", { Token(TokenTypes::SYMBOL, "foobar") } },
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

    { "\"Hello, world!\" # this is tokenize as a string", { Token(TokenTypes::STRING, "Hello, world!") } },
    { "\"This string has an octothorpe #\" # this is tokenize as a string and not loose the hash mark", { Token(TokenTypes::STRING, "This string has an octothorpe #") } },

    // escapes
    { "\"\\n\"", { Token(TokenTypes::STRING, "\\n") } },
    { "\"\\t\"", { Token(TokenTypes::STRING, "\\t") } },
    { "\"\\v\"", { Token(TokenTypes::STRING, "\\v") } },
    { "\"\\b\"", { Token(TokenTypes::STRING, "\\b") } },
    { "\"\\r\"", { Token(TokenTypes::STRING, "\\r") } },
    { "\"\\f\"", { Token(TokenTypes::STRING, "\\f") } },
    { "\"\\a\"", { Token(TokenTypes::STRING, "\\a") } },
    { "\"\\\\\"", { Token(TokenTypes::STRING, "\\\\") } },
    { "\"\\'\"", { Token(TokenTypes::STRING, "\\'") } },
    { "\"\\\"\"", { Token(TokenTypes::STRING, "\\\"") } },
    { "\"\\0\"", { Token(TokenTypes::STRING, "\\0") } },
    { "\"\\z\"", { Token(TokenTypes::UNKNOWN, "\\z") } },

    // hex escapes
    { "\"\\x0\"", { Token(TokenTypes::UNKNOWN, "\\x0") } },
    { "\"\\xa\"", { Token(TokenTypes::UNKNOWN, "\\xa") } },
    { "\"\\xg\"", { Token(TokenTypes::UNKNOWN, "\\xg") } },
    { "\"\\xx\"", { Token(TokenTypes::UNKNOWN, "\\xx") } },
    { "\"\\xn\"", { Token(TokenTypes::UNKNOWN, "\\xn") } },
    { "\"\\x\\\"", { Token(TokenTypes::UNKNOWN, "\\x\\") } },
    { "\"\\x1b\"", { Token(TokenTypes::STRING, "\\x1b") } },
    { "\"\\x1g\"", { Token(TokenTypes::UNKNOWN, "\\x1g") } },
    { "\"\\x1b2\"", { Token(TokenTypes::STRING, "\\x1b2") } },
    { "\"\\x1bg\"", { Token(TokenTypes::STRING, "\\x1bg") } },
    { "\"\\x0123456789abcdef\"", { Token(TokenTypes::STRING, "\\x0123456789abcdef") } },
    { "\"\\x0123456789abcdefg\"", { Token(TokenTypes::STRING, "\\x0123456789abcdefg") } },

    // strings + escape
    { "\"this is a multiline\\nstring\"", { Token(TokenTypes::STRING, "this is a multiline\\nstring") } },
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

TEST(Tokenizer, TestQuote) {
  RunTests({
    {"'", { Token(TokenTypes::QUOTE, "'") } },
    {" ' ", { Token(TokenTypes::QUOTE, "'") } },
    {"'foo", { Token(TokenTypes::QUOTE, "'"), Token(TokenTypes::SYMBOL, "foo") } },
    {"'42", { Token(TokenTypes::QUOTE, "'"), Token(TokenTypes::NUMBER, "42") } },
    {"'\"qux\"", { Token(TokenTypes::QUOTE, "'"), Token(TokenTypes::STRING, "qux") } },
    {"'(+ 1 2)", { 
      Token(TokenTypes::QUOTE, "'"), 
      Token(TokenTypes::PARENOPEN, "("),
      Token(TokenTypes::SYMBOL, "+"),
      Token(TokenTypes::NUMBER, "1"),
      Token(TokenTypes::NUMBER, "2"),
      Token(TokenTypes::PARENCLOSE, ")"),
    }},
  });
}

TEST(Tokenizer, TestUnknown) {
  RunTests({
    { "0a", { Token(TokenTypes::UNKNOWN, "0a") } },
    { "3*", { Token(TokenTypes::UNKNOWN, "3*") } },

    { "0x", { Token(TokenTypes::UNKNOWN, "0x") } },
    { "3x", { Token(TokenTypes::UNKNOWN, "3x") } },
    { "0xabcdefg", { Token(TokenTypes::UNKNOWN, "0xabcdefg") } },
    { "0xfex", { Token(TokenTypes::UNKNOWN, "0xfex") } },
    { "12f", { Token(TokenTypes::UNKNOWN, "12f") } },
    { "1230x123", { Token(TokenTypes::UNKNOWN, "1230x123") } },

    { "0b", { Token(TokenTypes::UNKNOWN, "0b") } },
    { "0b11012", { Token(TokenTypes::UNKNOWN, "0b11012") } },

    { "'a'", { Token(TokenTypes::QUOTE, "'"), Token(TokenTypes::UNKNOWN, "a'") } },
    { "a'", { Token(TokenTypes::UNKNOWN, "a'") } },
    { "'a'b", { Token(TokenTypes::QUOTE, "'"), Token(TokenTypes::UNKNOWN, "a'b") } },
    { "'a'b'", { Token(TokenTypes::QUOTE, "'"), Token(TokenTypes::UNKNOWN, "a'b'") } },
    { "a'b", { Token(TokenTypes::UNKNOWN, "a'b") } },
  });
}

TEST(Tokenizer, TestSimpleSexp) {
  vector<Token> tokens = {
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
