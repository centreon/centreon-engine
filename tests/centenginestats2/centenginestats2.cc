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

#include <gtest/gtest.h>
#include <sstream>

#define executable "centenginestats2/centenginestats2"

static std::string execute(const std::string& command) {
  std::ostringstream oss;
  char path[1024];

  FILE* fp = popen(command.c_str(), "r");
  while (fgets(path, sizeof(path), fp) != nullptr) {
    size_t count = strlen(path);
    if (count > 0)
      --count;
    oss << std::string(path, count);
  }
  pclose(fp);
  return oss.str();
}

TEST(stats2, Help) {
  ASSERT_EQ(execute(executable " -h"),
            "Usage: centenginestats2/centenginestats2 [options]Startup:  -V, "
            "--version        display program version information and exit.  "
            "-L, --license        display license information and exit.  -h, "
            "--help           display usage information and exit.Input "
            "options:  -c, --config=FILE    specifies location of main "
            "Centreon Engine config file.  -s, --statsfile=FILE specifies "
            "alternate location of file to read Centreon  -p, --port=NUMBER    "
            "specifies the port number of the engine grpc server to connect "
            "to.                       Engine performance data from.");
}

TEST(stats2, License) {
  ASSERT_EQ(
      execute(executable " -L"),
      "Copyright 2020 Centreon (https://www.centreon.com/)Licensed under the "
      "Apache License, Version 2.0 (the \" License\");you may not use this "
      "file except in compliance with the License.You may obtain a copy of the "
      "License athttp://www.apache.org/licenses/LICENSE-2.0Unless required by "
      "applicable law or agreed to in writing, softwaredistributed under the "
      "License is distributed on an \" AS IS\" BASIS,WITHOUT WARRANTIES OR "
      "CONDITIONS OF ANY KIND, either express or implied.See the License for "
      "the specific language governing permissions andlimitations under the "
      "License.For more information : contact@centreon.com");
}
