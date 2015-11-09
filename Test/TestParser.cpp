#include "gtest\gtest.h"
#include <initializer_list>

#include "Parser.h"
#include "CommandInterface.h"
#include "Tokenizer.h"

//TODO: More of a unit test needed? Tokenizer interface? Use a mock CommandInterface?

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

void TestParse(Parser &parser, const std::string &defaultSexp, TestTokenizer &tokenizer, std::initializer_list<Token> &&tokens, bool expectSuccess, std::initializer_list<Expression*> &&expectedArgs) {
  tokenizer.Tokens = tokens;
  ASSERT_TRUE(parser.Parse());
  ASSERT_TRUE(parser.Error().empty());
  auto exprTree = parser.ExpressionTree();
  ASSERT_EQ(2, exprTree->Args.size());
  auto sym = dynamic_cast<Symbol*>(exprTree->Args.front().get());
  ASSERT_TRUE(sym != nullptr);
  ASSERT_EQ(defaultSexp, sym->Value);
  exprTree->Args.pop_front();

  ArgList expected;
  for (auto *expectedArg : expectedArgs)
    expected.push_back(expectedArg->Clone());
  ASSERT_TRUE(ArgListHelper::AreEqual(exprTree->Args, expected));
}

void TestParseFail(Parser &parser, const std::string &defaultSexp, TestTokenizer &tokenizer, std::initializer_list<Token> &&tokens) {
  TestParse(parser, defaultSexp, tokenizer, std::move(tokens), false, {});
}

void TestParseOk(Parser &parser, const std::string &defaultSexp, TestTokenizer &tokenizer, std::initializer_list<Token> &&tokens, std::initializer_list<Expression*> &&expectedArgs) {
  TestParse(parser, defaultSexp, tokenizer, std::move(tokens), true, std::move(expectedArgs));
}

TEST(Parser, Test) {
  std::string defaultSexp = "default";
  TestCommandInterface cmdInterface;
  TestTokenizer tokenizer;
  Parser parser(cmdInterface, tokenizer, defaultSexp);
  
  tokenizer.Tokens = { Token(TokenTypes::NONE, "") };
  ASSERT_TRUE(parser.Parse());
  ASSERT_TRUE(parser.Error().empty());
  auto exprTree = parser.ExpressionTree();
  ASSERT_EQ(1, exprTree->Args.size());
  Symbol *sym = dynamic_cast<Symbol*>(exprTree->Args.front().get());
  ASSERT_TRUE(sym != nullptr);
  ASSERT_EQ(defaultSexp, sym->Value);

  ASSERT_NO_FATAL_FAILURE(TestParseOk(parser, defaultSexp, tokenizer,
    { Token(TokenTypes::PARENOPEN, ""), Token(TokenTypes::SYMBOL, "foo"), Token(TokenTypes::PARENCLOSE, "") },
    { new Sexp({ ExpressionPtr { new Symbol("foo") } }) }
  ));
}