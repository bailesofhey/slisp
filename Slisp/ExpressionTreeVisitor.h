#pragma once

#include "Expression.h"

class ExpressionTreeVisitor {
  public:
    enum States {
      BEGIN,
      MIDDLE,
      END
    };

    virtual ~ExpressionTreeVisitor();
    virtual bool OnExpression(Sexp *expr, States state) = 0;
    virtual bool OnExpression(Number *expr, States state) = 0;
    virtual bool OnExpression(String *expr, States state) = 0;
    virtual bool OnExpression(Symbol *expr, States state) = 0;
};