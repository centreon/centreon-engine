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

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <iostream>
#include <memory>
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
    grpc::Status status =
        _stub->ProcessServiceCheckResult(&context, sc, &response);
    if (!status.ok()) {
      std::cout << "ProcessServiceCheckResult failed." << std::endl;
      return false;
    }
    return true;
  }

  bool ProcessHostCheckResult(Check const& hc) {
    grpc::ClientContext context;
    CommandSuccess response;
    grpc::Status status =
        _stub->ProcessHostCheckResult(&context, hc, &response);
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

  bool AddHostComment(std::string const& hostname,
                      uint32_t& entrytime,
                      std::string const& user,
                      std::string const& commentdata,
                      bool& persistent,
                      CommandSuccess* response) {
    grpc::ClientContext context;
    EngineComment request;
    request.set_host_name(hostname);
    request.set_entry_time(entrytime);
    request.set_user(user);
    request.set_comment_data(commentdata);
    request.set_persistent(persistent);

    grpc::Status status = _stub->AddHostComment(&context, request, response);
    if (!status.ok()) {
      std::cout << "AddHostComment failed." << std::endl;
      return false;
    }
    return true;
  }

  bool AddServiceComment(std::string const& hostname,
                         std::string svcdsc,
                         uint32_t& entrytime,
                         std::string const& user,
                         std::string const& commentdata,
                         bool& persistent,
                         CommandSuccess* response) {
    grpc::ClientContext context;
    EngineComment request;
    request.set_host_name(hostname);
    request.set_svc_desc(svcdsc);
    request.set_entry_time(entrytime);
    request.set_user(user);
    request.set_comment_data(commentdata);
    request.set_persistent(persistent);

    grpc::Status status = _stub->AddServiceComment(&context, request, response);
    if (!status.ok()) {
      std::cout << "AddHostComment failed." << std::endl;
      return false;
    }
    return true;
  }

  bool DeleteComment(uint32_t& req, CommandSuccess* response) {
    GenericValue request;
    grpc::ClientContext context;
    request.set_value(req);

    grpc::Status status = _stub->DeleteComment(&context, request, response);
    if (!status.ok() || !response->value()) {
      std::cout << "DeleteComment failed." << std::endl;
      return false;
    }
    return true;
  }

  bool DeleteAllHostCommentsByName(std::string const& req,
                                   CommandSuccess* response) {
    HostIdentifier request;
    grpc::ClientContext context;
    request.set_name(req);

    grpc::Status status =
        _stub->DeleteAllHostComments(&context, request, response);
    if (!status.ok()) {
      std::cout << "DeleteAllHostCommentsByNames failed." << std::endl;
      return false;
    }
    return true;
  }

  bool DeleteAllHostCommentsById(uint32_t& req, CommandSuccess* response) {
    HostIdentifier request;
    grpc::ClientContext context;
    request.set_id(req);

    grpc::Status status =
        _stub->DeleteAllHostComments(&context, request, response);
    if (!status.ok()) {
      std::cout << "DeleteAllHostCommentsByName failed." << std::endl;
      return false;
    }
    return true;
  }

  bool DeleteAllServiceCommentsByNames(std::string const& hostname,
                                       std::string const& svcdsc,
                                       CommandSuccess* response) {
    ServiceIdentifier request;
    grpc::ClientContext context;
    request.mutable_names()->set_host_name(hostname);
    request.mutable_names()->set_service_name(svcdsc);

    grpc::Status status =
        _stub->DeleteAllServiceComments(&context, request, response);
    if (!status.ok()) {
      std::cout << "DeleteAllServiceCommentsByNames rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool DeleteAllServiceCommentsByIds(uint32_t& hostid,
                                     uint32_t& serviceid,
                                     CommandSuccess* response) {
    ServiceIdentifier request;
    grpc::ClientContext context;
    request.mutable_ids()->set_host_id(hostid);
    request.mutable_ids()->set_service_id(serviceid);

    grpc::Status status =
        _stub->DeleteAllServiceComments(&context, request, response);
    if (!status.ok()) {
      std::cout << "DeleteAllServiceCommentsByIds rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool RemoveHostAcknowledgementByNames(std::string const& hostname,
                                        CommandSuccess* response) {
    HostIdentifier request;
    grpc::ClientContext context;
    request.set_name(hostname);
    grpc::Status status =
        _stub->RemoveHostAcknowledgement(&context, request, response);
    if (!status.ok()) {
      std::cout << "RemoveHostAcknowledgementByNames failed." << std::endl;
      return false;
    }
    return true;
  }

  bool RemoveHostAcknowledgementByIds(uint32_t& hostid,
                                      CommandSuccess* response) {
    HostIdentifier request;
    grpc::ClientContext context;
    request.set_id(hostid);

    grpc::Status status =
        _stub->RemoveHostAcknowledgement(&context, request, response);
    if (!status.ok()) {
      std::cout << "RemoveHostAcknowledgementByIds failed." << std::endl;
      return false;
    }
    return true;
  }

  bool RemoveServiceAcknowledgementByNames(std::string const& hostname,
                                           std::string const& svcdsc,
                                           CommandSuccess* response) {
    ServiceIdentifier request;
    grpc::ClientContext context;
    request.mutable_names()->set_host_name(hostname);
    request.mutable_names()->set_service_name(svcdsc);

    grpc::Status status =
        _stub->RemoveServiceAcknowledgement(&context, request, response);
    if (!status.ok()) {
      std::cout << "RemoveServiceAcknowledgementByNames rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool RemoveServiceAcknowledgementByIds(uint32_t& hostid,
                                         uint32_t& serviceid,
                                         CommandSuccess* response) {
    ServiceIdentifier request;
    grpc::ClientContext context;
    request.mutable_ids()->set_host_id(hostid);
    request.mutable_ids()->set_service_id(serviceid);

    grpc::Status status =
        _stub->RemoveServiceAcknowledgement(&context, request, response);
    if (!status.ok()) {
      std::cout << "RemoveHostAcknowledgementByIds rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool AcknowledgementHostProblem(std::string const& hostname,
                                  std::string const& ackauthor,
                                  std::string const& ackdata,
                                  int type,
                                  bool notify,
                                  bool persistent,
                                  CommandSuccess* response) {
    EngineAcknowledgement request;
    grpc::ClientContext context;
    request.set_host_name(hostname);
    request.set_ack_author(ackauthor);
    request.set_ack_data(ackdata);
    request.set_type(static_cast<EngineAcknowledgement::Type>(type));
    request.set_notify(notify);
    request.set_persistent(persistent);

    grpc::Status status =
        _stub->AcknowledgementHostProblem(&context, request, response);
    if (!status.ok()) {
      std::cout << "AcknowledgementHostProblem rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool AcknowledgementServiceProblem(std::string const& hostname,
                                     std::string const& servicedesc,
                                     std::string const& ackauthor,
                                     std::string const& ackdata,
                                     int type,
                                     bool notify,
                                     bool persistent,
                                     CommandSuccess* response) {
    EngineAcknowledgement request;
    grpc::ClientContext context;
    request.set_host_name(hostname);
    request.set_service_desc(servicedesc);
    request.set_ack_author(ackauthor);
    request.set_ack_data(ackdata);
    request.set_type(static_cast<EngineAcknowledgement::Type>(type));
    request.set_notify(notify);
    request.set_persistent(persistent);

    grpc::Status status =
        _stub->AcknowledgementServiceProblem(&context, request, response);
    if (!status.ok()) {
      std::cout << "AcknowledgementServiceProblem rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool ScheduleHostDowntime(std::string const& hostname,
                            std::pair<bool, uint32_t> const& start,
                            std::pair<bool, uint32_t> const& end,
                            std::pair<bool, bool> const& fixed,
                            std::pair<bool, uint32_t> const& triggeredby,
                            std::pair<bool, uint32_t> const& duration,
                            std::string const& author,
                            std::string const& commentdata,
                            std::pair<bool, uint32_t> const& entrytime,
                            CommandSuccess* response) {
    ScheduleDowntimeIdentifier request;
    grpc::ClientContext context;
    request.set_host_name(hostname);
    if (start.first)
      request.set_start(start.second);
    if (end.first)
      request.set_end(end.second);
    if (fixed.first)
      request.set_fixed(fixed.second);
    if (triggeredby.first)
      request.set_triggered_by(triggeredby.second);
    if (duration.first)
      request.set_duration(duration.second);
    request.set_author(author);
    request.set_comment_data(commentdata);
    if (entrytime.first)
      request.set_entry_time(entrytime.second);

    grpc::Status status =
        _stub->ScheduleHostDowntime(&context, request, response);
    if (!status.ok() || !response->value()) {
      std::cout << "ScheduleDowntime rpc engine failed" << std::endl;
      return false;
    }

    return true;
  }

  bool ScheduleServiceDowntime(std::string const& hostname,
                               std::string const& svcdsc,
                               std::pair<bool, uint32_t> const& start,
                               std::pair<bool, uint32_t> const& end,
                               std::pair<bool, bool> const& fixed,
                               std::pair<bool, uint32_t> const& triggeredby,
                               std::pair<bool, uint32_t> const& duration,
                               std::string const& author,
                               std::string const& commentdata,
                               std::pair<bool, uint32_t> const& entrytime,
                               CommandSuccess* response) {
    ScheduleDowntimeIdentifier request;
    grpc::ClientContext context;
    request.set_host_name(hostname);
    request.set_service_desc(svcdsc);
    if (start.first)
      request.set_start(start.second);
    if (end.first)
      request.set_end(end.second);
    if (fixed.first)
      request.set_fixed(fixed.second);
    if (triggeredby.first)
      request.set_triggered_by(triggeredby.second);
    if (duration.first)
      request.set_duration(duration.second);
    request.set_author(author);
    request.set_comment_data(commentdata);
    if (entrytime.first)
      request.set_entry_time(entrytime.second);

    grpc::Status status =
        _stub->ScheduleServiceDowntime(&context, request, response);
    if (!status.ok() || !response->value()) {
      std::cout << "ScheduleDowntime rpc engine failed" << std::endl;
      return false;
    }

    return true;
  }

  bool ScheduleHostServicesDowntime(
      std::string const& hostname,
      std::pair<bool, uint32_t> const& start,
      std::pair<bool, uint32_t> const& end,
      std::pair<bool, bool> const& fixed,
      std::pair<bool, uint32_t> const& triggeredby,
      std::pair<bool, uint32_t> const& duration,
      std::string const& author,
      std::string const& commentdata,
      std::pair<bool, uint32_t> const& entrytime,
      CommandSuccess* response) {
    ScheduleDowntimeIdentifier request;
    grpc::ClientContext context;
    request.set_host_name(hostname);
    if (start.first)
      request.set_start(start.second);
    if (end.first)
      request.set_end(end.second);
    if (fixed.first)
      request.set_fixed(fixed.second);
    if (triggeredby.first)
      request.set_triggered_by(triggeredby.second);
    if (duration.first)
      request.set_duration(duration.second);
    request.set_author(author);
    request.set_comment_data(commentdata);
    if (entrytime.first)
      request.set_entry_time(entrytime.second);

    grpc::Status status =
        _stub->ScheduleHostServicesDowntime(&context, request, response);
    if (!status.ok() || !response->value()) {
      std::cout << "ScheduleHostServicesDowntime rpc engine failed"
                << std::endl;
      return false;
    }

    return true;
  }

  bool ScheduleHostGroupHostsDowntime(
      std::string const& hostgroupname,
      std::pair<bool, uint32_t> const& start,
      std::pair<bool, uint32_t> const& end,
      std::pair<bool, bool> const& fixed,
      std::pair<bool, uint32_t> const& triggeredby,
      std::pair<bool, uint32_t> const& duration,
      std::string const& author,
      std::string const& commentdata,
      std::pair<bool, uint32_t> const& entrytime,
      CommandSuccess* response) {
    ScheduleDowntimeIdentifier request;
    grpc::ClientContext context;
    request.set_host_group_name(hostgroupname);
    if (start.first)
      request.set_start(start.second);
    if (end.first)
      request.set_end(end.second);
    if (fixed.first)
      request.set_fixed(fixed.second);
    if (triggeredby.first)
      request.set_triggered_by(triggeredby.second);
    if (duration.first)
      request.set_duration(duration.second);
    request.set_author(author);
    request.set_comment_data(commentdata);
    if (entrytime.first)
      request.set_entry_time(entrytime.second);

    grpc::Status status =
        _stub->ScheduleHostGroupHostsDowntime(&context, request, response);
    if (!status.ok() || !response->value()) {
      std::cout << "ScheduleHostGroupHostsDowntimerpc engine failed"
                << std::endl;
      return false;
    }

    return true;
  }

  bool ScheduleHostGroupServicesDowntime(
      std::string const& hostgroupname,
      std::pair<bool, uint32_t> const& start,
      std::pair<bool, uint32_t> const& end,
      std::pair<bool, bool> const& fixed,
      std::pair<bool, uint32_t> const& triggeredby,
      std::pair<bool, uint32_t> const& duration,
      std::string const& author,
      std::string const& commentdata,
      std::pair<bool, uint32_t> const& entrytime,
      CommandSuccess* response) {
    ScheduleDowntimeIdentifier request;
    grpc::ClientContext context;
    request.set_host_group_name(hostgroupname);
    if (start.first)
      request.set_start(start.second);
    if (end.first)
      request.set_end(end.second);
    if (fixed.first)
      request.set_fixed(fixed.second);
    if (triggeredby.first)
      request.set_triggered_by(triggeredby.second);
    if (duration.first)
      request.set_duration(duration.second);
    request.set_author(author);
    request.set_comment_data(commentdata);
    if (entrytime.first)
      request.set_entry_time(entrytime.second);

    grpc::Status status =
        _stub->ScheduleHostGroupServicesDowntime(&context, request, response);
    if (!status.ok() || !response->value()) {
      std::cout << "ScheduleHostGroupServicesDowntimerpc engine failed"
                << std::endl;
      return false;
    }

    return true;
  }

  bool ScheduleServiceGroupHostsDowntime(
      std::string const& servicegroupname,
      std::pair<bool, uint32_t> const& start,
      std::pair<bool, uint32_t> const& end,
      std::pair<bool, bool> const& fixed,
      std::pair<bool, uint32_t> const& triggeredby,
      std::pair<bool, uint32_t> const& duration,
      std::string const& author,
      std::string const& commentdata,
      std::pair<bool, uint32_t> const& entrytime,
      CommandSuccess* response) {
    ScheduleDowntimeIdentifier request;
    grpc::ClientContext context;
    request.set_service_group_name(servicegroupname);
    if (start.first)
      request.set_start(start.second);
    if (end.first)
      request.set_end(end.second);
    if (fixed.first)
      request.set_fixed(fixed.second);
    if (triggeredby.first)
      request.set_triggered_by(triggeredby.second);
    if (duration.first)
      request.set_duration(duration.second);
    request.set_author(author);
    request.set_comment_data(commentdata);
    if (entrytime.first)
      request.set_entry_time(entrytime.second);

    grpc::Status status =
        _stub->ScheduleServiceGroupHostsDowntime(&context, request, response);
    if (!status.ok() || !response->value()) {
      std::cout << "ScheduleServiceGroupHostsDowntim engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool ScheduleServiceGroupServicesDowntime(
      std::string const& servicegroupname,
      std::pair<bool, uint32_t> const& start,
      std::pair<bool, uint32_t> const& end,
      std::pair<bool, bool> const& fixed,
      std::pair<bool, uint32_t> const& triggeredby,
      std::pair<bool, uint32_t> const& duration,
      std::string const& author,
      std::string const& commentdata,
      std::pair<bool, uint32_t> const& entrytime,
      CommandSuccess* response) {
    ScheduleDowntimeIdentifier request;
    grpc::ClientContext context;
    request.set_service_group_name(servicegroupname);
    if (start.first)
      request.set_start(start.second);
    if (end.first)
      request.set_end(end.second);
    if (fixed.first)
      request.set_fixed(fixed.second);
    if (triggeredby.first)
      request.set_triggered_by(triggeredby.second);
    if (duration.first)
      request.set_duration(duration.second);
    request.set_author(author);
    request.set_comment_data(commentdata);
    if (entrytime.first)
      request.set_entry_time(entrytime.second);

    grpc::Status status = _stub->ScheduleServiceGroupServicesDowntime(
        &context, request, response);
    if (!status.ok() || !response->value()) {
      std::cout << "ScheduleServiceGroupServicesDowntime engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool ScheduleAndPropagateHostDowntime(
      std::string const& hostname,
      std::pair<bool, uint32_t> const& start,
      std::pair<bool, uint32_t> const& end,
      std::pair<bool, bool> const& fixed,
      std::pair<bool, uint32_t> const& triggeredby,
      std::pair<bool, uint32_t> const& duration,
      std::string const& author,
      std::string const& commentdata,
      std::pair<bool, uint32_t> const& entrytime,
      CommandSuccess* response) {
    ScheduleDowntimeIdentifier request;
    grpc::ClientContext context;
    request.set_host_name(hostname);
    if (start.first)
      request.set_start(start.second);
    if (end.first)
      request.set_end(end.second);
    if (fixed.first)
      request.set_fixed(fixed.second);
    if (triggeredby.first)
      request.set_triggered_by(triggeredby.second);
    if (duration.first)
      request.set_duration(duration.second);
    request.set_author(author);
    request.set_comment_data(commentdata);
    if (entrytime.first)
      request.set_entry_time(entrytime.second);

    grpc::Status status =
        _stub->ScheduleAndPropagateHostDowntime(&context, request, response);
    if (!status.ok() || !response->value()) {
      std::cout << "ScheduleAndPropagateHostDowntime "
                   "rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool ScheduleAndPropagateTriggeredHostDowntime(
      std::string const& hostname,
      std::pair<bool, uint32_t> const& start,
      std::pair<bool, uint32_t> const& end,
      std::pair<bool, bool> const& fixed,
      std::pair<bool, uint32_t> const& triggeredby,
      std::pair<bool, uint32_t> const& duration,
      std::string const& author,
      std::string const& commentdata,
      std::pair<bool, uint32_t> const& entrytime,
      CommandSuccess* response) {
    ScheduleDowntimeIdentifier request;
    grpc::ClientContext context;
    request.set_host_name(hostname);
    if (start.first)
      request.set_start(start.second);
    if (end.first)
      request.set_end(end.second);
    if (fixed.first)
      request.set_fixed(fixed.second);
    if (triggeredby.first)
      request.set_triggered_by(triggeredby.second);
    if (duration.first)
      request.set_duration(duration.second);
    request.set_author(author);
    request.set_comment_data(commentdata);
    if (entrytime.first)
      request.set_entry_time(entrytime.second);

    grpc::Status status = _stub->ScheduleAndPropagateTriggeredHostDowntime(
        &context, request, response);
    if (!status.ok() || !response->value()) {
      std::cout << "ScheduleAndPropagateTriggeredHostDowntime "
                   "rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool DeleteDowntime(uint32_t& downtime_id, CommandSuccess* response) {
    GenericValue request;
    grpc::ClientContext context;
    request.set_value(downtime_id);

    grpc::Status status = _stub->DeleteDowntime(&context, request, response);
    if (!status.ok()) {
      std::cout << "RemoveHostAcknowledgementByIds rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool DeleteHostDowntimeFull(std::string const& hostname,
                              std::pair<bool, uint32_t> const& start,
                              std::pair<bool, uint32_t> const& end,
                              std::pair<bool, bool> const& fixed,
                              std::pair<bool, uint32_t> const& triggeredby,
                              std::pair<bool, uint32_t> const& duration,
                              std::string const& author,
                              std::string const& commentdata,
                              CommandSuccess* response) {
    DowntimeCriterias request;
    grpc::ClientContext context;

    request.set_host_name(hostname);
    request.set_author(author);
    request.set_comment_data(commentdata);

    /*checking if the criteria is defined or not*/
    if (start.first)
      request.mutable_start()->set_value(start.second);
    if (end.first)
      request.mutable_end()->set_value(end.second);
    if (fixed.first)
      request.mutable_fixed()->set_value(fixed.second);
    if (triggeredby.first)
      request.mutable_triggered_by()->set_value(triggeredby.second);
    if (duration.first)
      request.mutable_duration()->set_value(duration.second);

    grpc::Status status =
        _stub->DeleteHostDowntimeFull(&context, request, response);
    if (!status.ok()) {
      std::cout << "RemoveHostAcknowledgementByIds rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool DeleteServiceDowntimeFull(std::string const& hostname,
                                 std::string const& svcdsc,
                                 std::pair<bool, uint32_t> const& start,
                                 std::pair<bool, uint32_t> const& end,
                                 std::pair<bool, bool> const& fixed,
                                 std::pair<bool, uint32_t> const& triggeredby,
                                 std::pair<bool, uint32_t> const& duration,
                                 std::string const& author,
                                 std::string const& commentdata,
                                 CommandSuccess* response) {
    DowntimeCriterias request;
    grpc::ClientContext context;

    request.set_host_name(hostname);
    request.set_service_desc(svcdsc);
    request.set_author(author);
    request.set_comment_data(commentdata);

    /*checking if the criteria is defined or not*/
    if (start.first)
      request.mutable_start()->set_value(start.second);
    if (end.first)
      request.mutable_end()->set_value(end.second);
    if (fixed.first)
      request.mutable_fixed()->set_value(fixed.second);
    if (triggeredby.first)
      request.mutable_triggered_by()->set_value(triggeredby.second);
    if (duration.first)
      request.mutable_duration()->set_value(duration.second);

    grpc::Status status =
        _stub->DeleteServiceDowntimeFull(&context, request, response);
    if (!status.ok()) {
      std::cout << "DeleteServiceDowntimeFull rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool DeleteDowntimeByHostName(std::string const& hostname,
                                std::string const& svcdsc,
                                std::pair<bool, uint32_t> const& start,
                                std::string const& commentdata,
                                CommandSuccess* response) {
    DowntimeHostIdentifier request;
    grpc::ClientContext context;
    request.set_host_name(hostname);
    request.set_service_desc(svcdsc);
    request.set_comment_data(commentdata);
    if (start.first)
      request.mutable_start()->set_value(start.second);

    grpc::Status status =
        _stub->DeleteDowntimeByHostName(&context, request, response);
    if (!status.ok()) {
      std::cout << "DeleteDowntimeByHostName rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool DeleteDowntimeByHostGroupName(std::string const& hostgroupname,
                                     std::string const& hostname,
                                     std::string const& svcdsc,
                                     std::string const& commentdata,
                                     std::pair<bool, uint32_t> const& start,
                                     CommandSuccess* response) {
    DowntimeHostGroupIdentifier request;
    grpc::ClientContext context;
    request.set_host_group_name(hostgroupname);
    request.set_host_name(hostname);
    request.set_service_desc(svcdsc);
    request.set_comment_data(commentdata);
    if (start.first)
      request.mutable_start()->set_value(start.second);

    grpc::Status status =
        _stub->DeleteDowntimeByHostGroupName(&context, request, response);
    if (!status.ok()) {
      std::cout << "DeleteDowntimeByHostGroupName rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool DeleteDowntimeByStartTimeComment(uint32_t const start,
                                        std::string const& commentdata,
                                        CommandSuccess* response) {
    DowntimeStartTimeIdentifier request;
    grpc::ClientContext context;
    request.set_comment_data(commentdata);
    request.mutable_start()->set_value(start);

    grpc::Status status =
        _stub->DeleteDowntimeByStartTimeComment(&context, request, response);
    if (!status.ok()) {
      std::cout << "DeleteDowntimeByStartTimeComment rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool ScheduleHostCheck(std::string const& hostname,
                         uint32_t delaytime,
                         CommandSuccess* response) {
    HostCheckIdentifier request;
    grpc::ClientContext context;
    request.set_host_name(hostname);
    request.set_delay_time(delaytime);

    grpc::Status status = _stub->ScheduleHostCheck(&context, request, response);
    if (!status.ok()) {
      std::cout << "ScheduleHostCheck"
                   "rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool ScheduleHostServiceCheck(std::string const& hostname,
                                uint32_t delaytime,
                                CommandSuccess* response) {
    HostCheckIdentifier request;
    grpc::ClientContext context;
    request.set_host_name(hostname);
    request.set_delay_time(delaytime);

    grpc::Status status =
        _stub->ScheduleHostServiceCheck(&context, request, response);
    if (!status.ok()) {
      std::cout << "ScheduleHostServiceCheck"
                   "rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool ScheduleServiceCheck(std::string const& hostname,
                            std::string const& servicename,
                            uint32_t delaytime,
                            CommandSuccess* response) {
    ServiceCheckIdentifier request;
    grpc::ClientContext context;
    request.set_host_name(hostname);
    request.set_service_desc(servicename);
    request.set_delay_time(delaytime);

    grpc::Status status =
        _stub->ScheduleServiceCheck(&context, request, response);
    if (!status.ok()) {
      std::cout << "ScheduleServiceCheck"
                   "rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool SignalProcess(int& process,
                     uint32_t& scheduledtime,
                     CommandSuccess* response) {
    grpc::ClientContext context;
    EngineSignalProcess request;
    request.set_process(static_cast<EngineSignalProcess::Process>(process));
    request.set_scheduled_time(scheduledtime);

    grpc::Status status = _stub->SignalProcess(&context, request, response);
    if (!status.ok()) {
      std::cout << "SignalProcess failed." << std::endl;
      return false;
    }
    return true;
  }

  bool DelayHostNotificationByName(std::string const& hostname,
                                   uint32_t& delaytime,
                                   CommandSuccess* response) {
    HostDelayIdentifier request;
    grpc::ClientContext context;
    request.set_name(hostname);
    request.set_delay_time(delaytime);

    grpc::Status status =
        _stub->DelayHostNotification(&context, request, response);
    if (!status.ok()) {
      std::cout << "DelayHostNotification failed." << std::endl;
      return false;
    }
    return true;
  }

  bool DelayHostNotificationById(uint32_t& hostid,
                                 uint32_t& delaytime,
                                 CommandSuccess* response) {
    HostDelayIdentifier request;
    grpc::ClientContext context;
    request.set_id(hostid);
    request.set_delay_time(delaytime);

    grpc::Status status =
        _stub->DelayHostNotification(&context, request, response);
    if (!status.ok()) {
      std::cout << "DelayHostNotification failed." << std::endl;
      return false;
    }
    return true;
  }

  bool DelayServiceNotificationByNames(std::string const& hostname,
                                       std::string const& svcdsc,
                                       uint32_t& delaytime,
                                       CommandSuccess* response) {
    ServiceDelayIdentifier request;
    grpc::ClientContext context;
    request.mutable_names()->set_host_name(hostname);
    request.mutable_names()->set_service_name(svcdsc);
    request.set_delay_time(delaytime);
    grpc::Status status =
        _stub->DelayServiceNotification(&context, request, response);
    if (!status.ok()) {
      std::cout << "DelayServiceNotification rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool DelayServiceNotificationByIds(uint32_t& hostid,
                                     uint32_t& serviceid,
                                     uint32_t& delaytime,
                                     CommandSuccess* response) {
    ServiceDelayIdentifier request;
    grpc::ClientContext context;
    request.mutable_ids()->set_host_id(hostid);
    request.mutable_ids()->set_service_id(serviceid);
    request.set_delay_time(delaytime);

    grpc::Status status =
        _stub->DelayServiceNotification(&context, request, response);
    if (!status.ok()) {
      std::cout << "DelayServiceNotification rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool ChangeHostObjectIntVar(std::string const& hostname,
                              uint32_t& mode,
                              uint32_t& intval,
                              double& dval,
                              CommandSuccess* response) {
    ChangeObjectInt request;
    grpc::ClientContext context;

    request.set_host_name(hostname);
    request.set_mode(static_cast<ChangeObjectInt::Mode>(mode));
    request.set_intval(intval);
    request.set_dval(dval);

    grpc::Status status =
        _stub->ChangeHostObjectIntVar(&context, request, response);
    if (!status.ok()) {
      std::cout << "ChangeHostObjectIntVa rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool ChangeServiceObjectIntVar(std::string const& hostname,
                                 std::string const& servicedesc,
                                 uint32_t& mode,
                                 uint32_t& intval,
                                 double& dval,
                                 CommandSuccess* response) {
    ChangeObjectInt request;
    grpc::ClientContext context;

    request.set_host_name(hostname);
    request.set_service_desc(servicedesc);
    request.set_mode(static_cast<ChangeObjectInt::Mode>(mode));
    request.set_intval(intval);
    request.set_dval(dval);

    grpc::Status status =
        _stub->ChangeServiceObjectIntVar(&context, request, response);
    if (!status.ok()) {
      std::cout << "ChangeServiceObjectIntVa rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool ChangeContactObjectIntVar(std::string const& contactname,
                                 uint32_t& mode,
                                 uint32_t& intval,
                                 double& dval,
                                 CommandSuccess* response) {
    ChangeContactObjectInt request;
    grpc::ClientContext context;

    request.set_contact_name(contactname);
    request.set_mode(static_cast<ChangeContactObjectInt::Mode>(mode));
    request.set_intval(intval);
    request.set_dval(dval);
    grpc::Status status =
        _stub->ChangeContactObjectIntVar(&context, request, response);
    if (!status.ok()) {
      std::cout << "ChangeContactObjectIntVar rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool ChangeHostObjectCharVar(std::string const& hostname,
                               uint32_t mode,
                               std::string charval,
                               CommandSuccess* response) {
    ChangeObjectChar request;
    grpc::ClientContext context;

    request.set_host_name(hostname);
    request.set_mode(static_cast<ChangeObjectChar::Mode>(mode));
    request.set_charval(charval);

    grpc::Status status =
        _stub->ChangeHostObjectCharVar(&context, request, response);
    if (!status.ok()) {
      std::cout << "ChangeHostObjectCharVar rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool ChangeServiceObjectCharVar(std::string const& hostname,
                                  std::string const& servicedesc,
                                  uint32_t mode,
                                  std::string charval,
                                  CommandSuccess* response) {
    ChangeObjectChar request;
    grpc::ClientContext context;

    request.set_host_name(hostname);
    request.set_service_desc(servicedesc);
    request.set_mode(static_cast<ChangeObjectChar::Mode>(mode));
    request.set_charval(charval);

    grpc::Status status =
        _stub->ChangeServiceObjectCharVar(&context, request, response);
    if (!status.ok()) {
      std::cout << "ChangeServiceObjectCharVar rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool ChangeContactObjectCharVar(std::string const& contact,
                                  uint32_t mode,
                                  std::string charval,
                                  CommandSuccess* response) {
    ChangeContactObjectChar request;
    grpc::ClientContext context;

    request.set_contact(contact);
    request.set_mode(static_cast<ChangeContactObjectChar::Mode>(mode));
    request.set_charval(charval);

    grpc::Status status =
        _stub->ChangeContactObjectCharVar(&context, request, response);
    if (!status.ok()) {
      std::cout << "ChangeContactObjectCharVar rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool ChangeHostObjectCustomVar(std::string const& hostname,
                                 std::string const& varname,
                                 std::string const& varvalue,
                                 CommandSuccess* response) {
    ChangeObjectCustomVar request;
    grpc::ClientContext context;

    request.set_host_name(hostname);
    request.set_varname(varname);
    request.set_varvalue(varvalue);
    grpc::Status status =
        _stub->ChangeHostObjectCustomVar(&context, request, response);
    if (!status.ok()) {
      std::cout << "ChangeHostObjectCustomVar rpc engine failed" << std::endl;
      return false;
    }
    return true;
  }

  bool ChangeServiceObjectCustomVar(std::string const& hostname,
                                    std::string const& servicedesc,
                                    std::string const& varname,
                                    std::string const& varvalue,
                                    CommandSuccess* response) {
    ChangeObjectCustomVar request;
    grpc::ClientContext context;

    request.set_host_name(hostname);
    request.set_service_desc(servicedesc);
    request.set_varname(varname);
    request.set_varvalue(varvalue);
    grpc::Status status =
        _stub->ChangeServiceObjectCustomVar(&context, request, response);
    if (!status.ok()) {
      std::cout << "ChangeServiceObjectCustomVar rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }

  bool ChangeContactObjectCustomVar(std::string const& contact,
                                    std::string const& varname,
                                    std::string const& varvalue,
                                    CommandSuccess* response) {
    ChangeObjectCustomVar request;
    grpc::ClientContext context;

    request.set_contact(contact);
    request.set_varname(varname);
    request.set_varvalue(varvalue);
    grpc::Status status =
        _stub->ChangeContactObjectCustomVar(&context, request, response);
    if (!status.ok()) {
      std::cout << "ChangeContactObjectCustomVar rpc engine failed"
                << std::endl;
      return false;
    }
    return true;
  }
};

int main(int argc, char** argv) {
  int32_t status = 0;
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
  } else if (strcmp(argv[1], "GetStats") == 0) {
    Stats stats;
    status = client.GetStats(&stats) ? 0 : 2;
    std::cout << "GetStats: " << stats.DebugString();
  } else if (strcmp(argv[1], "ProcessServiceCheckResult") == 0) {
    Check sc;
    sc.set_host_name(argv[2]);
    sc.set_svc_desc(argv[3]);
    sc.set_code(std::stol(argv[4]));
    sc.set_output("Test external command");
    status = client.ProcessServiceCheckResult(sc) ? 0 : 3;
    std::cout << "ProcessServiceCheckResult: " << status << std::endl;
  } else if (strcmp(argv[1], "ProcessHostCheckResult") == 0) {
    Check hc;
    hc.set_host_name(argv[2]);
    hc.set_code(std::stol(argv[3]));
    hc.set_output("Test external command");
    status = client.ProcessHostCheckResult(hc) ? 0 : 4;
    std::cout << "ProcessHostCheckResult: " << status << std::endl;
  } else if (strcmp(argv[1], "NewThresholdsFile") == 0) {
    ThresholdsFile tf;
    tf.set_filename(argv[2]);
    status = client.NewThresholdsFile(tf) ? 0 : 5;
    std::cout << "NewThresholdsFile: " << status << std::endl;
  } else if (strcmp(argv[1], "GetHost") == 0) {
    if (argc != 4) {
      std::cout << "GetHost require arguments : GetHost [mode] [hostname or id]"
                << std::endl;
      return 1;
    } else if (strcmp(argv[2], "byhostid") == 0) {
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
    } else if (strcmp(argv[2], "byhostname") == 0) {
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
  } else if (strcmp(argv[1], "GetContact") == 0) {
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
  } else if (strcmp(argv[1], "GetService") == 0) {
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
  } else if (strcmp(argv[1], "GetHostsCount") == 0) {
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
  } else if (strcmp(argv[1], "AddHostComment") == 0) {
    if (argc != 7) {
      std::cout << "AddHostComment require arguments : "
                   "AddHostComment "
                   "[hostname] [user] [your_own_comment] [persistent] "
                   "[entry_time]"
                << std::endl;
      return 1;
    }
    CommandSuccess response;
    std::string hostname = argv[2];
    std::string user = argv[3];
    std::string commentdata = argv[4];
    bool persistent = atoi(argv[5]);
    uint32_t entrytime = atoi(argv[6]);
    status = client.AddHostComment(hostname, entrytime, user, commentdata,
                                   persistent, &response);
    std::cout << "AddHostComment" << std::endl;
  } else if (strcmp(argv[1], "AddServiceComment") == 0) {
    if (argc != 8) {
      std::cout << "AddHostComment require arguments : "
                   "AddHostComment "
                   "[hostname] [service_description] [user] [your_own_comment] "
                   "[persistent] [entry_time]"
                << std::endl;
      return 1;
    }
    CommandSuccess response;
    std::string hostname{argv[2]};
    std::string svcdsc{argv[3]};
    std::string user{argv[4]};
    std::string commentdata{argv[5]};
    bool persistent = atoi(argv[6]);
    uint32_t entrytime = atoi(argv[7]);
    status = client.AddServiceComment(hostname, svcdsc, entrytime, user,
                                      commentdata, persistent, &response);
    std::cout << "AddServiceComment " << status << std::endl;
  } else if (strcmp(argv[1], "DeleteAllHostComments") == 0) {
    if (argc != 4) {
      std::cout << "DeleteAllHostComments require arguments : GetHost [mode] "
                   "[hostname or id]"
                << std::endl;
      return 1;
    } else if (strcmp(argv[2], "byhostname") == 0) {
      CommandSuccess response;
      std::string hostname(argv[3]);
      status = client.DeleteAllHostCommentsByName(hostname, &response);
      std::cout << "DeleteAllHostComments" << std::endl;
    } else if (strcmp(argv[2], "byhostid") == 0) {
      CommandSuccess response;
      uint32_t hostid = atoi(argv[3]);
      status = client.DeleteAllHostCommentsById(hostid, &response);
      std::cout << "DeleteAllHostComments" << std::endl;
    }
  } else if (strcmp(argv[1], "DeleteAllServiceComments") == 0) {
    if (argc != 5) {
      std::cout << "DeleteAllServiceComments require arguments : "
                   "DeleteAllServiceComments "
                   "[mode] [hostname "
                   "or hostid] [servicename or serviceid]"
                << std::endl;
      return 1;
    } else if (strcmp(argv[2], "bynames") == 0) {
      CommandSuccess response;
      std::string hostname(argv[3]);
      std::string svcdsc(argv[4]);
      status =
          client.DeleteAllServiceCommentsByNames(hostname, svcdsc, &response);
      std::cout << "DeleteAllServiceComments" << std::endl;
    } else if (strcmp(argv[2], "byids") == 0) {
      CommandSuccess response;
      uint32_t hostid = atoi(argv[3]);
      uint32_t serviceid = atoi(argv[4]);
      status =
          client.DeleteAllServiceCommentsByIds(hostid, serviceid, &response);
      std::cout << "DeleteAllServiceComments" << std::endl;
    }
  } else if (strcmp(argv[1], "DeleteComment") == 0) {
    if (argc != 3) {
      std::cout
          << "DeleteComment require arguments : DeleteComment [comment_id]"
          << std::endl;
      return 1;
    }
    CommandSuccess response;
    uint32_t commentid = atoi(argv[2]);
    status = client.DeleteComment(commentid, &response);
    std::cout << "DeleteComment " << status << std::endl;
  } else if (strcmp(argv[1], "RemoveHostAcknowledgement") == 0) {
    if (argc != 4) {
      std::cout << "RemoveHostAcknowledgement require arguments : "
                   "RemoveHostAcknowledgement [mode] [hostname or id]"
                << std::endl;
      return 1;
    } else if (strcmp(argv[2], "byhostname") == 0) {
      CommandSuccess response;
      std::string hostname(argv[3]);
      status = client.RemoveHostAcknowledgementByNames(hostname, &response);
      std::cout << "RemoveHostAcknowledgement" << std::endl;
    } else if (strcmp(argv[2], "byhostid") == 0) {
      CommandSuccess response;
      uint32_t hostid = atoi(argv[3]);
      status = client.RemoveHostAcknowledgementByIds(hostid, &response);
      std::cout << "RemoveHostAcknowledgement" << std::endl;
    }
  } else if (strcmp(argv[1], "RemoveServiceAcknowledgement") == 0) {
    if (argc != 5) {
      std::cout << "RemoveServiceAcknowledgement require arguments : "
                   "RemoveServiceAcknowledgement "
                   "[mode] [hostname "
                   "or hostid] [servicename or serviceid]"
                << std::endl;
      return 1;
    } else if (strcmp(argv[2], "bynames") == 0) {
      CommandSuccess response;
      std::string hostname(argv[3]);
      std::string svcdsc(argv[4]);
      status = client.RemoveServiceAcknowledgementByNames(hostname, svcdsc,
                                                          &response);
      std::cout << "RemoveServiceAcknowledgement" << std::endl;
    } else if (strcmp(argv[2], "byids") == 0) {
      CommandSuccess response;
      uint32_t hostid = atoi(argv[3]);
      uint32_t serviceid = atoi(argv[4]);
      status = client.RemoveServiceAcknowledgementByIds(hostid, serviceid,
                                                        &response);
      std::cout << "RemoveServiceAcknowledgement" << std::endl;
    }
  } else if (strcmp(argv[1], "AcknowledgementHostProblem") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::string ackauthor(argv[3]);
    std::string ackdata(argv[4]);
    int type = atoi(argv[5]);
    bool notify = atoi(argv[6]);
    bool persistent = atoi(argv[7]);

    status = client.AcknowledgementHostProblem(
        hostname, ackauthor, ackdata, type, notify, persistent, &response);
    std::cout << "AcknowledgementHostProblem" << std::endl;
  } else if (strcmp(argv[1], "AcknowledgementServiceProblem") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::string servicedesc(argv[3]);
    std::string ackauthor(argv[4]);
    std::string ackdata(argv[5]);
    int type = atoi(argv[6]);
    bool notify = atoi(argv[7]);
    bool persistent = atoi(argv[8]);

    status = client.AcknowledgementServiceProblem(
        hostname, servicedesc, ackauthor, ackdata, type, notify, persistent,
        &response);
    std::cout << "AcknowledgementServiceProblem" << std::endl;
  } else if (strcmp(argv[1], "ScheduleHostDowntime") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::pair<bool, uint32_t> start;
    std::pair<bool, uint32_t> end;
    std::pair<bool, bool> fixed;
    std::pair<bool, uint32_t> triggeredby;
    std::pair<bool, uint32_t> duration;
    std::string author;
    std::string commentdata;
    std::pair<bool, uint32_t> entrytime;

    if (strcmp(argv[3], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[3]));
    if (strcmp(argv[4], "undef") == 0)
      end = std::make_pair(false, 0);
    else
      end = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") == 0)
      fixed = std::make_pair(false, 0);
    else
      fixed = std::make_pair(true, atoi(argv[5]));
    if (strcmp(argv[6], "undef") == 0)
      triggeredby = std::make_pair(false, 0);
    else
      triggeredby = std::make_pair(true, atoi(argv[6]));
    if (strcmp(argv[7], "undef") == 0)
      duration = std::make_pair(false, 0);
    else
      duration = std::make_pair(true, atoi(argv[7]));
    if (strcmp(argv[8], "undef") != 0)
      author = argv[8];
    if (strcmp(argv[9], "undef") != 0)
      commentdata = argv[9];
    if (strcmp(argv[10], "undef") == 0)
      entrytime = std::make_pair(false, 0);
    else
      entrytime = std::make_pair(true, atoi(argv[10]));

    status = client.ScheduleHostDowntime(hostname, start, end, fixed,
                                         triggeredby, duration, author,
                                         commentdata, entrytime, &response);
    std::cout << "ScheduleHostDowntime " << status << std::endl;
  } else if (strcmp(argv[1], "ScheduleServiceDowntime") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::string svcdsc(argv[3]);
    std::pair<bool, uint32_t> start;
    std::pair<bool, uint32_t> end;
    std::pair<bool, bool> fixed;
    std::pair<bool, uint32_t> triggeredby;
    std::pair<bool, uint32_t> duration;
    std::string author;
    std::string commentdata;
    std::pair<bool, uint32_t> entrytime;

    if (strcmp(argv[4], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") == 0)
      end = std::make_pair(false, 0);
    else
      end = std::make_pair(true, atoi(argv[5]));
    if (strcmp(argv[6], "undef") == 0)
      fixed = std::make_pair(false, 0);
    else
      fixed = std::make_pair(true, atoi(argv[6]));
    if (strcmp(argv[7], "undef") == 0)
      triggeredby = std::make_pair(false, 0);
    else
      triggeredby = std::make_pair(true, atoi(argv[7]));
    if (strcmp(argv[8], "undef") == 0)
      duration = std::make_pair(false, 0);
    else
      duration = std::make_pair(true, atoi(argv[8]));
    if (strcmp(argv[9], "undef") != 0)
      author = argv[9];
    if (strcmp(argv[10], "undef") != 0)
      commentdata = argv[10];
    if (strcmp(argv[11], "undef") == 0)
      entrytime = std::make_pair(false, 0);
    else
      entrytime = std::make_pair(true, atoi(argv[11]));

    status = client.ScheduleServiceDowntime(hostname, svcdsc, start, end, fixed,
                                            triggeredby, duration, author,
                                            commentdata, entrytime, &response);
    std::cout << "ScheduleServiceDowntime " << status << std::endl;
  } else if (strcmp(argv[1], "ScheduleHostServicesDowntime") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::pair<bool, uint32_t> start;
    std::pair<bool, uint32_t> end;
    std::pair<bool, bool> fixed;
    std::pair<bool, uint32_t> triggeredby;
    std::pair<bool, uint32_t> duration;
    std::string author;
    std::string commentdata;
    std::pair<bool, uint32_t> entrytime;

    if (strcmp(argv[3], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[3]));
    if (strcmp(argv[4], "undef") == 0)
      end = std::make_pair(false, 0);
    else
      end = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") == 0)
      fixed = std::make_pair(false, 0);
    else
      fixed = std::make_pair(true, atoi(argv[5]));
    if (strcmp(argv[6], "undef") == 0)
      triggeredby = std::make_pair(false, 0);
    else
      triggeredby = std::make_pair(true, atoi(argv[6]));
    if (strcmp(argv[7], "undef") == 0)
      duration = std::make_pair(false, 0);
    else
      duration = std::make_pair(true, atoi(argv[7]));
    if (strcmp(argv[8], "undef") != 0)
      author = argv[8];
    if (strcmp(argv[9], "undef") != 0)
      commentdata = argv[9];
    if (strcmp(argv[10], "undef") == 0)
      entrytime = std::make_pair(false, 0);
    else
      entrytime = std::make_pair(true, atoi(argv[10]));

    status = client.ScheduleHostServicesDowntime(
        hostname, start, end, fixed, triggeredby, duration, author, commentdata,
        entrytime, &response);
    std::cout << "ScheduleHostServicesDowntime " << status << std::endl;
  } else if (strcmp(argv[1], "ScheduleHostGroupHostsDowntime") == 0) {
    CommandSuccess response;
    std::string hostgroupname(argv[2]);
    std::pair<bool, uint32_t> start;
    std::pair<bool, uint32_t> end;
    std::pair<bool, bool> fixed;
    std::pair<bool, uint32_t> triggeredby;
    std::pair<bool, uint32_t> duration;
    std::string author;
    std::string commentdata;
    std::pair<bool, uint32_t> entrytime;

    if (strcmp(argv[3], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[3]));
    if (strcmp(argv[4], "undef") == 0)
      end = std::make_pair(false, 0);
    else
      end = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") == 0)
      fixed = std::make_pair(false, 0);
    else
      fixed = std::make_pair(true, atoi(argv[5]));
    if (strcmp(argv[6], "undef") == 0)
      triggeredby = std::make_pair(false, 0);
    else
      triggeredby = std::make_pair(true, atoi(argv[6]));
    if (strcmp(argv[7], "undef") == 0)
      duration = std::make_pair(false, 0);
    else
      duration = std::make_pair(true, atoi(argv[7]));
    if (strcmp(argv[8], "undef") != 0)
      author = argv[8];
    if (strcmp(argv[9], "undef") != 0)
      commentdata = argv[9];
    if (strcmp(argv[10], "undef") == 0)
      entrytime = std::make_pair(false, 0);
    else
      entrytime = std::make_pair(true, atoi(argv[10]));

    status = client.ScheduleHostGroupHostsDowntime(
        hostgroupname, start, end, fixed, triggeredby, duration, author,
        commentdata, entrytime, &response);
    std::cout << "ScheduleHostGroupHostsDowntime " << status << std::endl;
  } else if (strcmp(argv[1], "ScheduleHostGroupServicesDowntime") == 0) {
    CommandSuccess response;
    std::string hostgroupname(argv[2]);
    std::pair<bool, uint32_t> start;
    std::pair<bool, uint32_t> end;
    std::pair<bool, bool> fixed;
    std::pair<bool, uint32_t> triggeredby;
    std::pair<bool, uint32_t> duration;
    std::string author;
    std::string commentdata;
    std::pair<bool, uint32_t> entrytime;

    if (strcmp(argv[3], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[3]));
    if (strcmp(argv[4], "undef") == 0)
      end = std::make_pair(false, 0);
    else
      end = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") == 0)
      fixed = std::make_pair(false, 0);
    else
      fixed = std::make_pair(true, atoi(argv[5]));
    if (strcmp(argv[6], "undef") == 0)
      triggeredby = std::make_pair(false, 0);
    else
      triggeredby = std::make_pair(true, atoi(argv[6]));
    if (strcmp(argv[7], "undef") == 0)
      duration = std::make_pair(false, 0);
    else
      duration = std::make_pair(true, atoi(argv[7]));
    if (strcmp(argv[8], "undef") != 0)
      author = argv[8];
    if (strcmp(argv[9], "undef") != 0)
      commentdata = argv[9];
    if (strcmp(argv[10], "undef") == 0)
      entrytime = std::make_pair(false, 0);
    else
      entrytime = std::make_pair(true, atoi(argv[10]));

    status = client.ScheduleHostGroupServicesDowntime(
        hostgroupname, start, end, fixed, triggeredby, duration, author,
        commentdata, entrytime, &response);
    std::cout << "ScheduleHostGroupServicesDowntime " << status << std::endl;
  } else if (strcmp(argv[1], "ScheduleServiceGroupHostsDowntime") == 0) {
    CommandSuccess response;
    std::string servicegroupname(argv[2]);
    std::pair<bool, uint32_t> start;
    std::pair<bool, uint32_t> end;
    std::pair<bool, bool> fixed;
    std::pair<bool, uint32_t> triggeredby;
    std::pair<bool, uint32_t> duration;
    std::string author;
    std::string commentdata;
    std::pair<bool, uint32_t> entrytime;

    if (strcmp(argv[3], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[3]));
    if (strcmp(argv[4], "undef") == 0)
      end = std::make_pair(false, 0);
    else
      end = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") == 0)
      fixed = std::make_pair(false, 0);
    else
      fixed = std::make_pair(true, atoi(argv[5]));
    if (strcmp(argv[6], "undef") == 0)
      triggeredby = std::make_pair(false, 0);
    else
      triggeredby = std::make_pair(true, atoi(argv[6]));
    if (strcmp(argv[7], "undef") == 0)
      duration = std::make_pair(false, 0);
    else
      duration = std::make_pair(true, atoi(argv[7]));
    if (strcmp(argv[8], "undef") != 0)
      author = argv[8];
    if (strcmp(argv[9], "undef") != 0)
      commentdata = argv[9];
    if (strcmp(argv[10], "undef") == 0)
      entrytime = std::make_pair(false, 0);
    else
      entrytime = std::make_pair(true, atoi(argv[10]));

    status = client.ScheduleServiceGroupHostsDowntime(
        servicegroupname, start, end, fixed, triggeredby, duration, author,
        commentdata, entrytime, &response);
    std::cout << "ScheduleServiceGroupHostsDowntime " << status << std::endl;
  } else if (strcmp(argv[1], "ScheduleServiceGroupServicesDowntime") == 0) {
    CommandSuccess response;
    std::string servicegroupname(argv[2]);
    std::pair<bool, uint32_t> start;
    std::pair<bool, uint32_t> end;
    std::pair<bool, bool> fixed;
    std::pair<bool, uint32_t> triggeredby;
    std::pair<bool, uint32_t> duration;
    std::string author;
    std::string commentdata;
    std::pair<bool, uint32_t> entrytime;

    if (strcmp(argv[3], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[3]));
    if (strcmp(argv[4], "undef") == 0)
      end = std::make_pair(false, 0);
    else
      end = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") == 0)
      fixed = std::make_pair(false, 0);
    else
      fixed = std::make_pair(true, atoi(argv[5]));
    if (strcmp(argv[6], "undef") == 0)
      triggeredby = std::make_pair(false, 0);
    else
      triggeredby = std::make_pair(true, atoi(argv[6]));
    if (strcmp(argv[7], "undef") == 0)
      duration = std::make_pair(false, 0);
    else
      duration = std::make_pair(true, atoi(argv[7]));
    if (strcmp(argv[8], "undef") != 0)
      author = argv[8];
    if (strcmp(argv[9], "undef") != 0)
      commentdata = argv[9];
    if (strcmp(argv[10], "undef") == 0)
      entrytime = std::make_pair(false, 0);
    else
      entrytime = std::make_pair(true, atoi(argv[10]));

    status = client.ScheduleServiceGroupServicesDowntime(
        servicegroupname, start, end, fixed, triggeredby, duration, author,
        commentdata, entrytime, &response);
    std::cout << "ScheduleServiceGroupServicesDowntime " << status << std::endl;
  } else if (strcmp(argv[1], "ScheduleAndPropagateHostDowntime") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::pair<bool, uint32_t> start;
    std::pair<bool, uint32_t> end;
    std::pair<bool, bool> fixed;
    std::pair<bool, uint32_t> triggeredby;
    std::pair<bool, uint32_t> duration;
    std::string author;
    std::string commentdata;
    std::pair<bool, uint32_t> entrytime;

    if (strcmp(argv[3], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[3]));
    if (strcmp(argv[4], "undef") == 0)
      end = std::make_pair(false, 0);
    else
      end = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") == 0)
      fixed = std::make_pair(false, 0);
    else
      fixed = std::make_pair(true, atoi(argv[5]));
    if (strcmp(argv[6], "undef") == 0)
      triggeredby = std::make_pair(false, 0);
    else
      triggeredby = std::make_pair(true, atoi(argv[6]));
    if (strcmp(argv[7], "undef") == 0)
      duration = std::make_pair(false, 0);
    else
      duration = std::make_pair(true, atoi(argv[7]));
    if (strcmp(argv[8], "undef") != 0)
      author = argv[8];
    if (strcmp(argv[9], "undef") != 0)
      commentdata = argv[9];
    if (strcmp(argv[10], "undef") == 0)
      entrytime = std::make_pair(false, 0);
    else
      entrytime = std::make_pair(true, atoi(argv[10]));

    status = client.ScheduleAndPropagateHostDowntime(
        hostname, start, end, fixed, triggeredby, duration, author, commentdata,
        entrytime, &response);
    std::cout << "ScheduleAndPropagateHostDowntime " << status << std::endl;
  } else if (strcmp(argv[1], "ScheduleAndPropagateTriggeredHostDowntime") ==
             0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::pair<bool, uint32_t> start;
    std::pair<bool, uint32_t> end;
    std::pair<bool, bool> fixed;
    std::pair<bool, uint32_t> triggeredby;
    std::pair<bool, uint32_t> duration;
    std::string author;
    std::string commentdata;
    std::pair<bool, uint32_t> entrytime;

    if (strcmp(argv[3], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[3]));
    if (strcmp(argv[4], "undef") == 0)
      end = std::make_pair(false, 0);
    else
      end = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") == 0)
      fixed = std::make_pair(false, 0);
    else
      fixed = std::make_pair(true, atoi(argv[5]));
    if (strcmp(argv[6], "undef") == 0)
      triggeredby = std::make_pair(false, 0);
    else
      triggeredby = std::make_pair(true, atoi(argv[6]));
    if (strcmp(argv[7], "undef") == 0)
      duration = std::make_pair(false, 0);
    else
      duration = std::make_pair(true, atoi(argv[7]));
    if (strcmp(argv[8], "undef") != 0)
      author = argv[8];
    if (strcmp(argv[9], "undef") != 0)
      commentdata = argv[9];
    if (strcmp(argv[10], "undef") == 0)
      entrytime = std::make_pair(false, 0);
    else
      entrytime = std::make_pair(true, atoi(argv[10]));

    status = client.ScheduleAndPropagateTriggeredHostDowntime(
        hostname, start, end, fixed, triggeredby, duration, author, commentdata,
        entrytime, &response);
    std::cout << "ScheduleAndPropagateTriggeredHostDowntime " << status
              << std::endl;
  } else if (strcmp(argv[1], "DeleteDowntime") == 0) {
    CommandSuccess response;
    uint32_t downtimeid = atoi(argv[2]);
    status = client.DeleteDowntime(downtimeid, &response);
    std::cout << "DeleteDowntime" << std::endl;
  } else if (strcmp(argv[1], "DeleteDowntimeByHostName") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::string svcdsc;
    std::string commentdata;
    std::pair<bool, uint32_t> start;
    // hostname must be defined to delete the downtime but not others arguments
    if (strcmp(argv[3], "undef") != 0)
      svcdsc = argv[3];
    if (strcmp(argv[4], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") != 0)
      commentdata = argv[5];

    status = client.DeleteDowntimeByHostName(hostname, svcdsc, start,
                                             commentdata, &response);
    std::cout << "DeleteDowntimeByHostName" << std::endl;
  } else if (strcmp(argv[1], "DeleteDowntimeByStartTimeComment") == 0) {
    CommandSuccess response;
    uint32_t start = atoi(argv[2]);
    std::string commentdata = argv[3];

    status =
        client.DeleteDowntimeByStartTimeComment(start, commentdata, &response);
    std::cout << "DeleteDowntimeByStartTimeComment" << std::endl;
  } else if (strcmp(argv[1], "DeleteDowntimeByHostGroupName") == 0) {
    CommandSuccess response;
    std::string hostgroupname(argv[2]);
    std::string hostname;
    std::string svcdsc;
    std::string commentdata;
    std::pair<bool, uint32_t> start;
    // hostname must be defined to delete the downtime but not others arguments
    if (strcmp(argv[3], "undef") != 0)
      hostname = argv[3];
    if (strcmp(argv[4], "undef") != 0)
      svcdsc = argv[4];
    if (strcmp(argv[5], "undef") != 0)
      commentdata = argv[5];
    if (strcmp(argv[6], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[6]));
    status = client.DeleteDowntimeByHostGroupName(
        hostgroupname, hostname, svcdsc, commentdata, start, &response);
    std::cout << "DeleteDowntimeByHostGroupName" << std::endl;
  } else if (strcmp(argv[1], "DeleteHostDowntimeFull") == 0) {
    CommandSuccess response;
    std::string hostname;
    std::string author;
    std::string commentdata;
    std::pair<bool, uint32_t> start;
    std::pair<bool, uint32_t> end;
    std::pair<bool, uint32_t> triggeredby;
    std::pair<bool, uint32_t> duration;
    std::pair<bool, bool> fixed;

    // parsing parameters. we are checking if some parameters is definied or
    // not. here if this parameter is undefined we send an empty string to the
    // function otherwise we fill the string with the value of the parameter
    if (strcmp(argv[2], "undef") != 0)
      hostname = argv[2];
    // here if the parameter is not defined then we make a pair with default
    // values otherwise we make a pair with the true boolean and parameter value
    if (strcmp(argv[3], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[3]));
    if (strcmp(argv[4], "undef") == 0)
      end = std::make_pair(false, 0);
    else
      end = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") == 0)
      fixed = std::make_pair(false, false);
    else
      fixed = std::make_pair(true, atoi(argv[5]));
    if (strcmp(argv[6], "undef") == 0)
      triggeredby = std::make_pair(false, 0);
    else
      triggeredby = std::make_pair(true, atoi(argv[6]));
    if (strcmp(argv[7], "undef") == 0)
      duration = std::make_pair(false, 0);
    else
      duration = std::make_pair(true, atoi(argv[7]));
    if (strcmp(argv[8], "undef") != 0)
      author = argv[8];
    if (strcmp(argv[9], "undef") != 0)
      commentdata = argv[9];

    status =
        client.DeleteHostDowntimeFull(hostname, start, end, fixed, triggeredby,
                                      duration, author, commentdata, &response);
    std::cout << "DeleteHostDowntimeFull" << std::endl;
  } else if (strcmp(argv[1], "DeleteServiceDowntimeFull") == 0) {
    CommandSuccess response;
    std::string hostname;
    std::string svcdsc;
    std::string author;
    std::string commentdata;
    std::pair<bool, uint32_t> start;
    std::pair<bool, uint32_t> end;
    std::pair<bool, uint32_t> triggeredby;
    std::pair<bool, uint32_t> duration;
    std::pair<bool, bool> fixed;

    // parsing parameters. we are checking if some parameters is definied or
    // not. here if this parameter is undefined we send an empty string to the
    // function otherwise we fill the string with the value of the parameter
    if (strcmp(argv[2], "undef") != 0)
      hostname = argv[2];
    if (strcmp(argv[3], "undef") != 0)
      svcdsc = argv[3];
    // here if the parameter is not defined then we make a pair with default
    // values otherwise we make a pair with the true boolean and parameter value
    if (strcmp(argv[4], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") == 0)
      end = std::make_pair(false, 0);
    else
      end = std::make_pair(true, atoi(argv[5]));
    if (strcmp(argv[6], "undef") == 0)
      fixed = std::make_pair(false, false);
    else
      fixed = std::make_pair(true, atoi(argv[6]));
    if (strcmp(argv[7], "undef") == 0)
      triggeredby = std::make_pair(false, 0);
    else
      triggeredby = std::make_pair(true, atoi(argv[7]));
    if (strcmp(argv[8], "undef") == 0)
      duration = std::make_pair(false, 0);
    else
      duration = std::make_pair(true, atoi(argv[8]));
    if (strcmp(argv[9], "undef") != 0)
      author = argv[9];
    if (strcmp(argv[10], "undef") != 0)
      commentdata = argv[10];

    status = client.DeleteServiceDowntimeFull(hostname, svcdsc, start, end,
                                              fixed, triggeredby, duration,
                                              author, commentdata, &response);
    std::cout << "DeleteServiceDowntimeFull" << std::endl;
  } else if (strcmp(argv[1], "DeleteDowntimeByHostName") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::string svcdsc;
    std::string commentdata;
    std::pair<bool, uint32_t> start;
    // hostname must be defined to delete the downtime but not others arguments
    if (strcmp(argv[3], "undef") != 0)
      svcdsc = argv[3];
    if (strcmp(argv[4], "undef") == 0)
      start = std::make_pair(false, 0);
    else
      start = std::make_pair(true, atoi(argv[4]));
    if (strcmp(argv[5], "undef") != 0)
      commentdata = argv[5];

    status = client.DeleteDowntimeByHostName(hostname, svcdsc, start,
                                             commentdata, &response);
    std::cout << "DeleteDowntimeByHostName" << std::endl;
  } else if (strcmp(argv[1], "ScheduleHostCheck") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    uint32_t delaytime = atoi(argv[3]);

    status = client.ScheduleHostCheck(hostname, delaytime, &response);
    std::cout << "ScheduleHostCheck" << std::endl;
  } else if (strcmp(argv[1], "ScheduleHostServiceCheck") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    uint32_t delaytime = atoi(argv[3]);

    status = client.ScheduleHostServiceCheck(hostname, delaytime, &response);
    std::cout << "ScheduleHostServiceCheck" << std::endl;
  } else if (strcmp(argv[1], "ScheduleServiceCheck") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::string servicedesc(argv[3]);
    uint32_t delaytime = atoi(argv[4]);

    status = client.ScheduleServiceCheck(hostname, servicedesc, delaytime,
                                         &response);
    std::cout << "ScheduleServiceCheck" << std::endl;
  } else if (strcmp(argv[1], "SignalProcess") == 0) {
    CommandSuccess response;
    int process = atoi(argv[2]);
    uint32_t scheduledtime = atoi(argv[3]);

    status = client.SignalProcess(process, scheduledtime, &response);
    std::cout << "SignalProcess" << std::endl;
  } else if (strcmp(argv[1], "DelayHostNotification") == 0) {
    if (argc != 5) {
      std::cout
          << "RemoveHostAcknowledgement require arguments : "
             "RemoveHostAcknowledgement [mode] [hostname or id] [delay_time]"
          << std::endl;
      return 1;
    } else if (strcmp(argv[2], "byhostname") == 0) {
      CommandSuccess response;
      std::string hostname(argv[3]);
      uint32_t delaytime = atoi(argv[4]);
      status =
          client.DelayHostNotificationByName(hostname, delaytime, &response);
      std::cout << "DelayHostNotification" << std::endl;
    } else if (strcmp(argv[2], "byhostid") == 0) {
      CommandSuccess response;
      uint32_t hostid = atoi(argv[3]);
      uint32_t delaytime = atoi(argv[4]);
      status = client.DelayHostNotificationById(hostid, delaytime, &response);
      std::cout << "DelayHostNotification" << std::endl;
    }
  } else if (strcmp(argv[1], "DelayServiceNotification") == 0) {
    if (argc != 6) {
      std::cout << "RemoveHostAcknowledgement require arguments : "
                   "RemoveHostAcknowledgement [mode] [hostname or id] "
                   "[servicename or id] [delay_time]"
                << std::endl;
      return 1;
    } else if (strcmp(argv[2], "bynames") == 0) {
      CommandSuccess response;
      std::string hostname(argv[3]);
      std::string svcdsc(argv[4]);
      uint32_t delaytime = atoi(argv[5]);
      status = client.DelayServiceNotificationByNames(hostname, svcdsc,
                                                      delaytime, &response);
      std::cout << "DelayServiceNotification" << std::endl;
    } else if (strcmp(argv[2], "byids") == 0) {
      CommandSuccess response;
      uint32_t hostid = atoi(argv[3]);
      uint32_t serviceid = atoi(argv[4]);
      uint32_t delaytime = atoi(argv[5]);
      status = client.DelayServiceNotificationByIds(hostid, serviceid,
                                                    delaytime, &response);
      std::cout << "DelayServiceNotification" << std::endl;
    }
  } else if (strcmp(argv[1], "ChangeHostObjectIntVar") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    uint32_t mode = atoi(argv[3]);
    uint32_t intval = atoi(argv[4]);
    double dval = atof(argv[5]);

    status =
        client.ChangeHostObjectIntVar(hostname, mode, intval, dval, &response);
    std::cout << "ChangeHostObjectIntVar" << std::endl;
  } else if (strcmp(argv[1], "ChangeServiceObjectIntVar") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::string servicedesc(argv[3]);
    uint32_t mode = atoi(argv[4]);
    uint32_t intval = atoi(argv[5]);
    double dval = atof(argv[6]);

    status = client.ChangeServiceObjectIntVar(hostname, servicedesc, mode,
                                              intval, dval, &response);
    std::cout << "ChangeServiceObjectIntVar" << std::endl;
  } else if (strcmp(argv[1], "ChangeContactObjectIntVar") == 0) {
    CommandSuccess response;
    std::string contactname(argv[2]);
    uint32_t mode = atoi(argv[3]);
    uint32_t intval = atoi(argv[4]);
    double dval = atof(argv[5]);

    status = client.ChangeContactObjectIntVar(contactname, mode, intval, dval,
                                              &response);
    std::cout << "ChangeContactObjectIntVar" << std::endl;
  } else if (strcmp(argv[1], "ChangeHostObjectCustomVar") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::string varname(argv[3]);
    std::string varvalue(argv[4]);

    status = client.ChangeHostObjectCustomVar(hostname, varname, varvalue,
                                              &response);
    std::cout << "ChangeHostObjectCustomVar" << std::endl;
  } else if (strcmp(argv[1], "ChangeServiceObjectCustomVar") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::string servicedesc(argv[3]);
    std::string varname(argv[4]);
    std::string varvalue(argv[5]);

    status = client.ChangeServiceObjectCustomVar(hostname, servicedesc, varname,
                                                 varvalue, &response);
    std::cout << "ChangeServiceObjectCustomVar" << std::endl;
  } else if (strcmp(argv[1], "ChangeContactObjectCustomVar") == 0) {
    CommandSuccess response;
    std::string contact(argv[2]);
    std::string varname(argv[3]);
    std::string varvalue(argv[4]);

    status = client.ChangeContactObjectCustomVar(contact, varname, varvalue,
                                                 &response);
    std::cout << "ChangeContactObjectCustomVar" << std::endl;
  } else if (strcmp(argv[1], "ChangeHostObjectCharVar") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    uint32_t mode = atoi(argv[3]);
    std::string charval(argv[4]);

    status = client.ChangeHostObjectCharVar(hostname, mode, charval, &response);
    std::cout << "ChangeHostObjectCharVar " << status << std::endl;
  } else if (strcmp(argv[1], "ChangeServiceObjectCharVar") == 0) {
    CommandSuccess response;
    std::string hostname(argv[2]);
    std::string servicedesc(argv[3]);
    uint32_t mode = atoi(argv[4]);
    std::string charval(argv[5]);

    status = client.ChangeServiceObjectCharVar(hostname, servicedesc, mode,
                                               charval, &response);
    std::cout << "ChangeServiceObjectCharVar " << status << std::endl;
  } else if (strcmp(argv[1], "ChangeContactObjectCharVar") == 0) {
    CommandSuccess response;
    std::string contact(argv[2]);
    uint32_t mode = atoi(argv[3]);
    std::string charval(argv[4]);

    status =
        client.ChangeContactObjectCharVar(contact, mode, charval, &response);
    std::cout << "ChangeContactObjectCharVar " << status << std::endl;
  }

  exit(status);
}
