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

/**
 * @brief The main function of this program. It get statistics from centengine.
 * The subject is given by the \a object parameter.
 *
 * @param object A string.
 */
void centenginestats_client::get_stats(std::string const& object) {
  time_t current_time;
  time(&current_time);
  uint32_t time_difference = current_time - _status_creation_date;
  grpc::ClientContext context;
  Stats stats;
  Object obj;
  *obj.mutable_name() = object;
  grpc::Status status = _stub->GetStats(&context, obj, &stats);
  if (!status.ok())
    throw std::invalid_argument(
        "Failed to get the centengine statistics. It is probably not running.");

  if (object == "default") {
    uint32_t program_start =
        TimeUtil::TimestampToSeconds(stats.program_status().program_start());
    uint32_t program_age = current_time - program_start;

    printf(
        "CURRENT STATUS DATA\n"
        "---------------------------------------------------------------"
        "-----------------\n");

    printf("Status File:                            %s\n", _stats_file.c_str());
    printf("Status File Age:                        %s\n\n",
           duration_to_str(time_difference).c_str());
    printf("Program Running Time:                   %s\n",
           duration_to_str(program_age).c_str());
    printf("Centreon Engine PID:                    %ud\n",
           stats.program_status().pid());
    printf("Used/High/Total command Buffers:        %d / %d / %d\n\n",
           stats.program_status().used_external_command_buffer_slots(),
           stats.program_status().high_external_command_buffer_slots(),
           stats.program_status().total_external_command_buffer_slots());
    printf("Total Services:                         %ud\n",
           stats.services_stats().services_count());
    printf("Services Checked:                       %ud\n",
           stats.services_stats().checked_services());
    printf("Services Scheduled:                     %ud\n",
           stats.services_stats().scheduled_services());
    printf("Services Actively Checked:              %ud\n",
           stats.services_stats().actively_checked());
    printf("Services Passively Checked:             %ud\n",
           stats.services_stats().passively_checked());
    printf("Total Service State Change:             %.3f / %.3f / %.3f %%\n",
           stats.services_stats().min_state_change(),
           stats.services_stats().max_state_change(),
           stats.services_stats().average_state_change());
    printf("Active Service Latency:                 %.3f / %.3f / %.3f sec\n",
           stats.services_stats().active_services().min_latency(),
           stats.services_stats().active_services().max_latency(),
           stats.services_stats().active_services().average_latency());
    printf("Active Service Execution Time:          %.3f / %.3f / %.3f sec\n",
           stats.services_stats().active_services().min_execution_time(),
           stats.services_stats().active_services().max_execution_time(),
           stats.services_stats().active_services().average_execution_time());
    printf("Active Service State Change:            %.3f / %.3f / %.3f %%\n",
           stats.services_stats().active_services().min_state_change(),
           stats.services_stats().active_services().max_state_change(),
           stats.services_stats().active_services().average_state_change());
    printf("Active Services Last 1/5/15/60 min:     %ud / %ud / %ud / %ud\n",
           stats.services_stats().active_services().checks_last_1min(),
           stats.services_stats().active_services().checks_last_5min(),
           stats.services_stats().active_services().checks_last_15min(),
           stats.services_stats().active_services().checks_last_1hour());
    printf("Passive Service Latency:                %.3f / %.3f / %.3f sec\n",
           stats.services_stats().passive_services().min_latency(),
           stats.services_stats().passive_services().max_latency(),
           stats.services_stats().passive_services().average_latency());
    printf("Passive Service State Change:           %.3f / %.3f / %.3f %%\n",
           stats.services_stats().passive_services().min_state_change(),
           stats.services_stats().passive_services().max_state_change(),
           stats.services_stats().passive_services().average_state_change());
    printf("Passive Services Last 1/5/15/60 min:    %ud / %ud / %ud / %ud\n",
           stats.services_stats().passive_services().checks_last_1min(),
           stats.services_stats().passive_services().checks_last_5min(),
           stats.services_stats().passive_services().checks_last_15min(),
           stats.services_stats().passive_services().checks_last_1hour());
    printf("Services OK/Warn/Unk/Crit:              %ud / %ud / %ud / %ud\n",
           stats.services_stats().ok(), stats.services_stats().warning(),
           stats.services_stats().unknown(), stats.services_stats().critical());
    printf("Services Flapping:                      %ud\n",
           stats.services_stats().flapping());
    printf("Services In Downtime:                   %ud\n\n",
           stats.services_stats().downtime());
    printf("Total Hosts:                            %ud\n",
           stats.hosts_stats().hosts_count());
    printf("Hosts Checked:                          %ud\n",
           stats.hosts_stats().checked_hosts());
    printf("Hosts Scheduled:                        %ud\n",
           stats.hosts_stats().scheduled_hosts());
    printf("Hosts Actively Checked:                 %ud\n",
           stats.hosts_stats().actively_checked());
    printf("Hosts Passively Checked:                %ud\n",
           stats.hosts_stats().passively_checked());
    printf("Total Host State Change:                %.3f / %.3f / %.3f %%\n",
           stats.hosts_stats().min_state_change(),
           stats.hosts_stats().max_state_change(),
           stats.hosts_stats().average_state_change());
    printf("Active Host Latency:                    %.3f / %.3f / %.3f sec\n",
           stats.hosts_stats().active_hosts().min_latency(),
           stats.hosts_stats().active_hosts().max_latency(),
           stats.hosts_stats().active_hosts().average_latency());
    printf("Active Host Execution Time:             %.3f / %.3f / %.3f sec\n",
           stats.hosts_stats().active_hosts().min_execution_time(),
           stats.hosts_stats().active_hosts().max_execution_time(),
           stats.hosts_stats().active_hosts().average_execution_time());
    printf("Active Host State Change:               %.3f / %.3f / %.3f %%\n",
           stats.hosts_stats().active_hosts().min_state_change(),
           stats.hosts_stats().active_hosts().max_state_change(),
           stats.hosts_stats().active_hosts().average_state_change());
    printf("Active Hosts Last 1/5/15/60 min:        %ud / %ud / %ud / %ud\n",
           stats.hosts_stats().active_hosts().checks_last_1min(),
           stats.hosts_stats().active_hosts().checks_last_5min(),
           stats.hosts_stats().active_hosts().checks_last_15min(),
           stats.hosts_stats().active_hosts().checks_last_1hour());
    printf("Passive Host Latency:                   %.3f / %.3f / %.3f sec\n",
           stats.hosts_stats().passive_hosts().min_latency(),
           stats.hosts_stats().passive_hosts().max_latency(),
           stats.hosts_stats().passive_hosts().average_latency());
    printf("Passive Host State Change:              %.3f / %.3f / %.3f %%\n",
           stats.hosts_stats().passive_hosts().min_state_change(),
           stats.hosts_stats().passive_hosts().max_state_change(),
           stats.hosts_stats().passive_hosts().average_state_change());
    printf("Passive Hosts Last 1/5/15/60 min:       %ud / %ud / %ud / %ud\n",
           stats.hosts_stats().passive_hosts().checks_last_1min(),
           stats.hosts_stats().passive_hosts().checks_last_5min(),
           stats.hosts_stats().passive_hosts().checks_last_15min(),
           stats.hosts_stats().passive_hosts().checks_last_1hour());
    printf("Hosts Up/Down/Unreach:                  %ud / %ud / %ud\n",
           stats.hosts_stats().up(), stats.hosts_stats().down(),
           stats.hosts_stats().unreachable());
    printf("Hosts Flapping:                         %ud\n",
           stats.hosts_stats().flapping());
    printf("Hosts In Downtime:                      %ud\n\n",
           stats.hosts_stats().downtime());
    printf("Active Host Checks Last 1/5/15 min:     %ud / %ud / %ud\n",
           stats.program_status().active_scheduled_host_check_stats()[0] +
               stats.program_status().active_ondemand_host_check_stats()[0],
           stats.program_status().active_scheduled_host_check_stats()[1] +
               stats.program_status().active_ondemand_host_check_stats()[1],
           stats.program_status().active_scheduled_host_check_stats()[2] +
               stats.program_status().active_ondemand_host_check_stats()[2]);
    printf("   Scheduled:                           %ud / %ud / %ud\n",
           stats.program_status().active_scheduled_host_check_stats()[0],
           stats.program_status().active_scheduled_host_check_stats()[1],
           stats.program_status().active_scheduled_host_check_stats()[2]);
    printf("   On-demand:                           %ud / %ud / %ud\n",
           stats.program_status().active_ondemand_host_check_stats()[0],
           stats.program_status().active_ondemand_host_check_stats()[1],
           stats.program_status().active_ondemand_host_check_stats()[2]);
    printf("   Parallel:                            %ud / %ud / %ud\n",
           stats.program_status().parallel_host_check_stats()[0],
           stats.program_status().parallel_host_check_stats()[1],
           stats.program_status().parallel_host_check_stats()[2]);
    printf("   Serial:                              %ud / %ud / %ud\n",
           stats.program_status().serial_host_check_stats()[0],
           stats.program_status().serial_host_check_stats()[1],
           stats.program_status().serial_host_check_stats()[2]);
    printf("   Cached:                              %ud / %ud / %ud\n",
           stats.program_status().cached_host_check_stats()[0],
           stats.program_status().cached_host_check_stats()[1],
           stats.program_status().cached_host_check_stats()[2]);
    printf("Passive Host Checks Last 1/5/15 min:    %ud / %ud / %ud\n",
           stats.program_status().passive_host_check_stats()[0],
           stats.program_status().passive_host_check_stats()[1],
           stats.program_status().passive_host_check_stats()[2]);
    printf("Active Service Checks Last 1/5/15 min:  %ud / %ud / %ud\n",
           stats.program_status().active_scheduled_service_check_stats()[0] +
               stats.program_status().active_ondemand_service_check_stats()[0],
           stats.program_status().active_scheduled_service_check_stats()[1] +
               stats.program_status().active_ondemand_service_check_stats()[1],
           stats.program_status().active_scheduled_service_check_stats()[2] +
               stats.program_status().active_ondemand_service_check_stats()[2]);
    printf("   Scheduled:                           %ud / %ud / %ud\n",
           stats.program_status().active_scheduled_service_check_stats()[0],
           stats.program_status().active_scheduled_service_check_stats()[1],
           stats.program_status().active_scheduled_service_check_stats()[2]);
    printf("   On-demand:                           %ud / %ud / %ud\n",
           stats.program_status().active_ondemand_service_check_stats()[0],
           stats.program_status().active_ondemand_service_check_stats()[1],
           stats.program_status().active_ondemand_service_check_stats()[2]);
    printf("   Cached:                              %ud / %ud / %ud\n",
           stats.program_status().cached_service_check_stats()[0],
           stats.program_status().cached_service_check_stats()[1],
           stats.program_status().cached_service_check_stats()[2]);
    printf("Passive Service Checks Last 1/5/15 min: %ud / %ud / %ud\n\n",
           stats.program_status().passive_service_check_stats()[0],
           stats.program_status().passive_service_check_stats()[1],
           stats.program_status().passive_service_check_stats()[2]);
    printf("External Commands Last 1/5/15 min:      %ud / %ud / %ud\n\n\n",
           stats.program_status().external_command_stats()[0],
           stats.program_status().external_command_stats()[1],
           stats.program_status().external_command_stats()[2]);
  } else if (object == "start") {
    printf(
        "RESTART STATUS DATA\n"
        "---------------------------------------------------------------"
        "-----------------\n");
    printf("Apply start                             %s\n",
           TimeUtil::ToString(stats.restart_status().apply_start()).c_str());
    printf("   Objects expansion                    %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().objects_expansion())) /
               1000.);

    printf("   Objects differences                  %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().objects_difference())) /
               1000.);
    printf("   Global conf application              %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_config())) /
               1000.);
    printf("   Timeperiods application              %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_timeperiods())) /
               1000.);
    printf("   Connectors application               %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_connectors())) /
               1000.);
    printf("   Commands application                 %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_commands())) /
               1000.);
    printf("   Contacts application                 %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_contacts())) /
               1000.);
    printf("   Hosts application                    %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_hosts())) /
               1000.);
    printf("   Services application                 %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_services())) /
               1000.);
    printf("   Hosts resolution                     %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().resolve_hosts())) /
               1000.);
    printf("   Services resolution                  %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().resolve_services())) /
               1000.);
    printf("   Host dependencies application        %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_host_dependencies())) /
               1000.);
    printf("   Host dependencies resolution         %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().resolve_host_dependencies())) /
               1000.);
    printf("   Service dependencies application     %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_service_dependencies())) /
               1000.);
    printf("   Service dependencies resolution      %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().resolve_service_dependencies())) /
               1000.);
    printf("   Host escalations application         %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_host_escalations())) /
               1000.);
    printf("   Host escalations resolution          %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().resolve_host_escalations())) /
               1000.);
    printf("   Service escalations application      %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_service_escalations())) /
               1000.);
    printf("   Service escalations resolution       %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().resolve_service_escalations())) /
               1000.);
    printf("   New configuration application        %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_new_config())) /
               1000.);
    printf("   Scheduler configuration application  %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().apply_scheduler())) /
               1000.);
    printf("   Check circular paths                 %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().check_circular_paths())) /
               1000.);
    printf("   Modules reload                       %.3f s\n",
           static_cast<double>(TimeUtil::DurationToMilliseconds(
               stats.restart_status().reload_modules())) /
               1000.);

    printf("Apply end                               %s\n",
           TimeUtil::ToString(stats.restart_status().apply_end()).c_str());
  }
}
