/*
** Copyright 2000-2008 Ethan Galstad
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

#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/xpddefault.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;

static commands::command*        xpddefault_host_perfdata_command_ptr(nullptr);
static commands::command*        xpddefault_service_perfdata_command_ptr(nullptr);

static char*           xpddefault_host_perfdata_file_template(nullptr);
static char*           xpddefault_service_perfdata_file_template(nullptr);

static commands::command*        xpddefault_host_perfdata_file_processing_command_ptr(nullptr);
static commands::command*        xpddefault_service_perfdata_file_processing_command_ptr(nullptr);

static FILE*           xpddefault_host_perfdata_fp(nullptr);
static FILE*           xpddefault_service_perfdata_fp(nullptr);
static int             xpddefault_host_perfdata_fd(-1);
static int             xpddefault_service_perfdata_fd(-1);

static pthread_mutex_t xpddefault_host_perfdata_fp_lock;
static pthread_mutex_t xpddefault_service_perfdata_fp_lock;

/******************************************************************/
/************** INITIALIZATION & CLEANUP FUNCTIONS ****************/
/******************************************************************/

// initializes performance data.
int xpddefault_initialize_performance_data() {
  char* temp_command_name(nullptr);
  commands::command* temp_command(nullptr);

  // reset vars.
  xpddefault_host_perfdata_command_ptr = nullptr;
  xpddefault_service_perfdata_command_ptr = nullptr;
  xpddefault_host_perfdata_file_processing_command_ptr = nullptr;
  xpddefault_service_perfdata_file_processing_command_ptr = nullptr;

  // grab config info from main config file.
  xpddefault_host_perfdata_file_template = string::dup(config->host_perfdata_file_template());
  xpddefault_service_perfdata_file_template = string::dup(config->service_perfdata_file_template());

  // process special chars in templates.
  xpddefault_preprocess_file_templates(xpddefault_host_perfdata_file_template);
  xpddefault_preprocess_file_templates(xpddefault_service_perfdata_file_template);

  // open the performance data files.
  xpddefault_open_host_perfdata_file();
  xpddefault_open_service_perfdata_file();

  // verify that performance data commands are valid.
  if (!config->host_perfdata_command().empty()) {
    char* temp_buffer(string::dup(config->host_perfdata_command()));

    // get the command name, leave any arguments behind.
    temp_command_name = my_strtok(temp_buffer, "!");

    command_map::iterator cmd_found = commands::command::commands.find(
      temp_command_name);

    if (cmd_found == commands::command::commands.end() || !cmd_found->second) {
      logger(log_runtime_warning, basic)
        << "Warning: Host performance command '" << temp_command_name
        << "' was not found - host performance data will not "
        "be processed!";
    }
    else
      xpddefault_host_perfdata_command_ptr = cmd_found->second.get(); // save the command pointer for later.

    delete[] temp_buffer;
  }

  if (!config->service_perfdata_command().empty()) {
    char* temp_buffer(string::dup(config->service_perfdata_command()));

    // get the command name, leave any arguments behind.
    temp_command_name = my_strtok(temp_buffer, "!");

    command_map::iterator cmd_found = commands::command::commands.find(
      temp_command_name);

    if (cmd_found == commands::command::commands.end() || !cmd_found->second) {
      logger(log_runtime_warning, basic)
        << "Warning: Service performance command '" << temp_command_name
        << "' was not found - service performance data will not "
        "be processed!";
    }
    else
      xpddefault_service_perfdata_command_ptr = cmd_found->second.get();

    // free memory.
    delete[] temp_buffer;
  }

  if (!config->host_perfdata_file_processing_command().empty()) {
    char* temp_buffer
      = string::dup(config->host_perfdata_file_processing_command());

    // get the command name, leave any arguments behind.
    temp_command_name = my_strtok(temp_buffer, "!");
    command_map::iterator cmd_found = commands::command::commands.find(
      temp_command_name);

    if (cmd_found == commands::command::commands.end() || !cmd_found->second) {
      logger(log_runtime_warning, basic)
        << "Warning: Host performance file processing command '"
        << temp_command_name << "' was not found - host performance "
        "data file will not be processed!";
    }
    else
      xpddefault_host_perfdata_file_processing_command_ptr = cmd_found->second.get();

    // free memory.
    delete[] temp_buffer;
  }

  if (!config->service_perfdata_file_processing_command().empty()) {
    char* temp_buffer
      = string::dup(config->service_perfdata_file_processing_command());

    // get the command name, leave any arguments behind.
    temp_command_name = my_strtok(temp_buffer, "!");
    command_map::iterator cmd_found = commands::command::commands.find(
      temp_command_name);

    if (cmd_found == commands::command::commands.end() || !cmd_found->second) {
      logger(log_runtime_warning, basic)
        << "Warning: Service performance file processing command '"
        << temp_command_name << "' was not found - service performance "
        "data file will not be processed!";
    }
    else
      xpddefault_service_perfdata_file_processing_command_ptr = cmd_found->second.get();
    // free memory.
    delete[] temp_buffer;
  }

  return OK;
}

