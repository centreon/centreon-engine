#include "engine_impl.hh"

#include <google/protobuf/util/time_util.h>
#include <sys/types.h>
#include <unistd.h>

#include <functional>

#include "com/centreon/engine/anomalydetection.hh"
#include "com/centreon/engine/command_manager.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/version.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

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

grpc::Status engine_impl::GetStats(grpc::ServerContext* /*contect*/,
                                   const Object* request,
                                   Stats* response) {
  auto fn = std::packaged_task<int(void)>(
      std::bind(&command_manager::get_stats, &command_manager::instance(),
                request->name(), response));
  std::future<int> result = fn.get_future();
  command_manager::instance().enqueue(std::move(fn));
  int res = result.get();
  switch (res) {
    case 0:
      return grpc::Status::OK;
    default:
      return grpc::Status(grpc::StatusCode::UNKNOWN, "Unknown error");
  }
}

grpc::Status engine_impl::ProcessServiceCheckResult(
    grpc::ServerContext* /*context*/,
    const Check* request,
    CommandSuccess* /*response*/) {
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

grpc::Status engine_impl::ProcessHostCheckResult(
    grpc::ServerContext* /*context*/,
    const Check* request,
    CommandSuccess* /*response*/) {
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
grpc::Status engine_impl::NewThresholdsFile(grpc::ServerContext* /*context*/,
                                            const ThresholdsFile* request,
                                            CommandSuccess* /*response*/) {
  const std::string& filename = request->filename();
  auto fn = std::packaged_task<int(void)>(
      std::bind(&anomalydetection::update_thresholds, filename));
  command_manager::instance().enqueue(std::move(fn));
  return grpc::Status::OK;
}
