/*
** Copyright 2012-2013 Centreon
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

#ifndef CCE_PROCESS_LISTENER_HH
#define CCE_PROCESS_LISTENER_HH

#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/process.hh"

CCE_BEGIN()

/**
 *  @class process process_listener.hh "com/centreon/process_listener.hh"
 *  @brief Notify process events.
 *
 *  This class provide interface to notify process events.
 */
class process_listener {
 public:
  virtual ~process_listener() throw() {}
  virtual void data_is_available(process& p) throw() = 0;
  virtual void data_is_available_err(process& p) throw() = 0;
  virtual void finished(process& p) throw() = 0;
};

CCE_END()

#endif  // !CCE_PROCESS_LISTENER_HH
