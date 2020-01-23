#include "gtest/gtest.h"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;

TEST(string_utils, trim) {
  std::string str("Hi guys!");
  string::trim(str);
  ASSERT_EQ(str, "Hi guys!");

  str = " a b c  ";
  string::trim(str);
  ASSERT_EQ(str, "a b c");

  str = "performance_data=rta=0.053ms;3000.000;5000.000;0; pl=0%;80;100;0;100 rtmax=0.053ms;;;; rtmin=0.053ms;;;;";
  string::trim(str);
  ASSERT_EQ(str, "performance_data=rta=0.053ms");
}
