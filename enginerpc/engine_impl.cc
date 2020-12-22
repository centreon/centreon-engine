#include "com/centreon/engine/engine_impl.hh"

#include <functional>
#include <sys/types.h>
#include <unistd.h>
#include <google/protobuf/util/time_util.h>
#include <future>
#include <algorithm>

#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/version.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/anomalydetection.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/command_manager.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/downtimes/downtime.hh"
#include "com/centreon/engine/downtimes/downtime_finder.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/downtimes/service_downtime.hh"
#include "com/centreon/engine/events/loop.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/hostdependency.hh"
#include "com/centreon/engine/hostgroup.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/engine/servicedependency.hh"
#include "com/centreon/engine/servicegroup.hh"
#include "com/centreon/engine/statistics.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::downtimes;

/**
 * @brief Return the Engine's version.
 *
 * @param context gRPC context
 * @param  unused
 * @param response A Version object to fill
 *
 * @return Status::OK
 */
grpc::Status engine_impl::GetVersion(
    grpc::ServerContext* /*context*/,
    const ::google::protobuf::Empty* /*request*/,
    Version* response) {
  response->set_major(CENTREON_ENGINE_VERSION_MAJOR);
  response->set_minor(CENTREON_ENGINE_VERSION_MINOR);
  response->set_patch(CENTREON_ENGINE_VERSION_PATCH);
  return grpc::Status::OK;
}

grpc::Status engine_impl::GetStats(grpc::ServerContext* context
                                   __attribute__((unused)),
                                   const GenericString* request
                                   __attribute__((unused)),
                                   Stats* response) {
  auto fn = std::packaged_task<int(void)>(
      std::bind(&command_manager::get_stats, &command_manager::instance(),
                request->str_arg(), response));
  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));
  int32_t res = result.get();
  if (res == 0)
    return grpc::Status::OK;
  else
    return grpc::Status(grpc::StatusCode::UNKNOWN, "Unknown error");
}

grpc::Status engine_impl::ProcessServiceCheckResult(grpc::ServerContext* context
                                                    __attribute__((unused)),
                                                    const Check* request,
                                                    CommandSuccess* response
                                                    __attribute__((unused))) {
  std::string const& host_name = request->host_name();
  if (host_name.empty())
    return grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                        "host_name must not be empty");

  std::string const& svc_desc = request->svc_desc();
  if (svc_desc.empty())
    return grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                        "svc_desc must not be empty");

  auto fn = std::packaged_task<int(void)>(
      std::bind(&command_manager::process_passive_service_check,
                &command_manager::instance(),
                google::protobuf::util::TimeUtil::TimestampToSeconds(
                    request->check_time()),
                host_name, svc_desc, request->code(), request->output()));
  command_manager::instance().enqueue(std::move(fn));

  return grpc::Status::OK;
}


grpc::Status engine_impl::ProcessHostCheckResult(grpc::ServerContext* context
                                                 __attribute__((unused)),
                                                 const Check* request,
                                                 CommandSuccess* response
                                                 __attribute__((unused))) {
  std::string const& host_name = request->host_name();
  if (host_name.empty())
    return grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                        "host_name must not be empty");

  auto fn = std::packaged_task<int(void)>(
      std::bind(&command_manager::process_passive_host_check,
                &command_manager::instance(),
                google::protobuf::util::TimeUtil::TimestampToSeconds(
                    request->check_time()),
                host_name, request->code(), request->output()));
  command_manager::instance().enqueue(std::move(fn));

  return grpc::Status::OK;
}


/**
 * @brief When a new file arrives on the centreon server, this command is used
 * to notify engine to update its anomaly detection services with those new
 * thresholds. The update is done by the main loop thread because of concurrent
 * accesses.
 *
 * @param
 * @param request The Protobuf message containing the full name of the
 *                thresholds file.
 * @param
 *
 * @return A grpc::Status::OK if the file is well read, an error otherwise.
 */
grpc::Status engine_impl::NewThresholdsFile(grpc::ServerContext* context
                                            __attribute__((unused)),
                                            const ThresholdsFile* request,
                                            CommandSuccess* response
                                            __attribute__((unused))) {
  const std::string& filename = request->filename();
  auto fn = std::packaged_task<int(void)>(
      std::bind(&anomalydetection::update_thresholds, filename));
  command_manager::instance().enqueue(std::move(fn));
  return grpc::Status::OK;
}

/**
 * @brief Return host informations.
 *
 * @param context gRPC context
 * @param request Host's identifier (it can be a hostname or a hostid)
 * @param response The filled fields
 * 
 *@return Status::OK
 */
