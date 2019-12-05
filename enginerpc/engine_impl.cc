#include "engine_impl.hh"
#include "com/centreon/engine/version.hh"

using namespace grpc;
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
Status engine_impl::GetVersion(ServerContext* context,
                               const ::google::protobuf::Empty* /*request*/,
                               Version* response) {
  response->set_major(CENTREON_ENGINE_VERSION_MAJOR);
  response->set_minor(CENTREON_ENGINE_VERSION_MINOR);
  response->set_patch(CENTREON_ENGINE_VERSION_PATCH);
  return Status::OK;
}
