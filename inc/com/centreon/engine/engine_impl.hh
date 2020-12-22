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
                        const GenericString* request,
                        Stats* response) override;
  grpc::Status ProcessServiceCheckResult(grpc::ServerContext* context,
                                         const Check* request,
                                         CommandSuccess* response) override;
  grpc::Status ProcessHostCheckResult(grpc::ServerContext* context,
                                      const Check* request,
                                      CommandSuccess* response) override;
  grpc::Status NewThresholdsFile(grpc::ServerContext* context,
                                 const ThresholdsFile* request,
                                 CommandSuccess* response) override;
  grpc::Status GetHostsCount(grpc::ServerContext* context,
                             const ::google::protobuf::Empty*,
                             GenericValue*) override;
  grpc::Status GetContactsCount(grpc::ServerContext* context,
                                const ::google::protobuf::Empty*,
                                GenericValue*) override;
  grpc::Status GetServicesCount(grpc::ServerContext* context,
                                const ::google::protobuf::Empty*,
                                GenericValue*) override;
  grpc::Status GetServiceGroupsCount(grpc::ServerContext* context,
                                     const ::google::protobuf::Empty*,
                                     GenericValue*) override;
  grpc::Status GetContactGroupsCount(grpc::ServerContext* context,
                                     const ::google::protobuf::Empty*,
                                     GenericValue*) override;
  grpc::Status GetHostGroupsCount(grpc::ServerContext* context,
                                  const ::google::protobuf::Empty*,
                                  GenericValue*) override;
  grpc::Status GetServiceDependenciesCount(grpc::ServerContext* context,
                                           const ::google::protobuf::Empty*,
                                           GenericValue*) override;
  grpc::Status GetHostDependenciesCount(grpc::ServerContext* context,
                                        const ::google::protobuf::Empty*,
                                        GenericValue*) override;
  grpc::Status GetHost(grpc::ServerContext* context,
                       const HostIdentifier* request,
                       EngineHost* response) override;
  grpc::Status GetContact(grpc::ServerContext* context,
                          const ContactIdentifier* request,
                          EngineContact* response) override;
  grpc::Status GetService(grpc::ServerContext* context,
                          const ServiceIdentifier* request,
                          EngineService* response) override;

};

CCE_END()
#endif /* !CCE_ENGINERPC_ENGINE_IMPL_HH */
