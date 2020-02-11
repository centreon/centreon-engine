/*
 * Copyright 2020 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */
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

  str =
      "performance_data=rta=0.053ms;3000.000;5000.000;0; pl=0%;80;100;0;100 "
      "rtmax=0.053ms;;;; rtmin=0.053ms;;;;";
  string::trim(str);
  ASSERT_EQ(str, "performance_data=rta=0.053ms");
}

TEST(string_utils, extractPerfdataSimple) {
  std::string perfdata("metric_2=2;3;7;1;9 metric=12;25;50;0;118 metric_1=28;13;54;0;80");
  ASSERT_EQ(string::extract_perfdata(perfdata, "metric"), "metric=12;25;50;0;118");
}

TEST(string_utils, extractPerfdataQuotes) {
  std::string perfdata("'aa a aa'=2;3;7;1;9 'a aa'=12;25;50;0;118 'aa a'=28;13;54;0;80");
  ASSERT_EQ(string::extract_perfdata(perfdata, "a aa"), "'a aa'=12;25;50;0;118");
  ASSERT_EQ(string::extract_perfdata(perfdata, "aa a"), "'aa a'=28;13;54;0;80");
}

TEST(string_utils, extractPerfdataGaugeDiff) {
  std::string perfdata("'aa a aa'=2;3;7;1;9 g[a aa]=12;25;50;0;118 d[aa a]=28;13;54;0;80");
  ASSERT_EQ(string::extract_perfdata(perfdata, "a aa"), "g[a aa]=12;25;50;0;118");
  ASSERT_EQ(string::extract_perfdata(perfdata, "aa a"), "d[aa a]=28;13;54;0;80");
}