grpc::Status engine_impl::GetHost(grpc::ServerContext* context
                                  __attribute__((unused)),
                                 const HostIdentifier* request
                                 __attribute__((unused)),
                                 EngineHost* response) {
  auto fn =
    std::packaged_task<int(void)>([request, host = response]() -> int32_t {
      std::shared_ptr<com::centreon::engine::host> selectedhost;
      /* checking identifier hostname (by name or by id) */
      switch (request->identifier_case()) {
        case HostIdentifier::kName: {
          /* get the host */
          auto ithostname = host::hosts.find(request->name());
          if (ithostname != host::hosts.end())
            selectedhost = ithostname->second;
          else
            return 1;
        } break;
        case HostIdentifier::kId: {
          /* get the host */
          auto ithostid = host::hosts_by_id.find(request->id());
          if (ithostid != host::hosts_by_id.end())
            selectedhost = ithostid->second;
          else
            return 1;
        } break;
        default:
         return 1;
         break;
    }

    host->set_name(selectedhost->get_name());
    host->set_alias(selectedhost->get_alias());
    host->set_address(selectedhost->get_address());
    host->set_check_period(selectedhost->get_check_period());
    host->set_current_state(
    static_cast<EngineHost::State>(selectedhost->get_current_state()));
    host->set_id(selectedhost->get_host_id());
    return 0;
  });

  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));
  int32_t res = result.get();
  if (res == 0)
    return grpc::Status::OK;
  else
    return grpc::Status(grpc::INVALID_ARGUMENT, grpc::string("hostname not found"));
}

/**
* @brief Return contact informations.
*
* @param context gRPC context
* @param request Contact's identifier
* @param response The filled fields
*
* @return Status::OK
**/
grpc::Status engine_impl::GetContact(grpc::ServerContext* context
                                    __attribute__((unused)),
                                    const ContactIdentifier* request,
                                    EngineContact* response) {
  auto fn =
    std::packaged_task<int(void)>([request, contact = response]() -> int32_t {
      std::shared_ptr<com::centreon::engine::contact> selectedcontact;
      /* get the contact by his name */
      auto itcontactname = contact::contacts.find(request->name());
      if (itcontactname != contact::contacts.end())
        selectedcontact = itcontactname->second;
      else
        return 1;
      /* recovering contact's information */
      contact->set_name(selectedcontact->get_name());
      contact->set_alias(selectedcontact->get_alias());
      contact->set_email(selectedcontact->get_email());
      return 0;
  });
  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));
  if (result.get() == 0)
    return grpc::Status::OK;
  else
    return grpc::Status(grpc::INVALID_ARGUMENT,
  grpc::string("contact not found"));
}

/**
* @brief Return service informations.
* 
* @param context gRPC context
* @param request Service's identifier (it can be a hostname & servicename or a
*        hostid & serviceid)
* @param response The filled fields
* 
*@return Status::OK
*/
grpc::Status engine_impl::GetService(grpc::ServerContext* context,
const ServiceIdentifier* request,
EngineService* response) {
  auto fn =
    std::packaged_task<int(void)>([request, service = response]() -> int32_t {
      std::shared_ptr<com::centreon::engine::service> selectedservice;

      /* checking identifier sesrname (by names or by ids) */
      switch (request->identifier_case()) {
        case ServiceIdentifier::kNames: {
          NameIdentifier names = request->names();
          /* get the service */
          auto itservicenames = service::services.find(
          std::make_pair(names.host_name(), names.service_name()));
          if (itservicenames != service::services.end())
            selectedservice = itservicenames->second;
          else
            return 1;
          } break;
        case ServiceIdentifier::kIds: {
          IdIdentifier ids = request->ids();
          /* get the service */
          auto itserviceids = service::services_by_id.find(
          std::make_pair(ids.host_id(), ids.service_id()));
          if (itserviceids != service::services_by_id.end())
            selectedservice = itserviceids->second;
          else
            return 1;
          } break;
          default:
            return 1;
            break;
      }

      /* recovering service's information */
      service->set_host_id(selectedservice->get_host_id());
      service->set_service_id(selectedservice->get_service_id());
      service->set_host_name(selectedservice->get_hostname());
      service->set_description(selectedservice->get_description());
      service->set_check_period(selectedservice->get_check_period());
      service->set_current_state(static_cast<EngineService::State>(
      selectedservice->get_current_state()));
      return 0;
    });

  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));

  if (result.get() == 0)
    return grpc::Status::OK;
  else
    return grpc::Status(grpc::INVALID_ARGUMENT, grpc::string("service not found"));
}

