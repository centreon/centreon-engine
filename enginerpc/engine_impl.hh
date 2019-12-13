#ifndef CCE_ENGINERPC_ENGINE_IMPL_HH
#define CCE_ENGINERPC_ENGINE_IMPL_HH

#include "com/centreon/engine/namespace.hh"
#include "engine.grpc.pb.h"

CCE_BEGIN()
class engine_impl final : public Engine::Service {
  grpc::Status GetVersion(grpc::ServerContext* context,
                          const ::google::protobuf::Empty* /*request*/,
                          Version* response) override;
  grpc::Status GetStats(grpc::ServerContext* context,
                        const ::google::protobuf::Empty* /*request*/,
                        Stats* response) override;
  grpc::Status ProcessServiceCheckResult(
      grpc::ServerContext* context,
      const ServiceCheck* request,
      CommandSuccess* response) override;
};

CCE_END()
#endif /* !CCE_ENGINERPC_ENGINE_IMPL_HH */
