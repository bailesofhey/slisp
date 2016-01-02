#include <initializer_list>
#include <limits>
#include "gtest\gtest.h"

#include "Parser.h"
#include "CommandInterface.h"
#include "Tokenizer.h"
#include "InterpreterUtils.h"

#include "Common.h"

class ParserTest: public ::testing::Test {
  public:
    ParserTest():
      DummySymbols(),
      Settings(DummySymbols),
      CommandInterface(),
      Tokenizer(),
      Parser(CommandInterface, Tokenizer, Settings)
    {
      DummySymbols.PutSymbolFunction("+", &DummyFn, FuncDef { FuncDef::NoArgs(), FuncDef::NoArgs() });
      Settings.RegisterInfixSymbol("+");
      Settings.RegisterInfixSymbol("=");
    }

    static bool DummyFn(EvaluationContext &ctx) {
      return false;
    }
  
  protected:
    SymbolTable DummySymbols;
    InterpreterSettings Settings;
    TestCommandInterface CommandInterface;
    TestTokenizer Tokenizer;
    Parser Parser;

    void TestParse(std::initializer_list<Token> &&tokens, bool expectSuccess, std::initializer_list<Expression*> &&expectedArgs) {
      Tokenizer.Tokens = tokens;
      ASSERT_EQ(expectSuccess, Parser.Parse());
      ASSERT_EQ(expectSuccess, Parser.Error().empty());
      auto exprTree = Parser.ExpressionTree();
      ASSERT_EQ(1 + expectedArgs.size(), exprTree->Args.size());
      auto sym = dynamic_cast<Symbol*>(exprTree->Args.front().get());
      ASSERT_TRUE(sym != nullptr);
      ASSERT_EQ(Settings.GetDefaultSexp(), sym->Value);
      exprTree->Args.pop_front();

      ArgList expected;
      for (auto *expectedArg : expectedArgs)
        expected.push_back(expectedArg->Clone());
      ASSERT_TRUE(ArgListHelper::AreEqual(exprTree->Args, expected));
    }

    void TestParseFail(std::initializer_list<Token> &&tokens) {
      TestParse(std::move(tokens), false, {});
    }

    void TestParseOk(std::initializer_list<Token> &&tokens, std::initializer_list<Expression*> &&expectedArgs) {
      TestParse(std::move(tokens), true, std::move(expectedArgs));
    }
};

#define ASSERT_PARSE(...) ASSERT_NO_FATAL_FAILURE(TestParseOk(__VA_ARGS__))
#define ASSERT_NOPARSE(...) ASSERT_NO_FATAL_FAILURE(TestParseFail(__VA_ARGS__))

TEST_F(ParserTest, TestSingleNone) {
  ASSERT_PARSE({ Token(TokenTypes::NONE, "") }, { });
  ASSERT_PARSE({ Token(TokenTypes::NONE, "foo") }, {});
}

TEST_F(ParserTest, TestSingleUnknown) {
  ASSERT_NOPARSE({ Token(TokenTypes::UNKNOWN, "") });
  ASSERT_NOPARSE({ Token(TokenTypes::UNKNOWN, "foo") });
}

TEST_F(ParserTest, TestSingleHexInt) {
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x0") }, { new Int(0) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x9") }, { new Int(9) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0xa") }, { new Int(10) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x10") }, { new Int(16) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x103") }, { new Int(259) });
}

TEST_F(ParserTest, TestSingleBinaryInt) {
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0b0") }, { new Int(0) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0b1") }, { new Int(1) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0b10") }, { new Int(2) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0b1101") }, { new Int(13) });
}

TEST_F(ParserTest, TestSingleInt) {
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "42") }, { new Int(42) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "042") }, { new Int(42) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "000000000042") }, { new Int(42) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "00000000000000000000000000000000000000000042") }, { new Int(42) });

  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "-3") }, { new Int(-3) });

  ASSERT_NOPARSE({ Token(TokenTypes::NUMBER, "") });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0") }, { new Int(0) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "000") }, { new Int(0) });

  auto minValue = std::numeric_limits<decltype(Int::Value)>::min(),
       maxValue = std::numeric_limits<decltype(Int::Value)>::max();
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, std::to_string(minValue)) }, { new Int(minValue) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, std::to_string(maxValue)) }, { new Int(maxValue) });
}

