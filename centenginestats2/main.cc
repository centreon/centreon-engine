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
#include <getopt.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "centenginestats_client.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/version.hh"

using namespace com::centreon::engine;

static void display_help(const char* program_name) {
  printf(
      "Usage: %s [options]\n\n"
      "Startup:\n"
      "  -V, --version        display centreon-engine version and exit.\n"
      "  -L, --license        display license information and exit.\n"
      "  -h, --help           display usage information and exit.\n"
      "\n"
      "Input options:\n"
      "  -c, --config=FILE    specifies location of main Centreon "
      "Engine config file.\n"
      "  -s, --statsfile=FILE specifies alternate location of file to "
      "read Centreon\n"
      "  -p, --port=NUMBER    specifies the port number of the engine "
      "grpc server to connect to.\n"
      "  -o, --object=STRING  can contain be:\n"
      "     * default         to get centengine stats.\n"
      "     * start           to get durations during centengine start.\n\n"
      "                       Engine performance data from.\n\n",
      program_name);
}

static void display_license() {
  std::cout
      << "Copyright 2020 Centreon (https://www.centreon.com/)\n\n"
         "Licensed under the Apache License, Version 2.0 (the \" License\");\n"
         "you may not use this file except in compliance with the License.\n"
         "You may obtain a copy of the License at\n\n"
         "http://www.apache.org/licenses/LICENSE-2.0\n\n"
         "Unless required by applicable law or agreed to in writing, software\n"
         "distributed under the License is distributed on an \" AS IS\" "
         "BASIS,\n"
         "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or "
         "implied.\n"
         "See the License for the specific language governing permissions and\n"
         "limitations under the License.\n\n"
         "For more information : contact@centreon.com\n"
      << std::endl;
}

int main(int argc, char** argv) {
  std::string config_file("/etc/centreon-engine/centengine.cfg");
  std::string stats_file("/var/log/centreon-engine/status.dat");
  std::string object("default");

  static struct option const long_options[] = {
      {"help", no_argument, 0, 'h'},
      {"license", no_argument, 0, 'L'},
      {"config", required_argument, 0, 'c'},
      {"statsfile", required_argument, 0, 's'},
      {"port", required_argument, 0, 'p'},
      {"version", no_argument, 0, 'V'},
      {"object", required_argument, 0, 'o'},
      {0, 0, 0, 0}};

  bool error = false;
  bool get_version = false;
  uint16_t grpc_port = 0;
  while (!error) {
    int c = getopt_long(argc, argv, "+hLc:s:p:Vo:", long_options, nullptr);
    if (c == -1)
      break;

    switch (c) {
      case 'h':
        display_help(argv[0]);
        exit(0);
        break;
      case 'L':
        display_license();
        exit(0);
        break;
      case 'c':
        config_file = std::string(optarg);
        break;
      case 's':
        stats_file = std::string(optarg);
        break;
      case 'p':
        grpc_port = strtol(optarg, nullptr, 10);
        break;
      case 'V':
        get_version = true;
        break;
      case 'o':
        object = std::string(optarg);
        break;
      default:
        error = true;
    }
  }

  printf("Centreon Engine Statistics Utility " CENTREON_ENGINE_VERSION_STRING
         "\n\n"
         "Copyright 2020 Centreon\n"
         "License: Apache v2.0\n\n");

  int32_t status;
  centenginestats_client client(grpc_port, config_file, stats_file);

  if (!client.is_configured())
    exit(2);

  if (get_version) {
    try {
      std::string version = client.get_version();
      std::cout << "Centreon-engine " << version << std::endl;
    } catch (std::invalid_argument const& e) {
      std::cerr << e.what() << std::endl;
      return 3;
    }
  } else
    client.get_stats(object);

  return 0;
}
