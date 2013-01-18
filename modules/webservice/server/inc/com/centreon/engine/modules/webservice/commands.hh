/*
** Copyright 2011-2013 Merethis
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

#ifndef CCE_MOD_WS_COMMANDS_HH
#  define CCE_MOD_WS_COMMANDS_HH

#  include <exception>
#  include "com/centreon/engine/logging/logger.hh"
#  include "com/centreon/engine/modules/webservice/sync_lock.hh"

#  define COMMAND_BEGIN(arguments) \
  try { \
    sync_lock thread_safeness; \
    logger(dbg_functions, most) \
      << "Webservice: " << __func__ << "(" << arguments << ")";
#  define COMMAND_END() \
  } \
  catch (std::exception const& e) { \
    logger(log_runtime_error, more) \
      << "Webservice: " << __func__ << " failed: " << e.what(); \
    return (soap_receiver_fault(s, "Runtime error", e.what())); \
  } \
  catch (...) { \
    logger(log_runtime_error, more) \
      << "Webservice: " << __func__ << " failed: unknown exception"; \
    return (soap_receiver_fault( \
              s, \
              "Runtime error", \
              "unknown exception")); \
  }

#endif // !CCE_MOD_WS_COMMANDS_HH
