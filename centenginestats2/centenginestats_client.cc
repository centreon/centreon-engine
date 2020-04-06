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

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <stdint.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "enginerpc/engine.grpc.pb.h"

using namespace com::centreon::engine;

centenginestats_client::centenginestats_client(uint16_t grpc_port,
                                               std::string const& config_file,
                                               std::string const& stats_file)
    : _config_file(config_file),
      _stats_file(stats_file),
      _status_creation_date(0),
      _grpc_port(grpc_port) {
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
      if (strncmp(line.c_str(), "created", 7) == 0) {
        _status_creation_date = strtoul(line.c_str() + 8, nullptr, 10);
        count--;
      } else if (_grpc_port == 0 &&
                 strncmp(line.c_str(), "grpc_port", 9) == 0) {
        _grpc_port = strtoul(line.c_str() + 10, nullptr, 10);
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
