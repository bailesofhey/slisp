#pragma once

#include "Expression.h"

template<class T>
struct ExpressionContainer {
  ExpressionPtr Expr;
  T& Val;

  explicit ExpressionContainer(ExpressionPtr &&expr, T& val):
    Expr(move(expr)),
    Val(val)
  {
  }

  explicit operator bool() const {
    if (Expr)
      return true;
    else
      return false;
  }
};

class ExpressionFactory {
public:
  explicit ExpressionFactory(const SourceContext &sourceContext);

  template<typename T, typename... Args>
  T* Alloc(Args&&... args) {
    return new T(SourceContext_, std::forward<Args>(args)...);
  }

  template<typename T, typename... Args>
  ExpressionContainer<T> New(Args&&... args) {
    ExpressionPtr exprPtr { Alloc<T>(std::forward<Args>(args)...) };
    if (exprPtr)
      return ExpressionContainer<T>(move(exprPtr), static_cast<T&>(*exprPtr));
    else
      return ExpressionContainer<T>(ExpressionPtr {}, const_cast<T&>(T::Null));
  }

private:
  const SourceContext &SourceContext_;
};
