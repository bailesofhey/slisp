#pragma once

#include <list>
#include <string>
#include <cinttypes>
#include <memory>
#include <map>

//TODO: need global type list

struct TypeInfo {
  const std::string TypeName;

  explicit TypeInfo(const std::string &typeName);
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

  explicit Expression(const TypeInfo& typeInfo);
  virtual ~Expression();
  virtual ExpressionPtr Clone() const = 0;
  virtual void Print(std::ostream& out) const = 0;
  const std::string ToString() const;
  virtual bool operator==(const Expression &rhs) const = 0;
  bool operator!=(const Expression &rhs) const;
  const TypeInfo& Type() const;

  friend std::ostream& operator<<(std::ostream &out, const Expression &expr);
  static bool AreEqual(const ExpressionPtr &lhs, const ExpressionPtr &rhs);
};

struct Void: public Expression {
  static const TypeInfo TypeInstance;
};

struct Literal: public Expression {
  static const TypeInfo TypeInstance;

  explicit Literal(const TypeInfo& typeInfo);
};

struct Bool: public Literal {
  static const TypeInfo TypeInstance;

  bool Value;

  explicit Bool();
  explicit Bool(bool value);
  virtual ExpressionPtr Clone() const override;
  virtual void Print(std::ostream& out) const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Bool &rhs) const;
  bool operator!=(const Bool &rhs) const;
  bool operator<(const Bool &rhs) const;
  bool operator>=(const Bool &rhs) const;
  Bool& operator=(Bool rhs);
  void Swap(Bool &rhs);
};

struct Int: public Literal {
  static const TypeInfo TypeInstance; 
  
  int64_t Value;

  explicit Int();
  explicit Int(int64_t value);
  virtual ExpressionPtr Clone() const override;
  virtual void Print(std::ostream& out) const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Int &rhs) const;
  bool operator!=(const Int &rhs) const;
  bool operator<(const Int &rhs) const;
  bool operator>=(const Int &rhs) const;
  Int& operator=(Int rhs);
  void Swap(Int &rhs);
};

struct Float: public Literal {
  static const TypeInfo TypeInstance;

  double Value; 

  explicit Float();
  explicit Float(double value);
  virtual ExpressionPtr Clone() const override;
  virtual void Print(std::ostream& out) const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Float &rhs) const;
  bool operator!=(const Float &rhs) const;
  bool operator<(const Float &rhs) const;
  bool operator>=(const Float &rhs) const;
  Float& operator=(Float rhs);
  void Swap(Float &rhs);
};

struct Str: public Literal {
  static const TypeInfo TypeInstance;
  
  std::string Value;

  explicit Str();
  explicit Str(const std::string& value);
  virtual ExpressionPtr Clone() const override;
  virtual void Print(std::ostream& out) const;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Str &rhs) const;
  bool operator!=(const Str &rhs) const;
  bool operator<(const Str &rhs) const;
  bool operator>=(const Str &rhs) const;
  Str& operator=(Str rhs);
  void Swap(Str &rhs);
};

struct Quote: public Literal {
  static const TypeInfo TypeInstance;

  ExpressionPtr Value;

  explicit Quote(ExpressionPtr &&expr);
  virtual ExpressionPtr Clone() const override;
  virtual void Print(std::ostream& out) const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Quote &rhs) const;
  bool operator!=(const Quote &rhs) const;
};

struct Symbol: public Expression {
  static const TypeInfo TypeInstance;

  std::string Value;

  explicit Symbol(const std::string& value);
  virtual ExpressionPtr Clone() const override;
  virtual void Print(std::ostream& out) const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Symbol& rhs) const;
  bool operator!=(const Symbol& rhs) const;
  bool operator<(const Symbol &rhs) const;
  bool operator>=(const Symbol &rhs) const;
  Symbol& operator=(Symbol rhs);
  void Swap(Symbol &rhs);
};

using ArgList = std::list<ExpressionPtr>;

class ArgListHelper {
  public:
    static bool AreEqual(const ArgList &lhs, const ArgList &rhs);
    static void CopyTo(const ArgList &src, ArgList &dst);
};

struct Sexp: public Expression {
  static const TypeInfo TypeInstance;

  ArgList Args;

  explicit Sexp();
  explicit Sexp(ArgList &&args);
  explicit Sexp(std::initializer_list<ExpressionPtr> &&args);
  virtual void Print(std::ostream& out) const override;
  virtual ExpressionPtr Clone() const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Sexp &rhs) const;
  bool operator!=(const Sexp &rhs) const;
};
