#ifndef CCE_ENGINERPC_ENGINERPC_HH
#define CCE_ENGINERPC_ENGINERPC_HH

#include <string>
#include <memory>
#include <thread>
#include <grpcpp/server.h>
#include "com/centreon/engine/namespace.hh"
#include "engine_impl.hh"

CCE_BEGIN()
namespace enginerpc {
class enginerpc final {
  std::unique_ptr<grpc::Server> _server;
  std::thread _thread;
 public:
  enginerpc(const std::string& address, uint16_t port);
  ~enginerpc();
};
} // namespace enginerpc

CCE_END()
#endif /* !CCE_ENGINERPC_ENGINE_IMPL_HH */
