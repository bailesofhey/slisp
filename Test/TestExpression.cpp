#include "gtest\gtest.h"

#include "Expression.h"

template <class E>
void RunExpressionTest(E &defaultValue, E &emptyValue, E &otherValue) {
  ASSERT_EQ(defaultValue, emptyValue);
  ASSERT_NE(defaultValue, otherValue);
  ASSERT_NE(emptyValue, otherValue);

  ASSERT_LT(emptyValue, otherValue);
  ASSERT_GE(otherValue, emptyValue);

  ASSERT_FALSE(otherValue.ToString().empty());
  ASSERT_NE(otherValue.ToString(), emptyValue.ToString());

  ExpressionPtr defaultCopy = defaultValue.Clone(),
                emptyCopy = emptyValue.Clone(),
                otherCopy = otherValue.Clone();
  
  ASSERT_EQ(defaultValue, *defaultCopy);
  ASSERT_EQ(otherValue, *otherCopy);
  ASSERT_EQ(emptyValue, *emptyCopy);

  ASSERT_EQ(defaultValue, static_cast<E&>(*defaultCopy));
  ASSERT_EQ(otherValue, static_cast<E&>(*otherCopy));
  ASSERT_EQ(emptyValue, static_cast<E&>(*emptyCopy));

  ASSERT_EQ(emptyValue.ToString(), emptyCopy->ToString());
  ASSERT_EQ(otherValue.ToString(), otherCopy->ToString());

  emptyValue = otherValue;
  ASSERT_EQ(emptyValue, otherValue);
  ASSERT_EQ(emptyValue.ToString(), otherValue.ToString());
}

TEST(Expression, TestBool) {
  RunExpressionTest(Bool(), Bool(false), Bool(true));
}

TEST(Expression, TestNumber) {
  RunExpressionTest(Number(), Number(0), Number(5));
}

TEST(Expression, TestString) {
  RunExpressionTest(String(), String(""), String("Foo"));
}

TEST(Expression, TestSymbol) {
  RunExpressionTest(Symbol(""), Symbol(""), Symbol("a"));
}

TEST(Expression, TestQuote) {
  Quote qThree { ExpressionPtr { new Number(3) } };
  Quote qFoo { ExpressionPtr { new String("Foo") } };
  ASSERT_FALSE(qThree.ToString().empty());
  ASSERT_NE(qThree.ToString(), qFoo.ToString());
  ASSERT_NE(qThree, qFoo);

  ExpressionPtr threeCopyExpr { qThree.Clone() };
  Quote *threeCopy = static_cast<Quote*>(threeCopyExpr.get());
  ASSERT_EQ(qThree.ToString(), threeCopy->ToString());
  ASSERT_EQ(qThree, *threeCopy);

  threeCopy->Value = ExpressionPtr { new Number(33) };
  ASSERT_NE(qThree, *threeCopy);
  threeCopy->Value = ExpressionPtr { new Number(3) };
  ASSERT_EQ(qThree, *threeCopy);
}

TEST(Expression, TestSexp) {
  Sexp s;
  s.Args.push_back(ExpressionPtr { new Symbol("+") });
  s.Args.push_back(ExpressionPtr { new Number(3) });
  s.Args.push_back(ExpressionPtr { new Number(4) });

  ExpressionPtr sCopyExpr { s.Clone() };
  Sexp *sCopy = static_cast<Sexp*>(sCopyExpr.get());
  
  ASSERT_EQ(s, *sCopy);
  
  ExpressionPtr arg1 = std::move(sCopy->Args.front());
  sCopy->Args.pop_front();

  ASSERT_NE(s, *sCopy);

  ExpressionPtr arg2 = std::move(sCopy->Args.front());
  sCopy->Args.pop_front();
  ExpressionPtr arg3 = std::move(sCopy->Args.front());
  sCopy->Args.pop_front();

  auto argSym = dynamic_cast<Symbol*>(arg1.get());
  auto argNum1 = dynamic_cast<Number*>(arg2.get());
  auto argNum2 = dynamic_cast<Number*>(arg3.get());
  ASSERT_TRUE(argSym != nullptr);
  ASSERT_TRUE(argNum1 != nullptr);
  ASSERT_TRUE(argNum2 != nullptr);

  ASSERT_EQ(Symbol("+"), *argSym);
  ASSERT_EQ(Number(3), *argNum1);
  ASSERT_EQ(Number(4), *argNum2);
}