// cleans up performance data.
int xpddefault_cleanup_performance_data() {
  // free memory.
  delete[] xpddefault_host_perfdata_file_template;
  delete[] xpddefault_service_perfdata_file_template;

  xpddefault_host_perfdata_file_template = nullptr;
  xpddefault_service_perfdata_file_template = nullptr;

  // close the files.
  xpddefault_close_host_perfdata_file();
  xpddefault_close_service_perfdata_file();

  return OK;
}

/******************************************************************/
/****************** PERFORMANCE DATA FUNCTIONS ********************/
/******************************************************************/

// updates service performance data.
int xpddefault_update_service_performance_data(com::centreon::engine::service* svc) {
  nagios_macros mac;

  /*
   * bail early if we've got nothing to do so we don't spend a lot
   * of time calculating macros that never get used
   */
  if (!svc || svc->get_perf_data().empty())
    return OK;
  if ((!xpddefault_service_perfdata_fp
       || !xpddefault_service_perfdata_file_template)
      && config->service_perfdata_command().empty())
    return OK;

  host* hst{nullptr};
  host_map::const_iterator it(host::hosts.find(svc->get_hostname()));
  if (it != host::hosts.end())
    hst = it->second.get();

  grab_host_macros_r(&mac, hst);
  grab_service_macros_r(&mac, svc);

  // run the performance data command.
  xpddefault_run_service_performance_data_command(&mac, svc);

  // get rid of used memory we won't need anymore.
  clear_argv_macros_r(&mac);

  // update the performance data file.
  xpddefault_update_service_performance_data_file(&mac, svc);

  // now free() it all.
  clear_volatile_macros_r(&mac);

  return OK;
}


// updates host performance data.
int xpddefault_update_host_performance_data(host* hst) {
  nagios_macros mac;

  /*
   * bail early if we've got nothing to do so we don't spend a lot
   * of time calculating macros that never get used
   */
  if (!hst || !hst->get_perf_data().empty())
    return OK;
  if ((!xpddefault_host_perfdata_fp
       || !xpddefault_host_perfdata_file_template)
      && config->host_perfdata_command().empty())
    return OK;

  // set up macros and get to work.
  grab_host_macros_r(&mac, hst);

  // run the performance data command.
  xpddefault_run_host_performance_data_command(&mac, hst);

  // no more commands to run, so we won't need this any more.
  clear_argv_macros_r(&mac);

  // update the performance data file.
  xpddefault_update_host_performance_data_file(&mac, hst);

  // free() all.
  clear_volatile_macros_r(&mac);

  return OK;
}

/******************************************************************/
/************** PERFORMANCE DATA COMMAND FUNCTIONS ****************/
/******************************************************************/

