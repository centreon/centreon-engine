/*
** Copyright 2012 Merethis
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

#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/modules/webservice/sync_lock.hh"
#include "soapH.h"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules::webservice;

/**
 *  @brief Directs Engine to process all external commands that are
 *         found in the specified file.
 *
 *  If the "delete_once_processed" option is set, the file will be
 *  deleted once it has been processed. If the "delete_once_processed"
 *  option is not set, the file is left untouched.
 *
 *  @param[in]  s                     SOAP object.
 *  @param[in]  file                  File.
 *  @param[in]  delete_once_processed Set to true to delete file after
 *                                    processing.
 *  @param[out] res                   Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__processFile(
      soap* s,
      std::string file,
      bool delete_once_processed,
      centreonengine__processFileResponse& res) {
  (void)res;

  try {
    // Wait for thread safeness.
    sync_lock thread_safeness;
    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << file << ", "
      << (delete_once_processed ? "delete" : "no delete");

    // Process file.
    process_external_commands_from_file(
      file.c_str(),
      delete_once_processed);
  }
  // Exception handling.
  catch (std::exception const& e) {
    logger(log_runtime_error, more)
      << "Webservice: " << __func__ << " failed: " << e.what();
    return (soap_receiver_fault(s, "Runtime error", e.what()));
  }
  catch (...) {
    logger(log_runtime_error, more)
      << "Webservice: " << __func__ << " failed: unknown exception";
    return (soap_receiver_fault(
              s,
              "Runtime error",
              "unknown exception"));
  }
  return (SOAP_OK);
}