TEST_F(ParserTest, TestSingleSymbol) {
  ASSERT_PARSE({ Token(TokenTypes::SYMBOL, "a") }, { new Symbol("a") });
  ASSERT_PARSE({ Token(TokenTypes::SYMBOL, "1") }, { new Symbol("1") });
  ASSERT_PARSE({ Token(TokenTypes::SYMBOL, "+") }, { new Symbol("+") });
  ASSERT_NOPARSE({ Token(TokenTypes::SYMBOL, "") });

  std::string longSym = "abcdefghijklmnopqrstuvwxyz0123456789";
  ASSERT_PARSE({ Token(TokenTypes::SYMBOL, longSym) }, { new Symbol(longSym) });
}

TEST_F(ParserTest, TestSingleString) {
  ASSERT_PARSE({ Token(TokenTypes::STRING, "") }, { new String() });
  ASSERT_PARSE({ Token(TokenTypes::STRING, "") }, { new String("") });
  ASSERT_PARSE({ Token(TokenTypes::STRING, " ") }, { new String(" ") });
  ASSERT_PARSE({ Token(TokenTypes::STRING, "        ") }, { new String("        ") });

  ASSERT_PARSE({ Token(TokenTypes::STRING, "foo") }, { new String("foo") });
  ASSERT_PARSE({ Token(TokenTypes::STRING, "foo bar baz") }, { new String("foo bar baz") });

  // Escaping?
  // Unicode?

  ASSERT_PARSE({ Token(TokenTypes::STRING, "123") }, { new String("123") });
}

TEST_F(ParserTest, TestMultiples) {
  ASSERT_PARSE(
    { Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::NUMBER, "43")},
    { new Int(42), new Int(43) }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "foo"), Token(TokenTypes::SYMBOL, "bar") },
    { new Symbol("foo"), new Symbol("bar") }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::STRING, "hello"), Token(TokenTypes::STRING, "world!") },
    { new String("hello"), new String("world!") }
  );

  ASSERT_PARSE(
    { 
      Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::SYMBOL, "foo"), Token(TokenTypes::STRING, "hello"),
      Token(TokenTypes::NUMBER, "43"), Token(TokenTypes::SYMBOL, "bar"), Token(TokenTypes::STRING, "world!")
    },
    {
      new Int(42), new Symbol("foo"), new String("hello"),
      new Int(43), new Symbol("bar"), new String("world!") 
    }
  );
}

TEST_F(ParserTest, TestUnterminatedSexp) {
  ASSERT_NOPARSE(
    { Token(TokenTypes::PARENOPEN, ""), Token(TokenTypes::SYMBOL, "foo") }
  );
}

TEST_F(ParserTest, TestSimpleSexps) {
  ASSERT_PARSE(
    { Token(TokenTypes::PARENOPEN, ""), Token(TokenTypes::PARENCLOSE, "") },
    { new Sexp({ ExpressionPtr { } }) }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::PARENOPEN, ""), Token(TokenTypes::SYMBOL, "foo"), Token(TokenTypes::PARENCLOSE, "") },
    { new Sexp({ ExpressionPtr { new Symbol("foo") } }) }
  );
}

TEST_F(ParserTest, TestNoInfix) {
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "*"), Token(TokenTypes::NUMBER, "42") },
    { new Symbol("x"), new Symbol("*"), new Int(42) }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::PARENOPEN, "("), Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "*"), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::PARENCLOSE, ")") },
    { new Sexp({ ExpressionPtr { new Symbol("x") }, ExpressionPtr { new Symbol("*") }, ExpressionPtr { new Int(42) } }) }
  );
}

TEST_F(ParserTest, TestImplicitInfix_Set) {
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::NUMBER, "42") },
    { new Sexp({ ExpressionPtr { new Symbol("=") }, ExpressionPtr { new Symbol("x") }, ExpressionPtr { new Int(42) }})}
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::STRING, "foo") },
    { new Sexp({ ExpressionPtr { new Symbol("=") }, ExpressionPtr { new Symbol("x") }, ExpressionPtr { new String("foo") }})}
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::SYMBOL, "n") },
    { new Sexp({ ExpressionPtr { new Symbol("=") }, ExpressionPtr { new Symbol("x") }, ExpressionPtr { new Symbol("n") }})}
  );

  // Functions

  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::PARENOPEN, "("), Token(TokenTypes::SYMBOL, "myFn"), Token(TokenTypes::PARENCLOSE, ")") },
    { new Sexp({ ExpressionPtr { new Symbol("=") }, ExpressionPtr { new Symbol("x") }, ExpressionPtr { new Sexp({ExpressionPtr { new Symbol("myFn") }}) }})}
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::PARENOPEN, "("), Token(TokenTypes::SYMBOL, "myFn"), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::PARENCLOSE, ")") },
    { new Sexp({ ExpressionPtr { new Symbol("=") }, ExpressionPtr { new Symbol("x") }, ExpressionPtr { new Sexp({ExpressionPtr { new Symbol("myFn") }, ExpressionPtr { new Int(42) }}) } }) }
  );
}

