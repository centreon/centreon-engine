/*
** Copyright 2011-2014 Merethis
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

#ifndef CCE_CONFIGURATION_APPLIER_LOGGING_HH
#define CCE_CONFIGURATION_APPLIER_LOGGING_HH

#include <string>
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/logging/file.hh"
#include "com/centreon/engine/logging/syslogger.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace configuration {
namespace applier {
/**
 *  @class logging logging.hh
 *  @brief Simple configuration applier for logging class.
 *
 *  Simple configuration applier for logging class.
 */
class logging {
 public:
  void apply(state& config);
  static logging& instance();
  static void load();
  static void unload();

 private:
  logging();
  logging(state& config);
  logging(logging const&);
  ~logging() throw();
  logging& operator=(logging const&);
  void _add_stdout();
  void _add_stderr();
  void _add_syslog();
  void _add_log_file(state const& config);
  void _add_debug(state const& config);
  void _del_syslog();
  void _del_log_file();
  void _del_debug();
  void _del_stdout();
  void _del_stderr();

  engine::logging::file* _debug;
  unsigned long long _debug_level;
  unsigned long _debug_max_size;
  unsigned int _debug_verbosity;
  engine::logging::file* _log;
  engine::logging::file* _stderr;
  engine::logging::file* _stdout;
  engine::logging::syslogger* _syslog;
};
}  // namespace applier
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_APPLIER_LOGGING_HH
