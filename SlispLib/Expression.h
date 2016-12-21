#pragma once

#include <list>
#include <string>
#include <cinttypes>
#include <memory>
#include <map>

//TODO: need global type list

struct ModuleInfo {
  std::string Name;
  std::string FilePath;
};

struct SourceContext {
  ModuleInfo* Module;
  size_t LineNum;

  explicit SourceContext();
  explicit SourceContext(ModuleInfo* module, size_t lineNum);
  explicit SourceContext(const SourceContext &rhs);
};

static const SourceContext NullSourceContext;

struct Expression;

using ExpressionPtr = std::unique_ptr<Expression>;
using SymbolTableType = std::map<std::string, ExpressionPtr>;
typedef ExpressionPtr (*ExpressionNewFn)(const SourceContext &);

static ExpressionPtr NullExprPtr {};

class TypeInfo {
public:
  explicit TypeInfo(const std::string &typeName, const ExpressionNewFn newFn);
  TypeInfo() = delete;
  TypeInfo(const TypeInfo&) = delete;
  TypeInfo(TypeInfo&&) = delete;
  TypeInfo& operator=(TypeInfo) = delete;
  const std::string& Name() const;
  ExpressionPtr New(const SourceContext &sourceContext) const;
  static ExpressionPtr NewUndefined(const SourceContext &sourceContext);
private:
  const std::string TypeName;
  const ExpressionNewFn NewFn;
};

struct Expression {
  const TypeInfo& Type_;

  explicit Expression(const SourceContext &sourceContext, const TypeInfo& typeInfo);
  virtual ~Expression();
  virtual ExpressionPtr Clone() const = 0;
  virtual ExpressionPtr New(const SourceContext &sourceContext) const;
  virtual void Display(std::ostream& out) const = 0;
  virtual void Print(std::ostream& out) const;
  const std::string ToString() const;
  virtual bool operator==(const Expression &rhs) const = 0;
  bool operator!=(const Expression &rhs) const;
  const TypeInfo& Type() const;
  void SetSourceContext(const SourceContext &newSourceContext);
  void SetSourceContext(const Expression &fromExpr);
  const SourceContext& GetSourceContext() const;

  friend std::ostream& operator<<(std::ostream &out, const Expression &expr);
  static bool AreEqual(const ExpressionPtr &lhs, const ExpressionPtr &rhs);
private:
  SourceContext SourceContext_;
};

class IIterator {
public:
  static const int64_t LENGTH_UNKNOWN = -1;
  virtual ~IIterator();
  virtual ExpressionPtr& Next() = 0;
  virtual int64_t GetLength() = 0;
protected:
  ExpressionPtr Null;
};
 using IteratorPtr = std::unique_ptr<IIterator>;

class IIterable {
public:
  virtual IteratorPtr GetIterator() = 0;
};

struct Void: public Expression {
  static const TypeInfo TypeInstance;
};

struct Literal: public Expression {
  static const TypeInfo TypeInstance;

  explicit Literal(const SourceContext &sourceContext, const TypeInfo& typeInfo);
};

struct Bool: public Literal {
  static const TypeInfo TypeInstance;
  static const Bool Null;

  bool Value;

  explicit Bool(const SourceContext &sourceContext);
  explicit Bool(const SourceContext &sourceContext, bool value);
  virtual ExpressionPtr Clone() const override;
  virtual void Display(std::ostream& out) const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Bool &rhs) const;
  bool operator!=(const Bool &rhs) const;
  bool operator<(const Bool &rhs) const;
  bool operator>=(const Bool &rhs) const;
  Bool& operator=(Bool rhs);
  void Swap(Bool &rhs);
  static ExpressionPtr NewInstance(const SourceContext &sourceContext);
};

struct Int: public Literal {
  static const TypeInfo TypeInstance; 
  static const Int Null;
  
  int64_t Value;

  explicit Int(const SourceContext &sourceContext);
  explicit Int(const SourceContext &sourceContext, int64_t value);
  virtual ExpressionPtr Clone() const override;
  virtual void Display(std::ostream& out) const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Int &rhs) const;
  bool operator!=(const Int &rhs) const;
  bool operator<(const Int &rhs) const;
  bool operator>=(const Int &rhs) const;
  Int& operator=(Int rhs);
  void Swap(Int &rhs);
  static ExpressionPtr NewInstance(const SourceContext &sourceContext);
};

struct Float: public Literal {
  static const TypeInfo TypeInstance;
  static const Float Null;

  double Value; 

