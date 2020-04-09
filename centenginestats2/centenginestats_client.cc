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

#include "centenginestats_client.hh"

#include <google/protobuf/util/time_util.h>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <stdint.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "enginerpc/engine.grpc.pb.h"

using namespace com::centreon::engine;
using namespace google::protobuf::util;

static std::string duration_to_str(uint32_t diff) {
  std::ostringstream oss;
  uint32_t d = 0, h, m;
  if (diff > 86400) {
    d = diff / 86400;
    diff %= 86400;
    oss << d << "d ";
  }
  h = diff / 3600;
  diff %= 3600;
  oss << h << "h ";

  m = diff / 60;
  diff %= 60;
  oss << m << "m " << diff << "s";
  return oss.str();
}

centenginestats_client::centenginestats_client(uint16_t grpc_port,
                                               std::string const& config_file,
                                               std::string const& stats_file)
    : _config_file(config_file),
      _stats_file(stats_file),
      _status_creation_date(0),
      _grpc_port(grpc_port) {
  // If no grpc port is given, we can get it from here.
  if (!_stats_file.empty())
    read_stats_file();

  if (!_grpc_port)
    std::cerr << "The GRPC port is not configured. Unable to get informations "
                 "from centreon-engine"
              << std::endl;
  else {
    std::string address("127.0.0.1:");
    address += std::to_string(_grpc_port);
    std::shared_ptr<grpc::Channel> channel = std::shared_ptr<grpc::Channel>(
        grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
    _stub = Engine::NewStub(channel);
  }
}

bool centenginestats_client::is_configured() const {
  return static_cast<bool>(_stub);
}

/**
 * @brief Read the status.dat file. We that version of centenginestats we
 * just want to read to data: the _created_ value and the _grpc_port_ one.
 *
 * @return 0 on success.
 */
int centenginestats_client::read_stats_file() {
  std::ifstream ifs(_stats_file);
  std::string line;
  size_t count;
  if (_grpc_port == 0)
    /* created and grpc_port fields need to be read */
    count = 2;
  else
    /* only created field need to be read */
    count = 1;
  if (ifs) {
    while (count > 0 && std::getline(ifs, line)) {
      size_t pos;
      if ((pos = line.find("created=")) != std::string::npos) {
        _status_creation_date = strtoul(line.c_str() + pos + 8, nullptr, 10);
        count--;
      } else if (_grpc_port == 0 &&
                 (pos = line.find("grpc_port=")) != std::string::npos) {
        _grpc_port = strtoul(line.c_str() + pos + 10, nullptr, 10);
        count--;
      }
    }
    return 0;
  }
  std::cerr << "Unble to read '" << _stats_file << "'" << std::endl;
  return 1;
}

std::string centenginestats_client::get_version() {
  Version version;
  const ::google::protobuf::Empty e;
  grpc::ClientContext context;
  grpc::Status status = _stub->GetVersion(&context, e, &version);
  if (!status.ok())
    throw std::invalid_argument(
        "Failed to ask for the centreon-engine version. It is probably not "
        "running.");
  else {
    std::ostringstream oss;
    oss << version.major() << "." << version.minor() << "." << version.patch();
    return oss.str();
  }
}

void centenginestats_client::get_stats() {
  time_t current_time;
  time(&current_time);
  uint32_t time_difference = current_time - _status_creation_date;
  grpc::ClientContext context;
  Stats stats;
  const ::google::protobuf::Empty e;
  grpc::Status status = _stub->GetStats(&context, e, &stats);
  if (!status.ok())
    throw std::invalid_argument(
        "Failed to get the centengine statistics. It is probably not running.");

  uint32_t program_start =
      TimeUtil::TimestampToSeconds(stats.program_status().program_start());
  uint32_t program_age = current_time - program_start;

  std::cout
      << "CURRENT STATUS DATA\n"
         "---------------------------------------------------------------"
         "-----------------\n"
         "Status File:                            "
      << _stats_file
      << "\n"
         "Status File Age:                        "
      << duration_to_str(time_difference)
      << "\n\n"
         "Program Running Time:                   "
      << duration_to_str(program_age)
      << "\n"
         "Centreon Engine PID:                    "
      << stats.program_status().pid()
      << "\n"
         "Used/High/Total command Buffers:        "
      << stats.program_status().used_external_command_buffer_slots() << " / "
      << stats.program_status().high_external_command_buffer_slots() << " / "
      << stats.program_status().total_external_command_buffer_slots()
      << "\n\n"
         "Total Services:                         "
      << stats.services_stats().services_count()
      << "\nServices Checked:                       "
      << stats.services_stats().checked_services()
      << "\nServices Scheduled:                     "
      << stats.services_stats().scheduled_services()
      << "\nServices Actively Checked:              "
      << stats.services_stats().actively_checked()
      << "\nServices Passively Checked:             "
      << stats.services_stats().passively_checked()
      << "\nTotal Service State Change:             " << std::fixed
      << std::setprecision(3) << stats.services_stats().min_state_change()
      << " / " << stats.services_stats().max_state_change() << " / "
      << stats.services_stats().average_state_change() << " %"
      << "\nActive Service Latency:                 "
      << stats.services_stats().active_services().min_latency() << " / "
      << stats.services_stats().active_services().max_latency() << " / "
      << stats.services_stats().active_services().average_latency() << " sec\n"
      << "\nActive Service Execution Time:          "
      << stats.services_stats().active_services().min_execution_time() << " / "
      << stats.services_stats().active_services().max_execution_time() << " / "
      << stats.services_stats().active_services().average_execution_time()
      << " sec"
      << "\nActive Service State Change:            "
      << stats.services_stats().active_services().min_state_change() << " / "
      << stats.services_stats().active_services().max_state_change() << " / "
      << stats.services_stats().active_services().average_state_change() << " %"
      << "\nActive Services Last 1/5/15/60 min:     "
      << stats.services_stats().active_services().checks_last_1min() << " / "
      << stats.services_stats().active_services().checks_last_5min() << " / "
      << stats.services_stats().active_services().checks_last_15min() << " / "
      << stats.services_stats().active_services().checks_last_1hour()
      << "\nPassive Service Latency:                 "
      << stats.services_stats().passive_services().min_latency() << " / "
      << stats.services_stats().passive_services().max_latency() << " / "
      << stats.services_stats().passive_services().average_latency() << " sec\n"
      << "\nPassive Service State Change:            "
      << stats.services_stats().passive_services().min_state_change() << " / "
      << stats.services_stats().passive_services().max_state_change() << " / "
      << stats.services_stats().passive_services().average_state_change()
      << " %"
      << "\nPassive Services Last 1/5/15/60 min:     "
      << stats.services_stats().passive_services().checks_last_1min() << " / "
      << stats.services_stats().passive_services().checks_last_5min() << " / "
      << stats.services_stats().passive_services().checks_last_15min() << " / "
      << stats.services_stats().passive_services().checks_last_1hour()
      << "\nServices OK/Warn/Unk/Crit:               "
      << stats.services_stats().ok() << " / "
      << stats.services_stats().warning() << " / "
      << stats.services_stats().unknown() << " / "
      << stats.services_stats().critical()
      << "\nServices Flapping:                       "
      << stats.services_stats().flapping()
      << "\nServices In Downtime:                    "
      << stats.services_stats().downtime() << std::endl;
}

bool centenginestats_client::GetStats(Stats* stats) {
  const ::google::protobuf::Empty e;
  grpc::ClientContext context;
  grpc::Status status = _stub->GetStats(&context, e, stats);
  if (!status.ok()) {
    std::cout << "GetStats rpc failed." << std::endl;
    return false;
  }
  return true;
}

bool centenginestats_client::ProcessServiceCheckResult(Check const& sc) {
  grpc::ClientContext context;
  CommandSuccess response;
  grpc::Status status =
      _stub->ProcessServiceCheckResult(&context, sc, &response);
  if (!status.ok()) {
    std::cout << "ProcessServiceCheckResult failed." << std::endl;
    return false;
  }
  return true;
}

bool centenginestats_client::ProcessHostCheckResult(Check const& hc) {
  grpc::ClientContext context;
  CommandSuccess response;
  grpc::Status status = _stub->ProcessHostCheckResult(&context, hc, &response);
  if (!status.ok()) {
    std::cout << "ProcessHostCheckResult failed." << std::endl;
    return false;
  }
  return true;
}

bool centenginestats_client::NewThresholdsFile(const ThresholdsFile& tf) {
  grpc::ClientContext context;
  CommandSuccess response;
  grpc::Status status = _stub->NewThresholdsFile(&context, tf, &response);
  if (!status.ok()) {
    std::cout << "NewThresholdsFile failed." << std::endl;
    return false;
  }
  return true;
}