TEST_F(ParserTest, TestImplicitInfix_Add) {
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "42") },
    { new Sexp({ ExpressionPtr { new Symbol("+") }, ExpressionPtr { new Symbol("x") }, ExpressionPtr { new Int(42) }})}
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::STRING, "foo") },
    { new Sexp({ ExpressionPtr { new Symbol("+") }, ExpressionPtr { new Symbol("x") }, ExpressionPtr { new String("foo") }})}
  );

  ASSERT_PARSE(
    { Token(TokenTypes::NUMBER, "8"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "42") },
    { new Sexp({ ExpressionPtr { new Symbol("+") }, ExpressionPtr { new Int(8) }, ExpressionPtr { new Int(42) }})}
  );
  ASSERT_PARSE(
    { Token(TokenTypes::STRING, "foo"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::STRING, "bar") },
    { new Sexp({ ExpressionPtr { new Symbol("+") }, ExpressionPtr { new String("foo") }, ExpressionPtr { new String("bar") }})}
  );

  ASSERT_PARSE(
    { Token(TokenTypes::NUMBER, "8"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "5") },
    { new Sexp({ ExpressionPtr { new Symbol("+") }, ExpressionPtr { new Int(8) }, ExpressionPtr { new Int(42) }, ExpressionPtr { new Int(5) }})}
  );
  ASSERT_PARSE(
    { Token(TokenTypes::STRING, "foo"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::STRING, "bar"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::STRING, "baz") },
    { new Sexp({ ExpressionPtr { new Symbol("+") }, ExpressionPtr { new String("foo") }, ExpressionPtr { new String("bar") }, ExpressionPtr { new String("baz") }})}
  );

  // 8 + 42 +
  ASSERT_PARSE(
    { Token(TokenTypes::NUMBER, "8"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::SYMBOL, "+") },
    { new Int(8), new Symbol("+"), new Int(42), new Symbol("+") } 
  );
}

TEST_F(ParserTest, TestExplicitInfix_Add) {
  ASSERT_PARSE(
    { Token(TokenTypes::PARENOPEN, "("), Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::PARENCLOSE, ")") },
    { new Sexp({ ExpressionPtr { new Symbol("+") }, ExpressionPtr { new Symbol("x") }, ExpressionPtr { new Int(42) }})}
  );
}

TEST_F(ParserTest, TestImplicitInfix_SetAndAdd) {
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::PARENOPEN, "("), Token(TokenTypes::NUMBER, "42"),
                                                                                                         Token(TokenTypes::SYMBOL, "+"),
                                                                                                         Token(TokenTypes::NUMBER, "31"),
                                                                      Token(TokenTypes::PARENCLOSE, ")") },
    { new Sexp({ ExpressionPtr { new Symbol("=") },
                 ExpressionPtr { new Symbol("x") },
                 ExpressionPtr { new Sexp({ ExpressionPtr { new Symbol("+") },
                                            ExpressionPtr { new Int(42) },
                                            ExpressionPtr { new Int(31) }
                                          })
                                }
              })
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "31") },
    { new Sexp({ ExpressionPtr { new Symbol("=") },
                 ExpressionPtr { new Symbol("x") },
                 ExpressionPtr { new Sexp({ ExpressionPtr { new Symbol("+") },
                                            ExpressionPtr { new Int(42) },
                                            ExpressionPtr { new Int(31) }
                                          })
                                }
              })
    }
  );
}

TEST_F(ParserTest, TestComplexSexps) {
  ExpressionPtr num2 { new Int(2) },
                num3 { new Int(3) },
                symPrint { new Symbol("print") },
                symPlus { new Symbol("+") },
                symFoo { new Symbol("foo") },
                strHello { new String("hello") },
                sexpAdd { new Sexp({ std::move(symPlus), std::move(num2), std::move(num3) }) };
  ASSERT_PARSE(
    {
      Token(TokenTypes::PARENOPEN, ""), Token(TokenTypes::SYMBOL, "print"),
        Token(TokenTypes::PARENOPEN, ""), Token(TokenTypes::SYMBOL, "+"),
          Token(TokenTypes::NUMBER, "2"),
          Token(TokenTypes::NUMBER, "3"),
        Token(TokenTypes::PARENCLOSE, ""),
        Token(TokenTypes::SYMBOL, "foo"),
        Token(TokenTypes::STRING, "hello"),
      Token(TokenTypes::PARENCLOSE, "")
    },
    {
      new Sexp({ 
        std::move(symPrint),
          std::move(sexpAdd),
          std::move(symFoo),
          std::move(strHello),
      })
    }
  );
}
