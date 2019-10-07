/*
** Copyright 2011-2014 Centreon
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

#ifndef CCE_LOGGING_FILE_HH
#define CCE_LOGGING_FILE_HH

#include <cstdio>
#include <string>
#include "com/centreon/engine/logging/backend.hh"
#include "com/centreon/namespace.hh"

CCE_BEGIN()

namespace logging {
/**
 *  @class file file.hh "com/centreon/logging/file.hh"
 *  @brief Log messages to file.
 */
class file : public backend {
 public:
  file(FILE* file,
       bool is_sync = true,
       bool show_pid = true,
       time_precision show_timestamp = second,
       bool show_thread_id = false,
       long long max_size = 0);
  file(std::string const& path,
       bool is_sync = true,
       bool show_pid = true,
       time_precision show_timestamp = second,
       bool show_thread_id = false,
       long long max_size = 0);
  virtual ~file() throw();
  void close() throw();
  std::string const& filename() const throw();
  void log(unsigned long long types,
           unsigned int verbose,
           char const* msg,
           unsigned int size) throw();
  void open();
  void reopen();

 protected:
  virtual void _max_size_reached();

 private:
  file(file const& right);
  file& operator=(file const& right);
  void _flush() throw();

  long long _max_size;
  std::string _path;
  FILE* _out;
  long long _size;
};
}  // namespace logging

CCE_END()

#endif  // !CCE_LOGGING_FILE_HH
