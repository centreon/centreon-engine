#include <functional>
#include <sys/types.h>
#include <unistd.h>
#include <google/protobuf/util/time_util.h>
#include "engine_impl.hh"
#include "com/centreon/engine/command_manager.hh"
#include "com/centreon/engine/statistics.hh"
#include "com/centreon/engine/version.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine;

/**
 * @brief Return the Engine's version.
 *
 * @param context gRPC context
 * @param  unused
 * @param response A Version object to fill
 *
 * @return Status::OK
 */
grpc::Status engine_impl::GetVersion(grpc::ServerContext* /*context*/,
                               const ::google::protobuf::Empty* /*request*/,
                               Version* response) {
  response->set_major(CENTREON_ENGINE_VERSION_MAJOR);
  response->set_minor(CENTREON_ENGINE_VERSION_MINOR);
  response->set_patch(CENTREON_ENGINE_VERSION_PATCH);
  return grpc::Status::OK;
}

grpc::Status engine_impl::GetStats(grpc::ServerContext* /*context*/,
    const ::google::protobuf::Empty* /*request*/,
    Stats* response) {
  response->mutable_status_file()->set_name(config->status_file());
  time_t now = time(nullptr);
  std::ifstream status_file;
  status_file.open(config->status_file());
  std::string line;
  time_t created;
  while (std::getline(status_file, line)) {
    size_t r = line.find("created=");
    if (r != std::string::npos) {
      created = std::stol(line.c_str() + r + strlen("created="), NULL, 10);
      break;
    }
  }
  com::centreon::engine::statistics& s =
      com::centreon::engine::statistics::instance();
  *response->mutable_status_file()->mutable_age() =
      google::protobuf::util::TimeUtil::SecondsToDuration(now - created);
  *response->mutable_program_status()->mutable_running_time() =
    google::protobuf::util::TimeUtil::SecondsToDuration(now - program_start);
  response->mutable_program_status()->set_pid(s.get_pid());

  buffer_stats stats;
  if (s.get_external_command_buffer_stats(stats)) {
    response->mutable_buffer()->set_used(stats.used);
    response->mutable_buffer()->set_high(stats.high);
    response->mutable_buffer()->set_total(stats.total);
  }
  return grpc::Status::OK;
}

grpc::Status engine_impl::ProcessServiceCheckResult(
    grpc::ServerContext* /*context*/,
    const ServiceCheck* request,
    CommandSuccess* /*response*/) {
  std::string const& host_name = request->host_name();
  if (host_name.empty())
    return grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "host_name must not be empty");

  std::string const& svc_desc = request->svc_desc();
  if (svc_desc.empty())
    return grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "svc_desc must not be empty");

  auto fn = std::bind(&command_manager::process_passive_service_check,
                      &command_manager::instance(),
                    google::protobuf::util::TimeUtil::TimestampToSeconds(request->check_time()),
                      host_name,
                      svc_desc,
                      request->code(),
                      request->output());
  command_manager::instance().enqueue(std::move(fn));

  return grpc::Status::OK;
}
