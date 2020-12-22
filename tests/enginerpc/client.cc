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

using namespace com::centreon::engine;

class EngineRPCClient {
  std::unique_ptr<Engine::Stub> _stub;

 public:
  EngineRPCClient(std::shared_ptr<grpc::Channel> channel)
      : _stub(Engine::NewStub(channel)) {}

  bool GetVersion(Version* version) {
    const ::google::protobuf::Empty e;
    grpc::ClientContext context;
    grpc::Status status = _stub->GetVersion(&context, e, version);
    if (!status.ok()) {
      std::cout << "GetVersion rpc failed." << std::endl;
      return false;
    }
    return true;
  }

  bool GetStats(Stats* stats) {
    GenericString e;
    grpc::ClientContext context;
    grpc::Status status = _stub->GetStats(&context, e, stats);
    if (!status.ok()) {
      std::cout << "GetStats rpc failed." << std::endl;
      return false;
    }
    return true;
  }

  bool GetHostByHostName(std::string const& req, EngineHost* response) {
    HostIdentifier request;
    grpc::ClientContext context;
    request.set_name(req);

    grpc::Status status = _stub->GetHost(&context, request, response);

    if (!status.ok()) {
      std::cout << "GetHostByHostName rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetHostByHostId(uint32_t& req, EngineHost* response) {
    HostIdentifier request;
    grpc::ClientContext context;
    request.set_id(req);

    grpc::Status status = _stub->GetHost(&context, request, response);

    if (!status.ok()) {
      std::cout << "GetHostByHostId rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetContact(std::string const& req, EngineContact* response) {
    ContactIdentifier request;
    grpc::ClientContext context;
    request.set_name(req);

    grpc::Status status = _stub->GetContact(&context, request, response);

    if (!status.ok()) {
      std::cout << "GetContact rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetServiceByNames(std::string const& hostname,
                         std::string const& servicename,
                         EngineService* response) {
    ServiceIdentifier request;
    grpc::ClientContext context;
    request.mutable_names()->set_host_name(hostname);
    request.mutable_names()->set_service_name(servicename);

    grpc::Status status = _stub->GetService(&context, request, response);

    if (!status.ok()) {
      std::cout << "GetService rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetServiceByIds(uint32_t& hostid,
                       uint32_t serviceid,
                       EngineService* response) {
    ServiceIdentifier request;
    grpc::ClientContext context;
    request.mutable_ids()->set_host_id(hostid);
    request.mutable_ids()->set_service_id(serviceid);

    grpc::Status status = _stub->GetService(&context, request, response);

    if (!status.ok()) {
      std::cout << "GetService rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetHostsCount(GenericValue* response) {
    const ::google::protobuf::Empty e;
    grpc::ClientContext context;

    grpc::Status status = _stub->GetHostsCount(&context, e, response);

    if (!status.ok()) {
      std::cout << "GetHostsCount rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetContactsCount(GenericValue* response) {
    const ::google::protobuf::Empty e;
    grpc::ClientContext context;

    grpc::Status status = _stub->GetContactsCount(&context, e, response);

    if (!status.ok()) {
      std::cout << "GetContactsCount rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetServicesCount(GenericValue* response) {
    const ::google::protobuf::Empty e;
    grpc::ClientContext context;

    grpc::Status status = _stub->GetServicesCount(&context, e, response);

    if (!status.ok()) {
      std::cout << "GetServicesCount rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetServiceGroupsCount(GenericValue* response) {
    const ::google::protobuf::Empty e;
    grpc::ClientContext context;

    grpc::Status status = _stub->GetServiceGroupsCount(&context, e, response);

    if (!status.ok()) {
      std::cout << "GetServiceGroupsCount rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetContactGroupsCount(GenericValue* response) {
    const ::google::protobuf::Empty e;
    grpc::ClientContext context;

    grpc::Status status = _stub->GetContactGroupsCount(&context, e, response);

    if (!status.ok()) {
      std::cout << "GetContactGroupsCount rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetHostGroupsCount(GenericValue* response) {
    const ::google::protobuf::Empty e;
    grpc::ClientContext context;

    grpc::Status status = _stub->GetHostGroupsCount(&context, e, response);

    if (!status.ok()) {
      std::cout << "GetHostGroupsCount rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetServiceDependenciesCount(GenericValue* response) {
    const ::google::protobuf::Empty e;
    grpc::ClientContext context;

    grpc::Status status =
        _stub->GetServiceDependenciesCount(&context, e, response);

    if (!status.ok()) {
      std::cout << "GetServiceDependenciesCount engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool GetHostDependenciesCount(GenericValue* response) {
    const ::google::protobuf::Empty e;
    grpc::ClientContext context;

    grpc::Status status =
        _stub->GetHostDependenciesCount(&context, e, response);

    if (!status.ok()) {
      std::cout << "GetHostDependenciesCount engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool ProcessServiceCheckResult(Check const& sc) {
    grpc::ClientContext context;
    CommandSuccess response;
    grpc::Status status = _stub->ProcessServiceCheckResult(&context, sc, &response);
    if (!status.ok()) {
      std::cout << "ProcessServiceCheckResult failed." << std::endl;
      return false;
    }
    return true;
  }

  bool ProcessHostCheckResult(Check const& hc) {
    grpc::ClientContext context;
    CommandSuccess response;
    grpc::Status status = _stub->ProcessHostCheckResult(&context, hc, &response);
    if (!status.ok()) {
      std::cout << "ProcessHostCheckResult failed." << std::endl;
      return false;
    }
    return true;
  }

  bool NewThresholdsFile(const ThresholdsFile& tf) {
    grpc::ClientContext context;
    CommandSuccess response;
    grpc::Status status = _stub->NewThresholdsFile(&context, tf, &response);
    if (!status.ok()) {
      std::cout << "NewThresholdsFile failed." << std::endl;
      return false;
    }
    return true;
  }
};

int main(int argc, char** argv) {
  int32_t status;
  EngineRPCClient client(grpc::CreateChannel(
      "127.0.0.1:40001", grpc::InsecureChannelCredentials()));

  if (argc < 2) {
    std::cout << "ERROR: this client must be called with a command..."
              << std::endl;
    exit(1);
  }

  if (strcmp(argv[1], "GetVersion") == 0) {
    Version version;
    status = client.GetVersion(&version) ? 0 : 1;
    std::cout << "GetVersion: " << version.DebugString();
  }
  else if (strcmp(argv[1], "GetStats") == 0) {
    Stats stats;
    status = client.GetStats(&stats) ? 0 : 2;
    std::cout << "GetStats: " << stats.DebugString();
  }
  else if (strcmp(argv[1], "ProcessServiceCheckResult") == 0) {
    Check sc;
    sc.set_host_name(argv[2]);
    sc.set_svc_desc(argv[3]);
    sc.set_code(std::stol(argv[4]));
    sc.set_output("Test external command");
    status = client.ProcessServiceCheckResult(sc) ? 0 : 3;
    std::cout << "ProcessServiceCheckResult: " << status << std::endl;
  }
  else if (strcmp(argv[1], "ProcessHostCheckResult") == 0) {
    Check hc;
    hc.set_host_name(argv[2]);
    hc.set_code(std::stol(argv[3]));
    hc.set_output("Test external command");
    status = client.ProcessHostCheckResult(hc) ? 0 : 4;
    std::cout << "ProcessHostCheckResult: " << status << std::endl;
  }
  else if (strcmp(argv[1], "NewThresholdsFile") == 0) {
    ThresholdsFile tf;
    tf.set_filename(argv[2]);
    status = client.NewThresholdsFile(tf) ? 0 : 5;
    std::cout << "NewThresholdsFile: " << status << std::endl;
  }
  else if (strcmp(argv[1], "GetHost") == 0) {
    if (argc != 4) {
      std::cout << "GetHost require arguments : GetHost [mode] [hostname or id]"
                << std::endl;
      return 1;
    } 
    else if (strcmp(argv[2], "byhostid") == 0) {
      EngineHost response;
      uint32_t val = atoi(argv[3]);
      status = client.GetHostByHostId(val, &response) ? 0 : 1;
      std::cout << "GetHost" << std::endl;
      std::cout << "Host name: " << response.name() << std::endl;
      std::cout << "Host alias: " << response.alias() << std::endl;
      std::cout << "Host id: " << response.id() << std::endl;
      std::cout << "Host address: " << response.address() << std::endl;
      std::cout << "Host state: " << response.current_state() << std::endl;
      std::cout << "Host period: " << response.check_period() << std::endl;
    } 
    else if (strcmp(argv[2], "byhostname") == 0) {
      EngineHost response;
      std::string str(argv[3]);
      status = client.GetHostByHostName(str, &response) ? 0 : 1;
      std::cout << "GetHost" << std::endl;
      std::cout << "Host name: " << response.name() << std::endl;
      std::cout << "Host alias: " << response.alias() << std::endl;
      std::cout << "Host id: " << response.id() << std::endl;
      std::cout << "Host address: " << response.address() << std::endl;
      std::cout << "Host state: " << response.current_state() << std::endl;
      std::cout << "Host period: " << response.check_period() << std::endl;

    } 
  }
  else if (strcmp(argv[1], "GetContact") == 0) {
      if (argc != 3) {
        std::cout << "GetContact require arguments : GetContact [contactname]"
                  << std::endl;
        return 1;
      }
      EngineContact response;
      std::string str = (argv[2]);
      status = client.GetContact(str, &response) ? 0 : 1;
      std::cout << "GetContact" << std::endl;
      std::cout << response.name() << std::endl;
      std::cout << response.alias() << std::endl;
      std::cout << response.email() << std::endl;
  } 
  else if (strcmp(argv[1], "GetService") == 0) {
    if (argc != 5) {
      std::cout << "GetService require arguments : GetService [mode] [hostname "
                   "or hostid] [servicename or serviceid]"
                << std::endl;
      return 1;
    } else if (strcmp(argv[2], "bynames") == 0) {
      EngineService response;
      std::string hostname(argv[3]);
      std::string servicename(argv[4]);
      status =
          client.GetServiceByNames(hostname, servicename, &response) ? 0 : 1;
      std::cout << "GetService" << std::endl;
      std::cout << "Host id: " << response.host_id() << std::endl;
      std::cout << "Service id: " << response.service_id() << std::endl;
      std::cout << "Host name: " << response.host_name() << std::endl;
      std::cout << "Serv desc: " << response.description() << std::endl;
      std::cout << "Service state: " << response.current_state() << std::endl;
      std::cout << "Service period: " << response.check_period() << std::endl;
    } else if (strcmp(argv[2], "byids") == 0) {
      EngineService response;
      uint32_t hostid = atoi(argv[3]);
      uint32_t serviceid = atoi(argv[4]);
      status = client.GetServiceByIds(hostid, serviceid, &response) ? 0 : 1;
      std::cout << "GetService" << std::endl;
      std::cout << "Host id: " << response.host_id() << std::endl;
      std::cout << "Service id: " << response.service_id() << std::endl;
      std::cout << "Host name: " << response.host_name() << std::endl;
      std::cout << "Serv desc: " << response.description() << std::endl;
      std::cout << "Service state: " << response.current_state() << std::endl;
      std::cout << "Service period: " << response.check_period() << std::endl;
    }
  } 

  else if (strcmp(argv[1], "GetHostsCount") == 0) {
    GenericValue response;
    status = client.GetHostsCount(&response) ? 0 : 1;
    std::cout << "GetHostsCount from client" << std::endl;
    std::cout << response.value() << std::endl;
  } else if (strcmp(argv[1], "GetContactsCount") == 0) {
    GenericValue response;
    status = client.GetContactsCount(&response) ? 0 : 1;
    std::cout << "GetContactsCount from client" << std::endl;
    std::cout << response.value() << std::endl;
  } else if (strcmp(argv[1], "GetServicesCount") == 0) {
    GenericValue response;
    status = client.GetServicesCount(&response) ? 0 : 1;
    std::cout << "GetServicesCount from client" << std::endl;
    std::cout << response.value() << std::endl;
  } else if (strcmp(argv[1], "GetServiceGroupsCount") == 0) {
    GenericValue response;
    status = client.GetServiceGroupsCount(&response) ? 0 : 1;
    std::cout << "GetServiceGroupsCount from client" << std::endl;
    std::cout << response.value() << std::endl;
  } else if (strcmp(argv[1], "GetContactGroupsCount") == 0) {
    GenericValue response;
    status = client.GetContactGroupsCount(&response) ? 0 : 1;
    std::cout << "GetContactGroupsCount from client" << std::endl;
    std::cout << response.value() << std::endl;
  } else if (strcmp(argv[1], "GetHostGroupsCount") == 0) {
    GenericValue response;
    status = client.GetHostGroupsCount(&response) ? 0 : 1;
    std::cout << "GetHostGroupsCount from client" << std::endl;
    std::cout << response.value() << std::endl;
  } else if (strcmp(argv[1], "GetServiceDependenciesCount") == 0) {
    GenericValue response;
    status = client.GetServiceDependenciesCount(&response) ? 0 : 1;
    std::cout << "GetServiceDependenciesCount client" << std::endl;
    std::cout << response.value() << std::endl;
  } else if (strcmp(argv[1], "GetHostDependenciesCount") == 0) {
    GenericValue response;
    status = client.GetHostDependenciesCount(&response) ? 0 : 1;
    std::cout << "GetHostDependenciesCount client" << std::endl;
    std::cout << response.value() << std::endl;
  } 

  exit(status);
}
