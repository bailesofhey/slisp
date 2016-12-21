#include <initializer_list>
#include <limits>
#include "gtest/gtest.h"

#include "Parser.h"
#include "CommandInterface.h"
#include "Tokenizer.h"
#include "InterpreterUtils.h"

#include "Common.h"
#include "BaseTest.h"

using namespace std;

#define NSC NullSourceContext

class ParserTest: public BaseTest {
  public:
    ParserTest():
      DummySymbolStore(),
      DummySymbols(DummySymbolStore, NSC),
      Settings(DummySymbols),
      CommandInterface(),
      Tokenizer(),
      _Parser(CommandInterface, Tokenizer, Settings)
    {
      DummySymbols.PutSymbolFunction(
        "+", 
        {"(+) -> nil"},
        "no help",
        {},
        &DummyFn, 
        FuncDef { FuncDef::NoArgs(), FuncDef::NoArgs() }
      );
      Settings.RegisterInfixSymbol("+");
      Settings.RegisterInfixSymbol("=");
    }

    static bool DummyFn(EvaluationContext &ctx) {
      return false;
    }
  
  protected:
    SymbolTableType DummySymbolStore;
    SymbolTable DummySymbols;
    InterpreterSettings Settings;
    TestCommandInterface CommandInterface;
    TestTokenizer Tokenizer;
    Parser _Parser;

    void TestParse(initializer_list<Token> &&tokens, bool expectSuccess, initializer_list<Expression*> &&expectedArgs) {
      Tokenizer.Tokens = tokens;
      ASSERT_EQ(expectSuccess, _Parser.Parse());
      ASSERT_EQ(expectSuccess, _Parser.Error().empty());
      auto exprTree = _Parser.ExpressionTree();
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

    void TestParseFail(initializer_list<Token> &&tokens) {
      TestParse(move(tokens), false, {});
    }

    void TestParseOk(initializer_list<Token> &&tokens, initializer_list<Expression*> &&expectedArgs) {
      TestParse(move(tokens), true, move(expectedArgs));
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
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x0") },      { Factory.Alloc<Int>(0) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x1") },      { Factory.Alloc<Int>(1) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x2") },      { Factory.Alloc<Int>(2) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x3") },      { Factory.Alloc<Int>(3) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x4") },      { Factory.Alloc<Int>(4) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x5") },      { Factory.Alloc<Int>(5) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x6") },      { Factory.Alloc<Int>(6) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x7") },      { Factory.Alloc<Int>(7) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x8") },      { Factory.Alloc<Int>(8) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x9") },      { Factory.Alloc<Int>(9) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0xa") },      { Factory.Alloc<Int>(10) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0xb") },      { Factory.Alloc<Int>(11) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0xc") },      { Factory.Alloc<Int>(12) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0xd") },      { Factory.Alloc<Int>(13) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0xe") },      { Factory.Alloc<Int>(14) }); //#104
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0xf") },      { Factory.Alloc<Int>(15) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x10") },     { Factory.Alloc<Int>(16) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x07f") },    { Factory.Alloc<Int>(127) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x000080") }, { Factory.Alloc<Int>(128) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0xfe") },     { Factory.Alloc<Int>(254) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0xff") },     { Factory.Alloc<Int>(255) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0x103") },    { Factory.Alloc<Int>(259) });
}

TEST_F(ParserTest, TestSingleBinaryInt) {
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0b0") },    { Factory.Alloc<Int>(0) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0b1") },    { Factory.Alloc<Int>(1) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0b10") },   { Factory.Alloc<Int>(2) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0b1101") }, { Factory.Alloc<Int>(13) });
}

TEST_F(ParserTest, TestSingleInt) {
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "42") },           { Factory.Alloc<Int>(42) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "042") },          { Factory.Alloc<Int>(42) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "000000000042") }, { Factory.Alloc<Int>(42) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "00000000000000000000000000000000000000000042") }, { Factory.Alloc<Int>(42) });

  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "-3") },           { Factory.Alloc<Int>(-3) });

  ASSERT_NOPARSE({ Token(TokenTypes::NUMBER, "") });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0") },            { Factory.Alloc<Int>(0) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "000") },          { Factory.Alloc<Int>(0) });

  auto minValue = numeric_limits<decltype(Int::Value)>::min(),
       maxValue = numeric_limits<decltype(Int::Value)>::max();
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, to_string(minValue)) }, { Factory.Alloc<Int>(minValue) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, to_string(maxValue)) }, { Factory.Alloc<Int>(maxValue) });
}

