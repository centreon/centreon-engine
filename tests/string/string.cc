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
#include "com/centreon/engine/string.hh"

#include "gtest/gtest.h"

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
  std::string perfdata(
      "metric_2=2;3;7;1;9 metric=12;25;50;0;118 metric_1=28;13;54;0;80");
  ASSERT_EQ(string::extract_perfdata(perfdata, "metric"),
            "metric=12;25;50;0;118");
}

TEST(string_utils, extractPerfdataQuotes) {
  std::string perfdata(
      "'aa a aa'=2;3;7;1;9 'a aa'=12;25;50;0;118 'aa a'=28;13;54;0;80");
  ASSERT_EQ(string::extract_perfdata(perfdata, "a aa"),
            "'a aa'=12;25;50;0;118");
  ASSERT_EQ(string::extract_perfdata(perfdata, "aa a"), "'aa a'=28;13;54;0;80");
}

TEST(string_utils, extractPerfdataGaugeDiff) {
  std::string perfdata(
      "'aa a aa'=2;3;7;1;9 g[a aa]=12;25;50;0;118 d[aa a]=28;13;54;0;80");
  ASSERT_EQ(string::extract_perfdata(perfdata, "a aa"),
            "g[a aa]=12;25;50;0;118");
  ASSERT_EQ(string::extract_perfdata(perfdata, "aa a"),
            "d[aa a]=28;13;54;0;80");
}

TEST(string_utils, removeThresholdsWithoutThresholds) {
  std::string perfdata("a=2V");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V");
}

TEST(string_utils, removeThresholdsWithoutThresholds2) {
  std::string perfdata("a=2V;");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V");
}

TEST(string_utils, removeThresholdsWithoutThresholds3) {
  std::string perfdata("a=2V;");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V");
}

TEST(string_utils, removeThresholdsWithOneThreshold) {
  std::string perfdata("a=2V;5");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V");
}

TEST(string_utils, removeThresholdsWithOneThreshold2) {
  std::string perfdata("a=2V;5;");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V");
}

TEST(string_utils, removeThresholdsWithTwoThresholds1) {
  std::string perfdata("a=2V;5;9");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V");
}

TEST(string_utils, removeThresholdsWithTwoThresholds2) {
  std::string perfdata("a=2V;5;9;");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V;;;");
}

TEST(string_utils, removeThresholdsWithTwoThresholds3) {
  std::string perfdata("a=2V;;9;");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V;;;");
}

TEST(string_utils, removeThresholdsWithTwoThresholds4) {
  std::string perfdata("a=2V;;;");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V;;;");
}

TEST(string_utils, removeThresholdsMoreComplex) {
  std::string perfdata("a=2V;5;9;0;10");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V;;;0;10");
}

TEST(string_utils, removeThresholdsMoreComplex2) {
  std::string perfdata("a=2V;5;9;0;");
  ASSERT_EQ(string::remove_thresholds(perfdata), "a=2V;;;0;");
}

/*
 * Given a string encoded in ISO-8859-15 and CP-1252
 * Then the check_string_utf8 function converts it to UTF-8.
 */
TEST(string_check_utf8, simple) {
  std::string txt("L'acc\350s \340 l'h\364tel est encombr\351");
  ASSERT_EQ(string::check_string_utf8(txt), "L'accès à l'hôtel est encombré");
}

/*
 * Given a string encoded in UTF-8
 * Then the check_string_utf8 function returns itself.
 */
TEST(string_check_utf8, utf8) {
  std::string txt("L'accès à l'hôtel est encombré");
  ASSERT_EQ(string::check_string_utf8(txt), "L'accès à l'hôtel est encombré");
}

/*
 * Given a string encoded in CP-1252
 * Then the check_string_utf8 function converts it to UTF-8.
 */
TEST(string_check_utf8, cp1252) {
  std::string txt("Le ticket co\xfbte 12\x80\n");
  ASSERT_EQ(string::check_string_utf8(txt), "Le ticket coûte 12€\n");
}

/*
 * Given a string encoded in ISO-8859-15
 * Then the check_string_utf8 function converts it to UTF-8.
 */
TEST(string_check_utf8, iso8859) {
  std::string txt("Le ticket co\xfbte 12\xa4\n");
  ASSERT_EQ(string::check_string_utf8(txt), "Le ticket coûte 12€\n");
}

/*
 * Given a string encoded in ISO-8859-15
 * Then the check_string_utf8 function converts it to UTF-8.
 */
TEST(string_check_utf8, iso8859_cpx) {
  std::string txt("\xa4\xa6\xa8\xb4\xb8\xbc\xbd\xbe");
  ASSERT_EQ(string::check_string_utf8(txt), "€ŠšŽžŒœŸ");
}

/*
 * Given a string encoded in CP-1252
 * Then the check_string_utf8 function converts it to UTF-8.
 */
TEST(string_check_utf8, cp1252_cpx) {
  std::string txt("\x80\x95\x82\x89\x8a");
  ASSERT_EQ(string::check_string_utf8(txt), "€•‚‰Š");
}

/*
 * Given a string badly encoded in CP-1252
 * Then the check_string_utf8 function converts it to UTF-8 and replaces bad
 * characters into '_'.
 */
TEST(string_check_utf8, whatever_as_cp1252) {
  std::string txt;
  for (uint8_t c = 32; c < 255; c++)
    if (c != 127)
      txt.push_back(c);
  std::string result(
      " !\"#$%&'()*+,-./"
      "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
      "abcdefghijklmnopqrstuvwxyz{|}~€_‚ƒ„…†‡ˆ‰Š‹Œ_Ž__‘’“”•–—˜™š›œ_"
      "žŸ ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäå"
      "æçèéêëìíîïðñòóôõö÷øùúûüýþ");
  ASSERT_EQ(string::check_string_utf8(txt), result);
}

/*
 * Given a string badly encoded in ISO-8859-15
 * Then the check_string_utf8 function converts it to UTF-8 and replaces bad
 * characters into '_'.
 */
TEST(string_check_utf8, whatever_as_iso8859) {
  /* Construction of a string that is not cp1252 so it should be considered as
   * iso8859-15 */
  std::string txt;
  for (uint8_t c = 32; c < 255; c++) {
    if (c == 32)
      txt.push_back(129);
    if (c != 127)
      txt.push_back(c);
  }
  std::string result(
      "_ "
      "!\"#$%&'()*+,-./"
      "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
      "abcdefghijklmnopqrstuvwxyz{|}~_________________________________"
      "¡¢£€¥Š§š©ª«¬­®¯°±²³Žµ¶·ž¹º»ŒœŸ¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçè"
      "éêëìíîïðñòóôõö÷øùúûüýþ");
  ASSERT_EQ(string::check_string_utf8(txt), result);
}

/*
 * In case of a string containing multiple encoding, the resulting string should
 * be an UTF-8 string. Here we have a string beginning with UTF-8 and finishing
 * with cp1252. The resulting string is good and is UTF-8 only encoded.
 */
TEST(string_check_utf8, utf8_and_cp1252) {
  std::string txt(
      "\xc3\xa9\xc3\xa7\xc3\xa8\xc3\xa0\xc3\xb9\xc3\xaf\xc3\xab\x7e\x23\x0a\xe9"
      "\xe7\xe8\xe0\xf9\xef\xeb\x7e\x23\x0a");
  std::string result("éçèàùïë~#\néçèàùïë~#\n");
  ASSERT_EQ(string::check_string_utf8(txt), result);
}
