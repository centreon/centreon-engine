/*
** Copyright 2011-2013 Centreon
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** For more information : contact@centreon.com
*/

#ifndef CCE_LOGGING_SYSLOGGER_HH
#define CCE_LOGGING_SYSLOGGER_HH

#include <string>
#include "com/centreon/engine/logging/backend.hh"
#include "com/centreon/namespace.hh"

CCE_BEGIN()

namespace logging {
/**
 *  @class syslogger syslogger.hh "com/centreon/logging/syslogger.hh"
 *  @brief Log messages to syslog.
 */
class syslogger : public backend {
 public:
  syslogger(std::string const& id,
            int facility,
            bool is_sync = true,
            bool show_pid = true,
            time_precision show_timestamp = second,
            bool show_thread_id = false);
  ~syslogger() throw();
  void close() throw();
  void log(unsigned long long types,
           unsigned int verbose,
           char const* msg,
           unsigned int size) throw();
  void open();
  void reopen();

 private:
  syslogger(syslogger const& right);
  syslogger& operator=(syslogger const& right);

  int _facility;
  std::string _id;
};
}  // namespace logging

CCE_END()

#endif  // !CCE_LOGGING_SYSLOGGER_HH