// Automatically promote to float if overflow?
TEST_F(ParserTest, TestSingleFloat) {
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "0.0") },           { Factory.Alloc<Float>(0.0) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "3.14") },          { Factory.Alloc<Float>(3.14) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "3.14159265359") }, { Factory.Alloc<Float>(3.14159265359) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "-3.14") },         { Factory.Alloc<Float>(-3.14) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "3.14e1") },        { Factory.Alloc<Float>(3.14e1) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "3.14e23") },       { Factory.Alloc<Float>(3.14e23) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "-3.14e23") },      { Factory.Alloc<Float>(-3.14e23) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "-3.14e-23") },     { Factory.Alloc<Float>(-3.14e-23) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "3e2") },           { Factory.Alloc<Float>(3e2) });
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, "3e-2") },          { Factory.Alloc<Float>(3e-2) });
  
  auto minValue = numeric_limits<decltype(Float::Value)>::min(),
       maxValue = numeric_limits<decltype(Float::Value)>::max();
  stringstream ss;
  ss << setprecision(17)
     << minValue;
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, ss.str()) }, { Factory.Alloc<Float>(minValue) });
  ss.clear();
  ss.str("");
  ss << maxValue;
  ASSERT_PARSE({ Token(TokenTypes::NUMBER, ss.str()) }, { Factory.Alloc<Float>(maxValue) });
}

TEST_F(ParserTest, TestSingleSymbol) {
  ASSERT_PARSE({ Token(TokenTypes::SYMBOL, "a") }, { Factory.Alloc<Symbol>("a") });
  ASSERT_PARSE({ Token(TokenTypes::SYMBOL, "1") }, { Factory.Alloc<Symbol>("1") });
  ASSERT_PARSE({ Token(TokenTypes::SYMBOL, "+") }, { Factory.Alloc<Symbol>("+") });
  ASSERT_NOPARSE({ Token(TokenTypes::SYMBOL, "") });

  string longSym = "abcdefghijklmnopqrstuvwxyz0123456789";
  ASSERT_PARSE({ Token(TokenTypes::SYMBOL, longSym) }, { Factory.Alloc<Symbol>(longSym) });
}

TEST_F(ParserTest, TestSingleString) {
  ASSERT_PARSE({ Token(TokenTypes::STRING, "") },            { Factory.Alloc<Str>() });
  ASSERT_PARSE({ Token(TokenTypes::STRING, "") },            { Factory.Alloc<Str>("") });
  ASSERT_PARSE({ Token(TokenTypes::STRING, " ") },           { Factory.Alloc<Str>(" ") });
  ASSERT_PARSE({ Token(TokenTypes::STRING, "        ") },    { Factory.Alloc<Str>("        ") });

  ASSERT_PARSE({ Token(TokenTypes::STRING, "foo") },         { Factory.Alloc<Str>("foo") });
  ASSERT_PARSE({ Token(TokenTypes::STRING, "foo bar baz") }, { Factory.Alloc<Str>("foo bar baz") });

  // Escaping?
  // Unicode?

  ASSERT_PARSE({ Token(TokenTypes::STRING, "123") },         { Factory.Alloc<Str>("123") });
}

TEST_F(ParserTest, TestQuote) {
  ASSERT_NOPARSE({ Token(TokenTypes::QUOTE, "'") });
  ASSERT_PARSE({ Token(TokenTypes::QUOTE, "'"), Token(TokenTypes::NUMBER, "42") },   { Factory.Alloc<Quote>(ExpressionPtr {Factory.Alloc<Int>(42)}) });
  ASSERT_PARSE({ Token(TokenTypes::QUOTE, "'"), Token(TokenTypes::NUMBER, "3.14") }, { Factory.Alloc<Quote>(ExpressionPtr {Factory.Alloc<Float>(3.14)}) });
  ASSERT_PARSE({ Token(TokenTypes::QUOTE, "'"), Token(TokenTypes::STRING, "qux") },  { Factory.Alloc<Quote>(ExpressionPtr {Factory.Alloc<Str>("qux")}) });
  ASSERT_PARSE({ Token(TokenTypes::QUOTE, "'"), Token(TokenTypes::SYMBOL, "foo") },  { Factory.Alloc<Quote>(ExpressionPtr {Factory.Alloc<Symbol>("foo")}) });
  ASSERT_PARSE(
  { 
    Token(TokenTypes::QUOTE, "'"), 
    Token(TokenTypes::PARENOPEN, "("), 
      Token(TokenTypes::SYMBOL, "+"), 
      Token(TokenTypes::NUMBER, "1"), 
      Token(TokenTypes::NUMBER, "2"), 
    Token(TokenTypes::PARENCLOSE, ")"), 
  }, 
  {
    Factory.Alloc<Quote>(ExpressionPtr {
      new Sexp(NSC, {
        ExpressionPtr { Factory.Alloc<Symbol>("+") }, 
        ExpressionPtr { Factory.Alloc<Int>(1) }, 
        ExpressionPtr { Factory.Alloc<Int>(2) }
      })
    })
  });
  ASSERT_NOPARSE({ Token(TokenTypes::QUOTE, "'"), Token(TokenTypes::PARENCLOSE, ")") });
}

