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
#ifndef CCE_CENTENGINESTATS_CLIENT_HH
#define CCE_CENTENGINESTATS_CLIENT_HH

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

#include <memory>

#include "com/centreon/engine/namespace.hh"
#include "enginerpc/engine.grpc.pb.h"

CCE_BEGIN()
class centenginestats_client {
  std::string _config_file;
  std::string _stats_file;
  std::unique_ptr<Engine::Stub> _stub;
  time_t _status_creation_date;
  uint16_t _grpc_port;

 public:
  centenginestats_client() = delete;
  centenginestats_client(centenginestats_client const&) = delete;
  centenginestats_client& operator=(centenginestats_client const&) = delete;
  centenginestats_client(uint16_t grpc_port,
                         std::string const& config_file,
                         std::string const& stats_file);
  ~centenginestats_client() = default;
  bool is_configured() const;
  std::string get_version();

  bool GetStats(Stats* stats);
  bool ProcessServiceCheckResult(Check const& sc);
  bool ProcessHostCheckResult(Check const& hc);
  bool NewThresholdsFile(const ThresholdsFile& tf);
  int32_t read_stats_file();
};

CCE_END()

#endif /* !CCE_CENTENGINESTATS_CLIENT_HH */