/**
* @brief Return the total number of hosts.
*
* @param context gRPC context
* @param unused
* @param response Map size
* 
*@return Status::OK
*/
grpc::Status engine_impl::GetHostsCount(grpc::ServerContext* context
                                        __attribute__((unused)),
                                        const ::google::protobuf::Empty* request
                                        __attribute__((unused)),
                                        GenericValue* response) {
  auto fn = std::packaged_task<int32_t(void)>(
    []() -> int32_t { return host::hosts.size(); });

  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));


  response->set_value(result.get());

  return grpc::Status::OK;
}

/**
* @brief Return the total number of contacts.
* 
* @param context gRPC context
* @param unused
* @param response Map size
* 
* @return Status::OK
*/

grpc::Status engine_impl::GetContactsCount(
    grpc::ServerContext* context __attribute__((unused)),
    const ::google::protobuf::Empty* request __attribute__((unused)),
    GenericValue* response) {
  auto fn = std::packaged_task<int32_t(void)>(
    []() -> int32_t { return contact::contacts.size(); });

  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));

  response->set_value(result.get());

  return grpc::Status::OK;
}

/**
* @brief Return the total number of services.
* 
* @param context gRPC context
* @param unused
* @param response Map size
* 
* @return Status::OK
*/
grpc::Status engine_impl::GetServicesCount(
    grpc::ServerContext* context __attribute__((unused)),
    const ::google::protobuf::Empty* request __attribute__((unused)),
    GenericValue* response) {
  auto fn = std::packaged_task<int32_t(void)>(
    []() -> int32_t { return service::services.size(); });

  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));

  response->set_value(result.get());
  return grpc::Status::OK;
}


/**
* @brief Return the total number of service groups.
* 
* @param context gRPC context
* @param unused
* @param response Map size
* 
*@return Status::OK
*/
grpc::Status engine_impl::GetServiceGroupsCount(
    grpc::ServerContext* context __attribute__((unused)),
    const ::google::protobuf::Empty* request __attribute__((unused)),
    GenericValue* response) {
  auto fn = std::packaged_task<int32_t(void)>(
    []() -> int32_t { return servicegroup::servicegroups.size(); });
  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));

  response->set_value(result.get());
  return grpc::Status::OK;
}

/**
* @brief Return the total number of contact groups.
* 
* @param context gRPC context
* @param unused
* @param response Map size
* 
* @return Status::OK
*/
grpc::Status engine_impl::GetContactGroupsCount(
    grpc::ServerContext* context __attribute__((unused)),
    const ::google::protobuf::Empty* request __attribute__((unused)),
    GenericValue* response) {
  auto fn = std::packaged_task<int32_t(void)>(
    []() -> int32_t { return contactgroup::contactgroups.size(); });
  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));

  response->set_value(result.get());
  return grpc::Status::OK;
}

/**
* @brief Return the total number of host groups.
* 
* @param context gRPC context
* @param unused
* @param response Map size
* 
* @return Status::OK
*/
grpc::Status engine_impl::GetHostGroupsCount(
    grpc::ServerContext* context __attribute__((unused)),
    const ::google::protobuf::Empty* request __attribute__((unused)),
    GenericValue* response) {
  auto fn = std::packaged_task<int32_t(void)>(
    []() -> int32_t { return hostgroup::hostgroups.size(); });

  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));

  response->set_value(result.get());
  return grpc::Status::OK;
}

/**
* @brief Return the total number of service dependencies.
* 
* @param context gRPC context
* @param unused
* @param response Map size
* 
* @return Status::OK
*/
grpc::Status engine_impl::GetServiceDependenciesCount(
    grpc::ServerContext* context __attribute__((unused)),
    const ::google::protobuf::Empty* request __attribute__((unused)),
    GenericValue* response) {
  auto fn = std::packaged_task<int32_t(void)>([]() -> int32_t {
    return servicedependency::servicedependencies.size();
  });

  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));

  response->set_value(result.get());
  return grpc::Status::OK;
}

/**
* @brief Return the total number of host dependencies.
* 
* @param context gRPC context
* @param unused
* @param response Map size
* 
* @return Status::OK
*/
grpc::Status engine_impl::GetHostDependenciesCount(
    grpc::ServerContext* context __attribute__((unused)),
    const ::google::protobuf::Empty* request __attribute__((unused)),
    GenericValue* response) {
  auto fn = std::packaged_task<int32_t(void)>(
    []() -> int32_t { return hostdependency::hostdependencies.size(); });
  std::future<int32_t> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));

  response->set_value(result.get());
  return grpc::Status::OK;
}
