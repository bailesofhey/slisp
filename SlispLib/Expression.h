#pragma once

#include <list>
#include <string>
#include <cinttypes>
#include <memory>
#include <map>

struct TypeInfo {
  const std::string TypeName;

  TypeInfo(const std::string &typeName);
  TypeInfo() = delete;
  TypeInfo(const TypeInfo&) = delete;
  TypeInfo(TypeInfo&&) = delete;
  TypeInfo& operator=(TypeInfo) = delete;
};

struct Expression;

using ExpressionPtr = std::unique_ptr<Expression>;
using SymbolTableType = std::map<std::string, ExpressionPtr>;

struct Expression {
  const TypeInfo& _Type;

  Expression(const TypeInfo& typeInfo);
  virtual ~Expression();
  virtual ExpressionPtr Clone() const = 0;
  virtual const std::string ToString() const = 0;
  const TypeInfo& Type() const;
};

struct Void: public Expression {
  static const TypeInfo TypeInstance;
};

struct Literal: public Expression {
  static const TypeInfo TypeInstance;

  Literal(const TypeInfo& typeInfo);
};

struct Bool: public Literal {
  static const TypeInfo TypeInstance;

  bool Value;

  Bool();
  Bool(bool value);
  virtual ExpressionPtr Clone() const;
  virtual const std::string ToString() const;
  bool operator==(const Bool &rhs) const;
  bool operator<(const Bool &rhs) const;
  Bool& operator=(Bool rhs);
  void Swap(Bool &rhs);
};

struct Number: public Literal {
  static const TypeInfo TypeInstance; 
  
  int64_t Value;

  Number();
  Number(int64_t value);
  virtual ExpressionPtr Clone() const;
  virtual const std::string ToString() const;
  bool operator==(const Number &rhs) const;
  bool operator<(const Number &rhs) const;
  Number& operator=(Number rhs);
  void Swap(Number &rhs);
};

struct String: public Literal {
  static const TypeInfo TypeInstance;
  
  std::string Value;

  String();
  String(const std::string& value);
  virtual ExpressionPtr Clone() const;
  virtual const std::string ToString() const;
  bool operator==(const String &rhs) const;
  bool operator<(const String &rhs) const;
  String& operator=(String rhs);
  void Swap(String &rhs);
};

struct Quote: public Literal {
  static const TypeInfo TypeInstance;

  ExpressionPtr Value;

  Quote(ExpressionPtr &&expr);
  Quote& operator=(Quote rhs);
  void Swap(Quote &rhs);
  virtual ExpressionPtr Clone() const;
  virtual const std::string ToString() const;
};

struct Symbol: public Expression {
  static const TypeInfo TypeInstance;

  std::string Value;

  Symbol(const std::string& value);
  virtual ExpressionPtr Clone() const;
  virtual const std::string ToString() const;
  bool operator==(const Symbol& rhs) const;
  bool operator<(const Symbol &rhs) const;
  Symbol& operator=(Symbol rhs);
  void Swap(Symbol &rhs);
};

using ArgList = std::list<ExpressionPtr>;

struct Sexp: public Expression {
  static const TypeInfo TypeInstance;

  ArgList Args;

  Sexp();
  virtual ExpressionPtr Clone() const;
  virtual const std::string ToString() const;
};