TEST_F(ParserTest, TestMultiples) {
  ASSERT_PARSE(
    { Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::NUMBER, "43")},
    { Factory.Alloc<Int>(42), Factory.Alloc<Int>(43) }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::NUMBER, "3.14"), Token(TokenTypes::NUMBER, "1.5")},
    { Factory.Alloc<Float>(3.14), Factory.Alloc<Float>(1.5) }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "foo"), Token(TokenTypes::SYMBOL, "bar") },
    { Factory.Alloc<Symbol>("foo"), Factory.Alloc<Symbol>("bar") }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::STRING, "hello"), Token(TokenTypes::STRING, "world!") },
    { Factory.Alloc<Str>("hello"), Factory.Alloc<Str>("world!") }
  );

  ASSERT_PARSE(
    { 
      Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::NUMBER, "3.14"), Token(TokenTypes::SYMBOL, "foo"), Token(TokenTypes::STRING, "hello"),
      Token(TokenTypes::NUMBER, "43"), Token(TokenTypes::NUMBER, "1.5"), Token(TokenTypes::SYMBOL, "bar"), Token(TokenTypes::STRING, "world!")
    },
    {
      Factory.Alloc<Int>(42), Factory.Alloc<Float>(3.14), Factory.Alloc<Symbol>("foo"), Factory.Alloc<Str>("hello"),
      Factory.Alloc<Int>(43), Factory.Alloc<Float>(1.5),  Factory.Alloc<Symbol>("bar"), Factory.Alloc<Str>("world!") 
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
    { new Sexp(NSC, {ExpressionPtr { }} ) }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::PARENOPEN, ""), Token(TokenTypes::SYMBOL, "foo"), Token(TokenTypes::PARENCLOSE, "") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("foo") }}) }
  );
}

TEST_F(ParserTest, TestNoInfix) {
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "*"), Token(TokenTypes::NUMBER, "42") },
    { Factory.Alloc<Symbol>("x"), Factory.Alloc<Symbol>("*"), Factory.Alloc<Int>(42) }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::PARENOPEN, "("), Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "*"), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::PARENCLOSE, ")") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("x") },
                     ExpressionPtr { Factory.Alloc<Symbol>("*") }, 
                     ExpressionPtr { Factory.Alloc<Int>(42) }})
    }
  );
}

TEST_F(ParserTest, TestImplicitInfix_Set) {
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::NUMBER, "42") },
    { new Sexp(NSC,{ExpressionPtr { Factory.Alloc<Symbol>("=") }, 
                    ExpressionPtr { Factory.Alloc<Symbol>("x") }, 
                    ExpressionPtr { Factory.Alloc<Int>(42) }})
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::NUMBER, "3.14") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("=") }, 
                     ExpressionPtr { Factory.Alloc<Symbol>("x") }, 
                     ExpressionPtr { Factory.Alloc<Float>(3.14) }})
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::STRING, "foo") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("=") }, 
                     ExpressionPtr { Factory.Alloc<Symbol>("x") }, 
                     ExpressionPtr { Factory.Alloc<Str>("foo") }})
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::SYMBOL, "n") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("=") }, 
                     ExpressionPtr { Factory.Alloc<Symbol>("x") }, 
                     ExpressionPtr { Factory.Alloc<Symbol>("n") }})
    }
  );

  // Functions

  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::PARENOPEN, "("), Token(TokenTypes::SYMBOL, "myFn"), Token(TokenTypes::PARENCLOSE, ")") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("=") }, 
                     ExpressionPtr { Factory.Alloc<Symbol>("x") }, 
                     ExpressionPtr { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("myFn") }}) }})
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::PARENOPEN, "("), Token(TokenTypes::SYMBOL, "myFn"), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::PARENCLOSE, ")") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("=") }, 
                          ExpressionPtr { Factory.Alloc<Symbol>("x") }, 
                          ExpressionPtr { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("myFn") }, 
                                                         ExpressionPtr { Factory.Alloc<Int>(42) }}) }} )
    }
  );
}

