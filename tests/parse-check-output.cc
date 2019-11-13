#include "com/centreon/engine/utils.hh"
#include "gtest/gtest.h"

TEST(ParseCheckOutput, singleLineWithoutPerfdata) {
  std::string buf = "The service is OK";
  std::string short_output;
  std::string long_output;
  std::string perf_data;
  bool escape_newlines{true};
  bool newlines_are_escaped{true};

  parse_check_output(buf, short_output, long_output, perf_data, escape_newlines,
                     newlines_are_escaped);
  ASSERT_EQ(short_output, "The service is OK");
  ASSERT_EQ(long_output, "");
  ASSERT_EQ(perf_data, "");
}

TEST(ParseCheckOutput, singleLineWithPerfdata) {
  std::string buf = "The service is OK | a=25;50;75";
  std::string short_output;
  std::string long_output;
  std::string perf_data;
  bool escape_newlines{true};
  bool newlines_are_escaped{true};

  parse_check_output(buf, short_output, long_output, perf_data, escape_newlines,
                     newlines_are_escaped);
  ASSERT_EQ(short_output, "The service is OK");
  ASSERT_EQ(long_output, "");
  ASSERT_EQ(std::string(perf_data), std::string("a=25;50;75"));
}

TEST(ParseCheckOutput, multipleLineWithLongPerfdata) {
  std::string buf = "The service is OK | a=25;50;75 b=1\nToto is a good guy";
  std::string short_output;
  std::string long_output;
  std::string perf_data;
  bool escape_newlines{true};
  bool newlines_are_escaped{false};

  parse_check_output(buf, short_output, long_output, perf_data, escape_newlines,
                     newlines_are_escaped);
  ASSERT_EQ(short_output, "The service is OK");
  ASSERT_EQ(long_output, "Toto is a good guy");
  ASSERT_EQ(perf_data, "a=25;50;75 b=1");
}

TEST(ParseCheckOutput, multipleLineWithLongPerfdata1) {
  std::string buf =
      "The service is OK | a=25;50;75 b=1\nToto is a good guy\nBar is well "
      "known";
  std::string short_output;
  std::string long_output;
  std::string perf_data;
  bool escape_newlines{true};
  bool newlines_are_escaped{false};

  parse_check_output(buf, short_output, long_output, perf_data, escape_newlines,
                     newlines_are_escaped);
  ASSERT_EQ(short_output, "The service is OK");
  ASSERT_EQ(long_output, "Toto is a good guy\\nBar is well known");
  ASSERT_EQ(perf_data, "a=25;50;75 b=1");
}

TEST(ParseCheckOutput, singleLineWithMultiplePipes) {
  std::string buf = "The service is OK | The host is UP | a=25;50;75 b=1";
  std::string short_output;
  std::string long_output;
  std::string perf_data;
  bool escape_newlines{true};
  bool newlines_are_escaped{true};

  parse_check_output(buf, short_output, long_output, perf_data, escape_newlines,
                     newlines_are_escaped);
  ASSERT_EQ(short_output, "The service is OK | The host is UP");
  ASSERT_EQ(long_output, "");
  ASSERT_EQ(perf_data, std::string("a=25;50;75 b=1"));
}

TEST(ParseCheckOutput, multipleLineWithLongPerfdataEscaped) {
  std::string buf = "The service is OK | a=25;50;75 b=1\\nToto is a good guy";
  std::string short_output;
  std::string long_output;
  std::string perf_data;
  bool escape_newlines{true};
  bool newlines_are_escaped{true};

  parse_check_output(buf, short_output, long_output, perf_data, escape_newlines,
                     newlines_are_escaped);
  ASSERT_EQ(short_output, "The service is OK");
  ASSERT_EQ(long_output, "Toto is a good guy");
  ASSERT_EQ(perf_data, "a=25;50;75 b=1");
}

TEST(ParseCheckOutput, multipleLineWithLongPerfdata1Escaped) {
  std::string buf =
      "The service is OK | a=25;50;75 b=1\\nToto is a good guy\\nBar is well "
      "known";
  std::string short_output;
  std::string long_output;
  std::string perf_data;
  bool escape_newlines{true};
  bool newlines_are_escaped{true};

  parse_check_output(buf, short_output, long_output, perf_data, escape_newlines,
                     newlines_are_escaped);
  ASSERT_EQ(short_output, "The service is OK");
  ASSERT_EQ(long_output, "Toto is a good guy\\nBar is well known");
  ASSERT_EQ(perf_data, "a=25;50;75 b=1");
}

TEST(ParseCheckOutput, multipleLineWithLongPerfdataReturnEscaped) {
  std::string buf = "The service is OK | a=25;50;75 b=1\\nToto is a good guy";
  std::string short_output;
  std::string long_output;
  std::string perf_data;
  bool escape_newlines{false};
  bool newlines_are_escaped{true};

  parse_check_output(buf, short_output, long_output, perf_data, escape_newlines,
                     newlines_are_escaped);
  ASSERT_EQ(short_output, "The service is OK");
  ASSERT_EQ(long_output, "Toto is a good guy");
  ASSERT_EQ(perf_data, "a=25;50;75 b=1");
}

TEST(ParseCheckOutput, multipleLineWithLongPerfdata1ReturnEscaped) {
  std::string buf =
      "The service is OK | a=25;50;75 b=1\\nToto is a good guy\\nBar is well "
      "known";
  std::string short_output;
  std::string long_output;
  std::string perf_data;
  bool escape_newlines{false};
  bool newlines_are_escaped{true};

  parse_check_output(buf, short_output, long_output, perf_data, escape_newlines,
                     newlines_are_escaped);
  ASSERT_EQ(short_output, "The service is OK");
  ASSERT_EQ(long_output, "Toto is a good guy\nBar is well known");
  ASSERT_EQ(perf_data, "a=25;50;75 b=1");
}

TEST(ParseCheckOutput, multipleLineWithMultiplePerfdata1ReturnEscaped) {
  std::string buf =
      "DISK OK - free space: / 3326 MB (56%); | /=2643MB;5948;5958;0;5968\n/ "
      "15272 MB (77%);\n/boot 68 MB (69%);\n/home 69357 MB (27%);\n/var/log "
      "819 MB (84%); | "
      "/boot=68MB;88;93;0;98\n/home=69357MB;253404;253409;0;253414\n/var/"
      "log=818MB;970;975;0;980";
  std::string short_output;
  std::string long_output;
  std::string perf_data;
  bool escape_newlines{false};
  bool newlines_are_escaped{true};

  parse_check_output(buf, short_output, long_output, perf_data, escape_newlines,
                     newlines_are_escaped);
  ASSERT_EQ(short_output, "DISK OK - free space: / 3326 MB (56%);");
  ASSERT_EQ(long_output,
            "/ 15272 MB (77%);\n/boot 68 MB (69%);\n/home 69357 MB "
            "(27%);\n/var/log 819 MB (84%);");
  ASSERT_EQ(
      perf_data,
      "/=2643MB;5948;5958;0;5968 /boot=68MB;88;93;0;98 "
      "/home=69357MB;253404;253409;0;253414 /var/log=818MB;970;975;0;980");
}
