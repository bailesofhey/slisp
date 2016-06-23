#include "gtest\gtest.h"
#include "Utils.h"

using namespace std;

TEST(Utils, TestEndsWith) {
  EXPECT_FALSE(Utils::EndsWith("", ""));
  EXPECT_FALSE(Utils::EndsWith("a", ""));
  EXPECT_FALSE(Utils::EndsWith("", "a"));

  EXPECT_TRUE(Utils::EndsWith("a", "a"));
  EXPECT_FALSE(Utils::EndsWith("b", "a"));
  EXPECT_FALSE(Utils::EndsWith("a", "b"));

  EXPECT_TRUE(Utils::EndsWith("ab", "b"));
  EXPECT_TRUE(Utils::EndsWith("ab", "ab"));

  EXPECT_FALSE(Utils::EndsWith("abc", "a"));
  EXPECT_FALSE(Utils::EndsWith("abc", "b"));
  EXPECT_TRUE(Utils::EndsWith("abc", "c"));
  EXPECT_TRUE(Utils::EndsWith("abc", "bc"));
  EXPECT_TRUE(Utils::EndsWith("abc", "abc"));
  EXPECT_FALSE(Utils::EndsWith("abc", "abcd"));
}