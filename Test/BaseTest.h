#pragma once

#include "gtest/gtest.h"
#include "Expression.h"
#include "ExpressionFactory.h"

class BaseTest: public testing::Test {
public:
  ExpressionFactory Factory;

  explicit BaseTest();
};