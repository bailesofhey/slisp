#include <iostream>
#include <memory>

#include "Tokenizer.h"
#include "Parser.h"

using std::cout;
using std::endl;

//=============================================================================

Parser::Parser(CommandInterface &commandInterface, ITokenizer &tokenizer, const std::string &defaultSexp, bool debug):
  CommandInterface_ { commandInterface },
  Tokenizer_ { tokenizer },
  DefaultSexp { defaultSexp },
  Debug { debug }
{
}

bool Parser::Parse() {
  Reset();

  std::string line;
  if (CommandInterface_.ReadInputLine(line)) {
    Tokenizer_.SetLine(line);
    
    ++Tokenizer_;
    bool parseResult = true;
    while (parseResult && (*Tokenizer_).Type != TokenTypes::NONE) {
      parseResult = ParseToken(*ExprTree);
      ++Tokenizer_;
    }
    return parseResult;
  }

  return false;
}

const std::string& Parser::Error() const {
  return Error_;
}

std::unique_ptr<Sexp> Parser::ExpressionTree() const {
  return std::unique_ptr<Sexp>(static_cast<Sexp*>(ExprTree->Clone().release()));
}

void Parser::Reset() {
  ExprTree = std::unique_ptr<Sexp>(new Sexp);
  ExprTree->Args.push_back(ExpressionPtr { new Symbol { DefaultSexp } });
  Error_ = "";
  Depth = 0;
}

bool Parser::ParseToken(Sexp &root) {
  if (Debug)
    cout << static_cast<std::string>(*Tokenizer_) << endl;

  auto &tokenType = (*Tokenizer_).Type;
  if (tokenType == TokenTypes::NUMBER)
    return ParseNumber(root);
  else if (tokenType == TokenTypes::STRING)
    return ParseString(root);
  else if (tokenType == TokenTypes::SYMBOL)
    return ParseSymbol(root);
  else if (tokenType == TokenTypes::PARENOPEN)
    return ParseParenOpen(root);
  else if (tokenType == TokenTypes::PARENCLOSE)
    return ParseParenClose(root);
  else if (tokenType == TokenTypes::UNKNOWN)
    return ParseUnknown(root);
  else if (tokenType == TokenTypes::NONE)
    return ParseNone(root);
  else {
    Error_ = "Unexpected token: " + static_cast<std::string>(*Tokenizer_);
    return false;
  }
}

bool Parser::ParseNumber(Sexp &root) {
  auto &val = (*Tokenizer_).Value;
  if (!val.empty()) {
    root.Args.push_back(ExpressionPtr { new Number { std::atoll(val.c_str()) } });
    return true;
  }
  else {
    Error_ = "Number has no value";
    return false;
  }
}

bool Parser::ParseSymbol(Sexp &root) {
  auto &val = (*Tokenizer_).Value;
  if (!val.empty()) {
    root.Args.push_back(ExpressionPtr { new Symbol { val } });
    return true;
  }
  else {
    Error_ = "Symbol has no value";
    return false;
  }
}

bool Parser::ParseString(Sexp &root) {
  root.Args.push_back(ExpressionPtr { new String { (*Tokenizer_).Value } });
  return true;
}

bool Parser::ParseParenOpen(Sexp &root) {
  ++Depth;
  ++Tokenizer_;
  auto currSexpExpr = ExpressionPtr { new Sexp };
  auto currSexp = static_cast<Sexp*>(currSexpExpr.get());
  return ParseSexpArgs(root, *currSexp);
}

bool Parser::ParseParenClose(Sexp &root) {
  --Depth;
  return true;
}

bool Parser::ParseSexpArgs(Sexp &root, Sexp &curr) {
begin:
  bool parseResult = true;
  while ((parseResult = ParseToken(curr)) &&
         (*Tokenizer_).Type != TokenTypes::NONE &&
         (*Tokenizer_).Type != TokenTypes::PARENCLOSE) {
    ++Tokenizer_;
  }

  if (!parseResult)
    return false;
  else if ((*Tokenizer_).Type == TokenTypes::PARENCLOSE) {
    root.Args.push_back(ExpressionPtr { curr.Clone() });
    (*Tokenizer_).Type = TokenTypes::UNKNOWN;
    return true;
  }
  else  {
    if (Depth) {
      std::string line;
      if (CommandInterface_.HasMore()) {
        CommandInterface_.ReadContinuedInputLine(line);
        Tokenizer_.SetLine(line);
        ++Tokenizer_;
        goto begin;
      }
      else {
        Error_ = "Unterminated Sexp";
        return false;
      }
    }
    else
      throw std::exception("Logic bug: NONE should only be reached with Depth > 0");
  }
}

bool Parser::ParseUnknown(Sexp &root) {
  Error_ = "Unknown token: " + (*Tokenizer_).Value;
  return false;
}

bool Parser::ParseNone(Sexp &root) {
  return true;
}