#include "gtest/gtest.h"
#include "Expression.h"
#include "BaseTest.h"

using namespace std;

class ExpressionTest: public BaseTest {
};

template <class E>
void RunExpressionTest(E &&defaultValue, E &&emptyValue, E &&otherValue) {
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

TEST_F(ExpressionTest, TestBool) {
  RunExpressionTest(*Factory.Alloc<Bool>(), *Factory.Alloc<Bool>(false), *Factory.Alloc<Bool>(true));
}

TEST_F(ExpressionTest, TestInt) {
  RunExpressionTest(*Factory.Alloc<Int>(), *Factory.Alloc<Int>(0), *Factory.Alloc<Int>(5));
}

TEST_F(ExpressionTest, TestFloat) {
  RunExpressionTest(*Factory.Alloc<Float>(), *Factory.Alloc<Float>(0.0), *Factory.Alloc<Float>(3.14));
}

TEST_F(ExpressionTest, TestStr) {
  RunExpressionTest(*Factory.Alloc<Str>(), *Factory.Alloc<Str>(""), *Factory.Alloc<Str>("Foo"));
}

TEST_F(ExpressionTest, TestSymbol) {
  RunExpressionTest(*Factory.Alloc<Symbol>(""), *Factory.Alloc<Symbol>(""), *Factory.Alloc<Symbol>("a"));
}

TEST_F(ExpressionTest, TestQuote) {
  Quote qThree { NullSourceContext, ExpressionPtr { Factory.Alloc<Int>(3) } };
  Quote qFoo { NullSourceContext, ExpressionPtr { Factory.Alloc<Str>("Foo") } };
  ASSERT_FALSE(qThree.ToString().empty());
  ASSERT_NE(qThree.ToString(), qFoo.ToString());
  ASSERT_NE(qThree, qFoo);

  ExpressionPtr threeCopyExpr { qThree.Clone() };
  Quote *threeCopy = static_cast<Quote*>(threeCopyExpr.get());
  ASSERT_EQ(qThree.ToString(), threeCopy->ToString());
  ASSERT_EQ(qThree, *threeCopy);

  threeCopy->Value = ExpressionPtr { Factory.Alloc<Int>(33) };
  ASSERT_NE(qThree, *threeCopy);
  threeCopy->Value = ExpressionPtr { Factory.Alloc<Int>(3) };
  ASSERT_EQ(qThree, *threeCopy);
}

TEST_F(ExpressionTest, TestSexp) {
  auto *s = Factory.Alloc<Sexp>();
  s->Args.emplace_back(Factory.Alloc<Symbol>("+"));
  s->Args.emplace_back(Factory.Alloc<Int>(3));
  s->Args.emplace_back(Factory.Alloc<Int>(4));
  s->Args.emplace_back(Factory.Alloc<Float>(3.14));

  ExpressionPtr sCopyExpr { s->Clone() };
  Sexp *sCopy = static_cast<Sexp*>(sCopyExpr.get());
  
  ASSERT_EQ(*s, *sCopy);
  
  ExpressionPtr arg1 = move(sCopy->Args.front());
  sCopy->Args.pop_front();

  ASSERT_NE(*s, *sCopy);

  ExpressionPtr arg2 = move(sCopy->Args.front());
  sCopy->Args.pop_front();
  ExpressionPtr arg3 = move(sCopy->Args.front());
  sCopy->Args.pop_front();
  ExpressionPtr arg4 = move(sCopy->Args.front());
  sCopy->Args.pop_front();

  auto argSym = dynamic_cast<Symbol*>(arg1.get());
  auto argInt1 = dynamic_cast<Int*>(arg2.get());
  auto argInt2 = dynamic_cast<Int*>(arg3.get());
  auto argFloat = dynamic_cast<Float*>(arg4.get());
  ASSERT_TRUE(argSym != nullptr);
  ASSERT_TRUE(argInt1 != nullptr);
  ASSERT_TRUE(argInt2 != nullptr);
  ASSERT_TRUE(argFloat != nullptr);

  ASSERT_EQ(*Factory.Alloc<Symbol>("+"), *argSym);
  ASSERT_EQ(*Factory.Alloc<Int>(3), *argInt1);
  ASSERT_EQ(*Factory.Alloc<Int>(4), *argInt2);
  ASSERT_EQ(*Factory.Alloc<Float>(3.14), *argFloat);
}
