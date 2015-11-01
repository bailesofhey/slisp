#pragma once

#include "ExpressionTreeVisitor.h"
#include "Expression.h"

class ExpressionTreeDumper: public ExpressionTreeVisitor {
  public:
    ExpressionTreeDumper(bool displayExpression);
    virtual bool OnExpression(Sexp *expr, States state);
    virtual bool OnExpression(Number *expr, States state);
    virtual bool OnExpression(String *expr, States state);
    virtual bool OnExpression(Symbol *expr, States state);

  private:
    int  Depth;
    bool DisplayExpression;
    bool FirstArg;

    template<class T> bool DumpValue(T *expr, char *wrapper = nullptr);
};