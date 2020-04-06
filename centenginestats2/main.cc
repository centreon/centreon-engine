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
  std::cout << "Usage: " << program_name
            << " [options]\n\n"
               "Startup:\n"
               "  -V, --version        display program version information and "
               "exit.\n"
               "  -L, --license        display license information and exit.\n"
               "  -V, --version        display centreon-engine version.\n"
               "  -h, --help           display usage information and exit.\n"
               "\n"
               "Input options:\n"
               "  -c, --config=FILE    specifies location of main Centreon "
               "Engine config file.\n"
               "  -s, --statsfile=FILE specifies alternate location of file to "
               "read Centreon\n"
               "  -p, --port=NUMBER    specifies the port number of the engine "
               "grpc server to connect to.\n"
               "                       Engine performance data from.\n"
            << std::endl;
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

  static struct option const long_options[] = {
      {"help", no_argument, 0, 'h'},
      {"license", no_argument, 0, 'L'},
      {"config", required_argument, 0, 'c'},
      {"statsfile", required_argument, 0, 's'},
      {"port", required_argument, 0, 'p'},
      {"version", no_argument, 0, 'V'},
      {0, 0, 0, 0}};

  bool error = false;
  bool get_version = false;
  uint16_t grpc_port;
  while (!error) {
    int c = getopt_long(argc, argv, "+hLc:s:p:V", long_options, nullptr);
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
      default:
        error = true;
    }
  }

  std::cout
      << "Centreon Engine Statistics Utility " CENTREON_ENGINE_VERSION_STRING
         "\n\n"
         "Copyright 2020 Centreon\n"
         "License: Apache v2.0\n\n";

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
  }
  return 0;
  //  if (strcmp(argv[1], "GetVersion") == 0) {
  //    Version version;
  //    status = client.GetVersion(&version) ? 0 : 1;
  //    std::cout << "GetVersion: " << version.DebugString();
  //  } else if (strcmp(argv[1], "GetStats") == 0) {
  //    Stats stats;
  //    status = client.GetStats(&stats) ? 0 : 2;
  //    std::cout << "GetStats: " << stats.DebugString();
  //  } else if (strcmp(argv[1], "ProcessServiceCheckResult") == 0) {
  //    Check sc;
  //    sc.set_host_name(argv[2]);
  //    sc.set_svc_desc(argv[3]);
  //    sc.set_code(std::stol(argv[4]));
  //    sc.set_output("Test external command");
  //    status = client.ProcessServiceCheckResult(sc) ? 0 : 3;
  //    std::cout << "ProcessServiceCheckResult: " << status << std::endl;
  //  } else if (strcmp(argv[1], "ProcessHostCheckResult") == 0) {
  //    Check hc;
  //    hc.set_host_name(argv[2]);
  //    hc.set_code(std::stol(argv[3]));
  //    hc.set_output("Test external command");
  //    status = client.ProcessHostCheckResult(hc) ? 0 : 4;
  //    std::cout << "ProcessHostCheckResult: " << status << std::endl;
  //  } else if (strcmp(argv[1], "NewThresholdsFile") == 0) {
  //    ThresholdsFile tf;
  //    tf.set_filename(argv[2]);
  //    status = client.NewThresholdsFile(tf) ? 0 : 5;
  //    std::cout << "NewThresholdsFile: " << status << std::endl;
  //  }
  //  exit(status);
}