TEST_F(ParserTest, TestImplicitInfix_Add) {
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "42") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") }, 
                     ExpressionPtr { Factory.Alloc<Symbol>("x") }, 
                     ExpressionPtr { Factory.Alloc<Int>(42) }})
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "3.14") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") }, 
                     ExpressionPtr { Factory.Alloc<Symbol>("x") }, 
                     ExpressionPtr { Factory.Alloc<Float>(3.14) }})
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::STRING, "foo") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") }, 
                     ExpressionPtr { Factory.Alloc<Symbol>("x") }, 
                     ExpressionPtr { Factory.Alloc<Str>("foo") }})
    }
  );

  ASSERT_PARSE(
    { Token(TokenTypes::NUMBER, "8"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "42") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") }, 
                     ExpressionPtr { Factory.Alloc<Int>(8) }, 
                     ExpressionPtr { Factory.Alloc<Int>(42) }})
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::NUMBER, "3.14"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "1.5") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") }, 
                     ExpressionPtr { Factory.Alloc<Float>(3.14) }, 
                     ExpressionPtr { Factory.Alloc<Float>(1.5) }})
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::STRING, "foo"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::STRING, "bar") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") }, 
                     ExpressionPtr { Factory.Alloc<Str>("foo") }, 
                     ExpressionPtr { Factory.Alloc<Str>("bar") }})
    }
  );

  ASSERT_PARSE(
    { Token(TokenTypes::NUMBER, "8"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "5") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") }, 
                     ExpressionPtr { Factory.Alloc<Int>(8) }, 
                     ExpressionPtr { Factory.Alloc<Int>(42) }, 
                     ExpressionPtr { Factory.Alloc<Int>(5) }})
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::NUMBER, "3.14"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "1.5"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "2.345") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") }, 
                     ExpressionPtr { Factory.Alloc<Float>(3.14) }, 
                     ExpressionPtr { Factory.Alloc<Float>(1.5) }, 
                     ExpressionPtr { Factory.Alloc<Float>(2.345) }})
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::STRING, "foo"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::STRING, "bar"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::STRING, "baz") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") }, 
                     ExpressionPtr { Factory.Alloc<Str>("foo") }, 
                     ExpressionPtr { Factory.Alloc<Str>("bar") }, 
                     ExpressionPtr { Factory.Alloc<Str>("baz") }})
    }
  );

  // 8 + 42 +
  ASSERT_PARSE(
    { Token(TokenTypes::NUMBER, "8"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::SYMBOL, "+") },
    { Factory.Alloc<Int>(8), Factory.Alloc<Symbol>("+"), Factory.Alloc<Int>(42), Factory.Alloc<Symbol>("+") } 
  );
}

TEST_F(ParserTest, TestExplicitInfix_Add) {
  ASSERT_PARSE(
    { Token(TokenTypes::PARENOPEN, "("), Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::PARENCLOSE, ")") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") }, 
                     ExpressionPtr { Factory.Alloc<Symbol>("x") }, 
                     ExpressionPtr { Factory.Alloc<Int>(42) }})
    }
  );
}

TEST_F(ParserTest, TestImplicitInfix_SetAndAdd) {
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::PARENOPEN, "("), Token(TokenTypes::NUMBER, "42"),
                                                                                                         Token(TokenTypes::SYMBOL, "+"),
                                                                                                         Token(TokenTypes::NUMBER, "31"),
                                                                      Token(TokenTypes::PARENCLOSE, ")") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("=") },
                     ExpressionPtr { Factory.Alloc<Symbol>("x") },
                     ExpressionPtr { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") },
                                                    ExpressionPtr { Factory.Alloc<Int>(42) },
                                                    ExpressionPtr { Factory.Alloc<Int>(31) }}) } })
    }
  );
  ASSERT_PARSE(
    { Token(TokenTypes::SYMBOL, "x"), Token(TokenTypes::SYMBOL, "="), Token(TokenTypes::NUMBER, "42"), Token(TokenTypes::SYMBOL, "+"), Token(TokenTypes::NUMBER, "31") },
    { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("=") },
                     ExpressionPtr { Factory.Alloc<Symbol>("x") },
                     ExpressionPtr { new Sexp(NSC, {ExpressionPtr { Factory.Alloc<Symbol>("+") },
                                                ExpressionPtr { Factory.Alloc<Int>(42) },
                                                ExpressionPtr { Factory.Alloc<Int>(31) }})}})
    }
  );
}

TEST_F(ParserTest, TestComplexSexps) {
  ExpressionPtr num2 {     Factory.Alloc<Int>(2) },
                num3 {     Factory.Alloc<Int>(3) },
                symPrint { Factory.Alloc<Symbol>("print") },
                symPlus {  Factory.Alloc<Symbol>("+") },
                symFoo {   Factory.Alloc<Symbol>("foo") },
                strHello { Factory.Alloc<Str>("hello") },
                sexpAdd {  new Sexp(NSC, {move(symPlus), move(num2), move(num3)} ) };
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
      new Sexp(NSC, {
        move(symPrint),
          move(sexpAdd),
          move(symFoo),
          move(strHello)
        }
      )
    }
  );
}
