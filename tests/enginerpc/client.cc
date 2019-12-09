/*
 * Copyright 2019 Centreon (https://www.centreon.com/)
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

#include <iostream>
#include <memory>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include "engine.grpc.pb.h"

using namespace grpc;

class EngineRPCClient {
  std::unique_ptr<Engine::Stub> _stub;

 public:
  EngineRPCClient(std::shared_ptr<Channel> channel)
      : _stub(Engine::NewStub(channel)) {}

  bool GetVersion(Version* version) {
    const ::google::protobuf::Empty e;
    ClientContext context;
    Status status = _stub->GetVersion(&context, e, version);
    if (!status.ok()) {
      std::cout << "GetVersion rpc failed." << std::endl;
      return false;
    }
    return true;
  }
};

int main(int argc, char** argv) {
  EngineRPCClient client(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

  if (argc != 2) {
    std::cout << "ERROR: this client must be called with a command..."
              << std::endl;
    exit(1);
  }

  if (strcmp(argv[1], "GetVersion") == 0) {
    Version version;
    client.GetVersion(&version);
    std::cout << "GetVersion: " << version.major() << " " << version.minor()
              << " " << version.patch() << std::endl;
  }
  exit(0);
}