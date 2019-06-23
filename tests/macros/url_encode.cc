#include "gtest/gtest.h"
#include "com/centreon/engine/macros.hh"

TEST(TestMacros, UrlEncode) {
  std::string ret{get_url_encoded_string("This is a simple line")};
  ASSERT_EQ(std::string{ret}, "This+is+a+simple+line");
}

//TEST(preprocess_file_templates, NullLine) {
//  char* test = nullptr;
//  xpddefault_preprocess_file_templates(test);
//  ASSERT_EQ(test, nullptr);
//}
//
//TEST(preprocess_file_templates, EmptyLine) {
//  char test[] = "";
//  xpddefault_preprocess_file_templates(test);
//  ASSERT_EQ(std::string(test), "");
//}
//
//TEST(preprocess_file_templates, AntiSlash_t) {
//  char test[] = "\\tb\\tc\\t\\t\\td";
//  xpddefault_preprocess_file_templates(test);
//  ASSERT_EQ(std::string(test), "\tb\tc\t\t\td");
//}
//
//TEST(preprocess_file_templates, AntiSlash_r) {
//  char test[] = "a\\rb\\rc\\r\\r\\r";
//  xpddefault_preprocess_file_templates(test);
//  ASSERT_EQ(std::string(test), "a\rb\rc\r\r\r");
//}
