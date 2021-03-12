/*
** Copyright 2011-2021 Centreon
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/
#ifndef CCE_LOGGING_BROKER_HH
#define CCE_LOGGING_BROKER_HH

#include <thread>
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/logging/backend.hh"

CCE_BEGIN()

namespace logging {
/**
 *  @class broker broker.hh
 *  @brief Call broker for all logging message.
 *
 *  Call broker for all logging message without debug.
 */
class broker : public com::centreon::logging::backend {
  bool _enable;
  std::thread::id _thread;

 public:
  broker();
  broker(broker const& right);
  ~broker() noexcept override;
  broker& operator=(broker const& right);
  void close() noexcept override final;
  void log(uint64_t types,
           uint32_t verbose,
           char const* msg,
           uint32_t size) noexcept override;
  void open() override final;
  void reopen() override;
};
}  // namespace logging

CCE_END()

#endif  // !CCE_LOGGING_BROKER_HH
