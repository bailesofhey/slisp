#include "gtest/gtest.h"
#include "BaseTest.h"
#include "Expression.h"
#include "ExpressionFactory.h"

using namespace std;

//=============================================================================

BaseTest::BaseTest():
  Test(),
  Factory(NullSourceContext)
{
}
