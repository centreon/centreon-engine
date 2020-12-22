#ifndef CCE_ENGINERPC_ENGINERPC_HH
#define CCE_ENGINERPC_ENGINERPC_HH

#include <string>
#include <memory>
#include <grpcpp/server.h>
#include "com/centreon/engine/namespace.hh"
#include "engine_impl.hh"

CCE_BEGIN()
class enginerpc final {
  std::unique_ptr<grpc::Server> _server;
 public:
  enginerpc(const std::string& address, uint16_t port);
  enginerpc() = delete;
  enginerpc(const enginerpc&) = delete;
  ~enginerpc() = default;
  void shutdown();
};

CCE_END()
#endif /* !CCE_ENGINERPC_ENGINE_IMPL_HH */