  explicit Float(const SourceContext &sourceContext);
  explicit Float(const SourceContext &sourceContext, double value);
  virtual ExpressionPtr Clone() const override;
  virtual void Display(std::ostream& out) const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Float &rhs) const;
  bool operator!=(const Float &rhs) const;
  bool operator<(const Float &rhs) const;
  bool operator>=(const Float &rhs) const;
  Float& operator=(Float rhs);
  void Swap(Float &rhs);
  static ExpressionPtr NewInstance(const SourceContext &sourceContext);
};

struct Str: public Literal, IIterable {
  static const TypeInfo TypeInstance;
  static const Str Null;
  
  std::string Value;

  explicit Str(const SourceContext &sourceContext);
  explicit Str(const SourceContext &sourceContext, const std::string& value);
  virtual ExpressionPtr Clone() const override;
  virtual void Display(std::ostream& out) const override;
  virtual void Print(std::ostream& out) const override;
  virtual IteratorPtr GetIterator();
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Str &rhs) const;
  bool operator!=(const Str &rhs) const;
  bool operator<(const Str &rhs) const;
  bool operator>=(const Str &rhs) const;
  Str& operator=(Str rhs);
  void Swap(Str &rhs);
  static ExpressionPtr NewInstance(const SourceContext &sourceContext);
};

class StrIterator: public IIterator {
public:
  explicit StrIterator(Str &str);
  virtual ExpressionPtr& Next() override;
  virtual int64_t GetLength() override;
private:
  ExpressionPtr Curr;
  Str &Value;
  size_t Index;
};

struct Quote: public Literal, IIterable {
  static const TypeInfo TypeInstance;
  static const Quote Null;

  ExpressionPtr Value;

  explicit Quote(const SourceContext &sourceContext, ExpressionPtr &&expr);
  virtual ExpressionPtr Clone() const override;
  virtual void Display(std::ostream& out) const override;
  virtual void Print(std::ostream& out) const override;
  virtual IteratorPtr GetIterator();
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Quote &rhs) const;
  bool operator!=(const Quote &rhs) const;
  static ExpressionPtr NewInstance(const SourceContext &sourceContext);
};

struct Symbol: public Expression {
  static const TypeInfo TypeInstance;
  static const Symbol Null;

  std::string Value;

  explicit Symbol(const SourceContext &sourceContext, const std::string& value);
  virtual ExpressionPtr Clone() const override;
  virtual void Display(std::ostream& out) const override;
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Symbol& rhs) const;
  bool operator!=(const Symbol& rhs) const;
  bool operator<(const Symbol &rhs) const;
  bool operator>=(const Symbol &rhs) const;
  Symbol& operator=(Symbol rhs);
  void Swap(Symbol &rhs);
  static ExpressionPtr NewInstance(const SourceContext &sourceContext);
};

using ArgList = std::list<ExpressionPtr>;

class ArgListHelper {
  public:
    static bool AreEqual(const ArgList &lhs, const ArgList &rhs);
    static void CopyTo(const ArgList &src, ArgList &dst);
};

struct Sexp: public Expression, IIterable {
  static const TypeInfo TypeInstance;
  static const Sexp Null;

  ArgList Args;

  explicit Sexp(const SourceContext &sourceContext);
  explicit Sexp(const SourceContext &sourceContext, ArgList &&args);
  explicit Sexp(const SourceContext &sourceContext, std::initializer_list<ExpressionPtr> &&args);
  virtual void Display(std::ostream& out) const override;
  virtual ExpressionPtr Clone() const override;
  virtual IteratorPtr GetIterator();
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Sexp &rhs) const;
  bool operator!=(const Sexp &rhs) const;
  static ExpressionPtr NewInstance(const SourceContext &sourceContext);
};

class SexpIterator: public IIterator {
public:
  explicit SexpIterator(Sexp &sexp);
  virtual ExpressionPtr& Next() override;
  virtual int64_t GetLength() override;
private:
  ArgList::iterator Curr;
  ArgList::iterator End;
  int64_t           Length;
};

struct Ref: public Expression, IIterable {
  static const TypeInfo TypeInstance;
  static const Ref Null;

  ExpressionPtr &Value;

  explicit Ref(const SourceContext &sourceContext, ExpressionPtr &value);
  virtual ExpressionPtr Clone() const override;
  ExpressionPtr NewRef() const;
  virtual void Display(std::ostream& out) const override;
  virtual IteratorPtr GetIterator();
  virtual bool operator==(const Expression &rhs) const override;
  bool operator==(const Ref &rhs) const;
  bool operator!=(const Ref &rhs) const;
};

namespace List {
  ExpressionPtr GetNil(const SourceContext &sourceContext);
}
