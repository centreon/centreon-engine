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

#ifndef CCE_LOGGING_ENGINE_HH
#define CCE_LOGGING_ENGINE_HH

#include <climits>
#include <vector>
#include "com/centreon/concurrency/mutex.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace logging {
class backend;

/**
 *  @class engine engine.hh "com/centreon/logging/engine.hh"
 *  @brief Logging object manager.
 *
 *  This is an external access point to logging system. Allow to
 *  register backends and write log message into them.
 */
class engine {
 public:
  unsigned long add(backend* obj,
                    unsigned long long types,
                    unsigned int verbose);
  /**
   *  Get the logger engine singleton.
   *
   *  @return The unique instance of logger engine.
   */
  static engine& instance() throw() { static engine instance; return instance; }

  /**
   *  Check if at least one backend can log with this parameter.
   *
   *  @param[in] flag     The logging type to log.
   *  @param[in] verbose  The verbosity level.
   *
   *  @return True if at least one backend can log with this
   *          parameter, otherwise false.
   */
  bool is_log(unsigned long long types, unsigned int verbose) const throw() {
    if (verbose >= sizeof(unsigned int) * CHAR_BIT)
      return (false);
    return (_list_types[verbose] & types);
  }
  void log(unsigned long long types,
           unsigned int verbose,
           char const* msg,
           unsigned int size);
  bool remove(unsigned long id);
  unsigned int remove(backend* obj);
  void reopen();

 private:
  struct backend_info {
    unsigned long id;
    backend* obj;
    unsigned long long types;
    unsigned int verbose;
  };

  engine();
  engine(engine const& right);
  ~engine() throw();
  engine& operator=(engine const& right);
  void _rebuild_types();

  std::vector<backend_info*> _backends;
  unsigned long _id;
  unsigned long long _list_types[sizeof(unsigned int) * CHAR_BIT];
  mutable concurrency::mutex _mtx;
};
}  // namespace logging

CCE_END()

#endif  // !CCE_LOGGING_ENGINE_HH
