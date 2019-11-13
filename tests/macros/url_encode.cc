#include "com/centreon/engine/macros.hh"
#include "gtest/gtest.h"

TEST(TestMacros, UrlEncode) {
  std::string ret{url_encode("This is a simple line")};
  ASSERT_EQ(std::string{ret}, "This%20is%20a%20simple%20line");
}

TEST(TestMacros, UrlEncodeAccents) {
  std::string ret{url_encode("La leçon du château de l'araignée")};
  ASSERT_EQ(std::string{ret},
            "La%20le%C3%A7on%20du%20ch%C3%A2teau%20de%20l%27araign%C3%A9e");
}

TEST(TestMacros, UrlEncodeSymbols) {
  std::string ret{url_encode("A.a-b_B:c/C?d=D&e~")};
  ASSERT_EQ(std::string{ret}, "A.a-b_B%3Ac%2FC%3Fd%3DD%26e~");
}
