#include "com/centreon/engine/xpddefault.hh"
#include "gtest/gtest.h"

TEST(preprocess_file_templates, WithAntislash_n) {
  char test[] = "This is\\na multiline\\n\\text.\\n";
  xpddefault_preprocess_file_templates(test);
  ASSERT_EQ(std::string(test), "This is\na multiline\n\text.\n");
}

TEST(preprocess_file_templates, NullLine) {
  char* test = nullptr;
  xpddefault_preprocess_file_templates(test);
  ASSERT_EQ(test, nullptr);
}

TEST(preprocess_file_templates, EmptyLine) {
  char test[] = "";
  xpddefault_preprocess_file_templates(test);
  ASSERT_EQ(std::string(test), "");
}

TEST(preprocess_file_templates, AntiSlash_t) {
  char test[] = "\\tb\\tc\\t\\t\\td";
  xpddefault_preprocess_file_templates(test);
  ASSERT_EQ(std::string(test), "\tb\tc\t\t\td");
}

TEST(preprocess_file_templates, AntiSlash_r) {
  char test[] = "a\\rb\\rc\\r\\r\\r";
  xpddefault_preprocess_file_templates(test);
  ASSERT_EQ(std::string(test), "a\rb\rc\r\r\r");
}