// runs the service performance data command.
int xpddefault_run_service_performance_data_command(
      nagios_macros* mac,
      com::centreon::engine::service* svc) {
  std::string raw_command_line;
  std::string processed_command_line;
  int early_timeout(false);
  double exectime;
  int result(OK);
  int macro_options(STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);

  logger(dbg_functions, basic)
    << "run_service_performance_data_command()";

  if (svc == nullptr)
    return ERROR;

  // we don't have a command.
  if (config->service_perfdata_command().empty())
    return OK;

  // get the raw command line.
  get_raw_command_line_r(
    mac,
    xpddefault_service_perfdata_command_ptr,
    config->service_perfdata_command().c_str(),
    raw_command_line,
    macro_options);
  if (raw_command_line.c_str())
    return ERROR;

  logger(dbg_perfdata, most)
    << "Raw service performance data command line: "
    << raw_command_line;

  // process any macros in the raw command line.
  process_macros_r(
    mac,
    raw_command_line,
    processed_command_line,
    macro_options);
  if (processed_command_line.empty())
    return ERROR;

  logger(dbg_perfdata, most)
    << "Processed service performance data "
    "command line: " << processed_command_line;

  // run the command.
  try {
    std::string tmp;
    my_system_r(
      mac,
      processed_command_line,
      config->perfdata_timeout(),
      &early_timeout,
      &exectime,
      tmp,
      0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: can't execute service performance data command line '"
      << processed_command_line << "' : " << e.what();
  }

  // check to see if the command timed out.
  if (early_timeout == true)
    logger(log_runtime_warning, basic)
      << "Warning: Service performance data command '"
      << processed_command_line << "' for service '"
      << svc->get_description() << "' on host '"
      << svc->get_hostname() << "' timed out after "
      << config->perfdata_timeout() << " seconds";

  return result;
}

// runs the host performance data command.
int xpddefault_run_host_performance_data_command(
      nagios_macros* mac,
      host* hst) {
  std::string raw_command_line;
  std::string processed_command_line;
  int early_timeout(false);
  double exectime;
  int result(OK);
  int macro_options(STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);

  logger(dbg_functions, basic)
    << "run_host_performance_data_command()";

  if (hst == nullptr)
    return ERROR;

  // we don't have a command.
  if (config->host_perfdata_command().empty())
    return OK;

  // get the raw command line.
  get_raw_command_line_r(
    mac,
    xpddefault_host_perfdata_command_ptr,
    config->host_perfdata_command().c_str(),
    raw_command_line,
    macro_options);
  if (raw_command_line.empty())
    return ERROR;

  logger(dbg_perfdata, most)
    << "Raw host performance data command line: " << raw_command_line;

  // process any macros in the raw command line.
  process_macros_r(
    mac,
    raw_command_line,
    processed_command_line,
    macro_options);

  logger(dbg_perfdata, most)
    << "Processed host performance data command line: "
    << processed_command_line;

  // run the command.
  try {
    std::string tmp;
    my_system_r(
      mac,
      processed_command_line,
      config->perfdata_timeout(),
      &early_timeout,
      &exectime,
      tmp,
      0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: can't execute host performance data command line '"
      << processed_command_line << "' : " << e.what();
  }

  if (processed_command_line.empty())
    return ERROR;

  // check to see if the command timed out.
  if (early_timeout == true)
    logger(log_runtime_warning, basic)
      << "Warning: Host performance data command '"
      << processed_command_line << "' for host '" << hst->get_name()
      << "' timed out after " << config->perfdata_timeout()
      << " seconds";

  return result;
}

/******************************************************************/
/**************** FILE PERFORMANCE DATA FUNCTIONS *****************/
/******************************************************************/

// open the host performance data file for writing.
int xpddefault_open_host_perfdata_file() {
  if (!config->host_perfdata_file().empty()) {
    if (config->host_perfdata_file_mode() == configuration::state::mode_pipe) {
      // must open read-write to avoid failure if the other end isn't ready yet.
      xpddefault_host_perfdata_fd
        = open(config->host_perfdata_file().c_str(), O_NONBLOCK | O_RDWR);
      xpddefault_host_perfdata_fp
        = fdopen(xpddefault_host_perfdata_fd, "w");
    }
    else
      xpddefault_host_perfdata_fp
        = fopen(
            config->host_perfdata_file().c_str(),
            (config->host_perfdata_file_mode() == configuration::state::mode_file) ? "w" : "a");

    if (xpddefault_host_perfdata_fp == nullptr) {
      logger(log_runtime_warning, basic)
        << "Warning: File '" << xpddefault_host_perfdata_fp
        << "' could not be opened - host performance data will not "
        "be written to file!";

      return ERROR;
    }
  }

  return OK;
}

// open the service performance data file for writing.
int xpddefault_open_service_perfdata_file() {
  if (!config->service_perfdata_file().empty()) {
    if (config->service_perfdata_file_mode() == configuration::state::mode_pipe) {
      // must open read-write to avoid failure if the other end isn't ready yet.
      xpddefault_service_perfdata_fd
        = open(config->service_perfdata_file().c_str(), O_NONBLOCK | O_RDWR);
      xpddefault_service_perfdata_fp
        = fdopen(xpddefault_service_perfdata_fd, "w");
    }
    else
      xpddefault_service_perfdata_fp =
	fopen(
          config->service_perfdata_file().c_str(),
          (config->service_perfdata_file_mode() == configuration::state::mode_file) ? "w" : "a");

    if (xpddefault_service_perfdata_fp == nullptr) {
      logger(log_runtime_warning, basic)
        << "Warning: File '" << config->service_perfdata_file()
        << "' could not be opened - service performance data will not "
        "be written to file!";

      return ERROR;
    }
  }

  return OK;
}

// close the host performance data file.
int xpddefault_close_host_perfdata_file() {
  if (xpddefault_host_perfdata_fp != nullptr)
    fclose(xpddefault_host_perfdata_fp);
  if (xpddefault_host_perfdata_fd >= 0) {
    close(xpddefault_host_perfdata_fd);
    xpddefault_host_perfdata_fd = -1;
  }

  return OK;
}

// close the service performance data file.
int xpddefault_close_service_perfdata_file() {
  if (xpddefault_service_perfdata_fp != nullptr)
    fclose(xpddefault_service_perfdata_fp);
  if (xpddefault_service_perfdata_fd >= 0) {
    close(xpddefault_service_perfdata_fd);
    xpddefault_service_perfdata_fd = -1;
  }

  return OK;
}

// processes delimiter characters in templates.
void xpddefault_preprocess_file_templates(char* tmpl) {
  if (!tmpl)
    return ;
  char* tmp1{tmpl}, *tmp2{tmpl};

  for (; *tmp1 != 0; tmp1++, tmp2++) {
    if (*tmp1 == '\\') {
      switch (tmp1[1]) {
        case 't':
          *tmp2 = '\t';
          tmp1++;
          break;
        case 'r':
          *tmp2 = '\r';
          tmp1++;
          break;
        case 'n':
          *tmp2 = '\n';
          tmp1++;
          break;
        default:
          *tmp2 = *tmp1;
          break;
      }
    }
    else
      *tmp2 = *tmp1;
  }
  *tmp2 = 0;
}

// updates service performance data file.
int xpddefault_update_service_performance_data_file(
      nagios_macros* mac,
      com::centreon::engine::service* svc) {
  std::string raw_output;
  std::string processed_output;
  int result(OK);

  logger(dbg_functions, basic)
    << "update_service_performance_data_file()";

  if (svc == nullptr)
    return ERROR;

  // we don't have a file to write to.
  if (xpddefault_service_perfdata_fp == nullptr
      || xpddefault_service_perfdata_file_template == nullptr)
    return OK;

  // get the raw line to write.
  raw_output = xpddefault_service_perfdata_file_template;

  logger(dbg_perfdata, most)
    << "Raw service performance data file output: " << raw_output;

  // process any macros in the raw output line.
  process_macros_r(mac, raw_output, processed_output, 0);
  if (processed_output.empty())
    return ERROR;

  logger(dbg_perfdata, most)
    << "Processed service performance data file output: "
    << processed_output;

  // lock, write to and unlock host performance data file.
  pthread_mutex_lock(&xpddefault_service_perfdata_fp_lock);
  fputs(processed_output.c_str(), xpddefault_service_perfdata_fp);
  fputc('\n', xpddefault_service_perfdata_fp);
  fflush(xpddefault_service_perfdata_fp);
  pthread_mutex_unlock(&xpddefault_service_perfdata_fp_lock);

  return result;
}

// updates host performance data file.
int xpddefault_update_host_performance_data_file(
      nagios_macros* mac,
      host* hst) {
  std::string raw_output;
  std::string processed_output;
  int result(OK);

  logger(dbg_functions, basic)
    << "update_host_performance_data_file()";

  if (hst == nullptr)
    return ERROR;

  // we don't have a host perfdata file.
  if (xpddefault_host_perfdata_fp == nullptr
      || xpddefault_host_perfdata_file_template == nullptr)
    return OK;

  // get the raw output.
  raw_output = string::dup(xpddefault_host_perfdata_file_template);

  logger(dbg_perfdata, most)
    << "Raw host performance file output: " << raw_output;

  // process any macros in the raw output.
  process_macros_r(mac, raw_output, processed_output, 0);
  if (processed_output.empty())
    return ERROR;

  logger(dbg_perfdata, most)
    << "Processed host performance data file output: "
    << processed_output;

  // lock, write to and unlock host performance data file.
  pthread_mutex_lock(&xpddefault_host_perfdata_fp_lock);
  fputs(processed_output.c_str(), xpddefault_host_perfdata_fp);
  fputc('\n', xpddefault_host_perfdata_fp);
  fflush(xpddefault_host_perfdata_fp);
  pthread_mutex_unlock(&xpddefault_host_perfdata_fp_lock);

  return result;
}

// periodically process the host perf data file.
int xpddefault_process_host_perfdata_file() {
  std::string raw_command_line;
  std::string processed_command_line;
  int early_timeout(false);
  double exectime(0.0);
  int result(OK);
  int macro_options(STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);
  nagios_macros mac;

  logger(dbg_functions, basic)
    << "process_host_perfdata_file()";

  // we don't have a command.
  if (config->host_perfdata_file_processing_command().empty())
    return OK;

  // get the raw command line.
  get_raw_command_line_r(
    &mac,
    xpddefault_host_perfdata_file_processing_command_ptr,
    config->host_perfdata_file_processing_command().c_str(),
    raw_command_line,
    macro_options);
  if (raw_command_line.empty()) {
    clear_volatile_macros_r(&mac);
    return ERROR;
  }

  logger(dbg_perfdata, most)
    << "Raw host performance data file processing command line: "
    << raw_command_line;

  // process any macros in the raw command line.
  process_macros_r(
    &mac,
    raw_command_line,
    processed_command_line,
    macro_options);
  if (processed_command_line.empty()) {
    clear_volatile_macros_r(&mac);
    return ERROR;
  }

  logger(dbg_perfdata, most)
    << "Processed host performance data file processing command "
    "line: " << processed_command_line;

  // lock and close the performance data file.
  pthread_mutex_lock(&xpddefault_host_perfdata_fp_lock);
  xpddefault_close_host_perfdata_file();

  // run the command.
  try {
    std::string tmp;
    my_system_r(
      &mac,
      processed_command_line,
      config->perfdata_timeout(),
      &early_timeout,
      &exectime,
      tmp,
      0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: can't execute host performance data file processing command "
         "line '"
      << processed_command_line << "' : " << e.what();
  }
  clear_volatile_macros_r(&mac);

  // re-open and unlock the performance data file.
  xpddefault_open_host_perfdata_file();
  pthread_mutex_unlock(&xpddefault_host_perfdata_fp_lock);

  // check to see if the command timed out.
  if (early_timeout == true)
    logger(log_runtime_warning, basic)
      << "Warning: Host performance data file processing command '"
      << processed_command_line << "' timed out after "
      << config->perfdata_timeout() << " seconds";
  return result;
}

// periodically process the service perf data file.
int xpddefault_process_service_perfdata_file() {
  std::string raw_command_line;
  std::string processed_command_line;
  int early_timeout(false);
  double exectime(0.0);
  int result(OK);
  int macro_options(STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);
  nagios_macros mac;

  logger(dbg_functions, basic)
    << "process_service_perfdata_file()";

  // we don't have a command.
  if (config->service_perfdata_file_processing_command().empty())
    return OK;

  // get the raw command line.
  get_raw_command_line_r(
    &mac,
    xpddefault_service_perfdata_file_processing_command_ptr,
    config->service_perfdata_file_processing_command().c_str(),
    raw_command_line,
    macro_options);
  if (raw_command_line.empty()) {
    clear_volatile_macros_r(&mac);
    return ERROR;
  }

  logger(dbg_perfdata, most)
    << "Raw service performance data file processing "
    "command line: " << raw_command_line;

  // process any macros in the raw command line.
  process_macros_r(
    &mac,
    raw_command_line,
    processed_command_line,
    macro_options);
  if (processed_command_line.empty()) {
    clear_volatile_macros_r(&mac);
    return ERROR;
  }

  logger(dbg_perfdata, most)
    << "Processed service performance data file processing "
    "command line: " << processed_command_line;

  // lock and close the performance data file.
  pthread_mutex_lock(&xpddefault_service_perfdata_fp_lock);
  xpddefault_close_service_perfdata_file();

  // run the command.
  try {
    std::string tmp;
    my_system_r(
      &mac,
      processed_command_line,
      config->perfdata_timeout(),
      &early_timeout,
      &exectime,
      tmp,
      0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: can't execute service performance data file processing "
         "command line '"
      << processed_command_line << "' : " << e.what();
  }

  // re-open and unlock the performance data file.
  xpddefault_open_service_perfdata_file();
  pthread_mutex_unlock(&xpddefault_service_perfdata_fp_lock);

  clear_volatile_macros_r(&mac);

  // check to see if the command timed out.
  if (early_timeout == true)
    logger(log_runtime_warning, basic)
      << "Warning: Service performance data file processing command '"
      << processed_command_line << "' timed out after "
      << config->perfdata_timeout() << " seconds";

  // free memory.
  return result;
}
