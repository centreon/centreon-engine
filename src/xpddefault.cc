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
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/service.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/xpddefault.hh"
#include "find.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

static command*        xpddefault_host_perfdata_command_ptr(NULL);
static command*        xpddefault_service_perfdata_command_ptr(NULL);

static char*           xpddefault_host_perfdata_file_template(NULL);
static char*           xpddefault_service_perfdata_file_template(NULL);

static command*        xpddefault_host_perfdata_file_processing_command_ptr(NULL);
static command*        xpddefault_service_perfdata_file_processing_command_ptr(NULL);

static FILE*           xpddefault_host_perfdata_fp(NULL);
static FILE*           xpddefault_service_perfdata_fp(NULL);
static int             xpddefault_host_perfdata_fd(-1);
static int             xpddefault_service_perfdata_fd(-1);

static pthread_mutex_t xpddefault_host_perfdata_fp_lock;
static pthread_mutex_t xpddefault_service_perfdata_fp_lock;

/******************************************************************/
/************** INITIALIZATION & CLEANUP FUNCTIONS ****************/
/******************************************************************/

// initializes performance data.
int xpddefault_initialize_performance_data() {
  char* temp_command_name(NULL);
  command* temp_command(NULL);

  // reset vars.
  xpddefault_host_perfdata_command_ptr = NULL;
  xpddefault_service_perfdata_command_ptr = NULL;
  xpddefault_host_perfdata_file_processing_command_ptr = NULL;
  xpddefault_service_perfdata_file_processing_command_ptr = NULL;

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

    if ((temp_command = find_command(temp_command_name)) == NULL) {
      logger(log_runtime_warning, basic)
        << "Warning: Host performance command '" << temp_command_name
        << "' was not found - host performance data will not "
        "be processed!";
    }
    delete[] temp_buffer;

    // save the command pointer for later.
    xpddefault_host_perfdata_command_ptr = temp_command;
  }

  if (!config->service_perfdata_command().empty()) {
    char* temp_buffer(string::dup(config->service_perfdata_command()));

    // get the command name, leave any arguments behind.
    temp_command_name = my_strtok(temp_buffer, "!");

    if ((temp_command = find_command(temp_command_name)) == NULL) {
      logger(log_runtime_warning, basic)
        << "Warning: Service performance command '" << temp_command_name
        << "' was not found - service performance data will not "
        "be processed!";
    }

    // free memory.
    delete[] temp_buffer;

    // save the command pointer for later.
    xpddefault_service_perfdata_command_ptr = temp_command;
  }

  if (!config->host_perfdata_file_processing_command().empty()) {
    char* temp_buffer
      = string::dup(config->host_perfdata_file_processing_command());

    // get the command name, leave any arguments behind.
    temp_command_name = my_strtok(temp_buffer, "!");
    if ((temp_command = find_command(temp_command_name)) == NULL) {
      logger(log_runtime_warning, basic)
        << "Warning: Host performance file processing command '"
        << temp_command_name << "' was not found - host performance "
        "data file will not be processed!";
    }

    // free memory.
    delete[] temp_buffer;

    // save the command pointer for later.
    xpddefault_host_perfdata_file_processing_command_ptr = temp_command;
  }

  if (!config->service_perfdata_file_processing_command().empty()) {
    char* temp_buffer
      = string::dup(config->service_perfdata_file_processing_command());

    // get the command name, leave any arguments behind.
    temp_command_name = my_strtok(temp_buffer, "!");
    if ((temp_command = find_command(temp_command_name)) == NULL) {
      logger(log_runtime_warning, basic)
        << "Warning: Service performance file processing command '"
        << temp_command_name << "' was not found - service performance "
        "data file will not be processed!";
    }

    // free memory.
    delete[] temp_buffer;

    // save the command pointer for later.
    xpddefault_service_perfdata_file_processing_command_ptr
      = temp_command;
  }

  return (OK);
}

// cleans up performance data.
int xpddefault_cleanup_performance_data() {
  // free memory.
  delete[] xpddefault_host_perfdata_file_template;
  delete[] xpddefault_service_perfdata_file_template;

  xpddefault_host_perfdata_file_template = NULL;
  xpddefault_service_perfdata_file_template = NULL;

  // close the files.
  xpddefault_close_host_perfdata_file();
  xpddefault_close_service_perfdata_file();

  return (OK);
}

/******************************************************************/
/****************** PERFORMANCE DATA FUNCTIONS ********************/
/******************************************************************/

// updates service performance data.
int xpddefault_update_service_performance_data(service* svc) {
  nagios_macros mac;
  host* hst(NULL);

  /*
   * bail early if we've got nothing to do so we don't spend a lot
   * of time calculating macros that never get used
   */
  if (!svc || !svc->perf_data || !*svc->perf_data)
    return (OK);
  if ((!xpddefault_service_perfdata_fp
       || !xpddefault_service_perfdata_file_template)
      && config->service_perfdata_command().empty())
    return (OK);

  /*
   * we know we've got some work to do, so grab the necessary
   * macros and get busy
   */
  memset(&mac, 0, sizeof(mac));
  hst = find_host(svc->host_name);
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

  return (OK);
}


// updates host performance data.
int xpddefault_update_host_performance_data(host* hst) {
  nagios_macros mac;

  /*
   * bail early if we've got nothing to do so we don't spend a lot
   * of time calculating macros that never get used
   */
  if (!hst || !hst->perf_data || !*hst->perf_data)
    return (OK);
  if ((!xpddefault_host_perfdata_fp
       || !xpddefault_host_perfdata_file_template)
      && config->host_perfdata_command().empty())
    return (OK);

  // set up macros and get to work.
  memset(&mac, 0, sizeof(mac));
  grab_host_macros_r(&mac, hst);

  // run the performance data command.
  xpddefault_run_host_performance_data_command(&mac, hst);

  // no more commands to run, so we won't need this any more.
  clear_argv_macros_r(&mac);

  // update the performance data file.
  xpddefault_update_host_performance_data_file(&mac, hst);

  // free() all.
  clear_volatile_macros_r(&mac);

  return (OK);
}

/******************************************************************/
/************** PERFORMANCE DATA COMMAND FUNCTIONS ****************/
/******************************************************************/

// runs the service performance data command.
int xpddefault_run_service_performance_data_command(
      nagios_macros* mac,
      service* svc) {
  char* raw_command_line(NULL);
  char* processed_command_line(NULL);
  int early_timeout(false);
  double exectime;
  int result(OK);
  int macro_options(STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);

  logger(dbg_functions, basic)
    << "run_service_performance_data_command()";

  if (svc == NULL)
    return (ERROR);

  // we don't have a command.
  if (config->service_perfdata_command().empty())
    return (OK);

  // get the raw command line.
  get_raw_command_line_r(
    mac,
    xpddefault_service_perfdata_command_ptr,
    config->service_perfdata_command().c_str(),
    &raw_command_line,
    macro_options);
  if (raw_command_line == NULL)
    return (ERROR);

  logger(dbg_perfdata, most)
    << "Raw service performance data command line: "
    << raw_command_line;

  // process any macros in the raw command line.
  process_macros_r(
    mac,
    raw_command_line,
    &processed_command_line,
    macro_options);
  if (processed_command_line == NULL)
    return (ERROR);

  logger(dbg_perfdata, most)
    << "Processed service performance data "
    "command line: " << processed_command_line;

  // run the command.
  try {
    my_system_r(
      mac,
      processed_command_line,
      config->perfdata_timeout(),
      &early_timeout,
      &exectime,
      NULL,
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
      << svc->description << "' on host '"
      << svc->host_name << "' timed out after "
      << config->perfdata_timeout() << " seconds";

  // free memory.
  delete[] raw_command_line;
  delete[] processed_command_line;

  return (result);
}

// runs the host performance data command.
int xpddefault_run_host_performance_data_command(
      nagios_macros* mac,
      host* hst) {
  char* raw_command_line(NULL);
  char* processed_command_line(NULL);
  int early_timeout(false);
  double exectime;
  int result(OK);
  int macro_options(STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);

  logger(dbg_functions, basic)
    << "run_host_performance_data_command()";

  if (hst == NULL)
    return (ERROR);

  // we don't have a command.
  if (config->host_perfdata_command().empty())
    return (OK);

  // get the raw command line.
  get_raw_command_line_r(
    mac,
    xpddefault_host_perfdata_command_ptr,
    config->host_perfdata_command().c_str(),
    &raw_command_line,
    macro_options);
  if (raw_command_line == NULL)
    return (ERROR);

  logger(dbg_perfdata, most)
    << "Raw host performance data command line: " << raw_command_line;

  // process any macros in the raw command line.
  process_macros_r(
    mac,
    raw_command_line,
    &processed_command_line,
    macro_options);

  logger(dbg_perfdata, most)
    << "Processed host performance data command line: "
    << processed_command_line;

  // run the command.
  try {
    my_system_r(
      mac,
      processed_command_line,
      config->perfdata_timeout(),
      &early_timeout,
      &exectime,
      NULL,
      0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: can't execute host performance data command line '"
      << processed_command_line << "' : " << e.what();
  }

  if (processed_command_line == NULL)
    return (ERROR);

  // check to see if the command timed out.
  if (early_timeout == true)
    logger(log_runtime_warning, basic)
      << "Warning: Host performance data command '"
      << processed_command_line << "' for host '" << hst->name
      << "' timed out after " << config->perfdata_timeout()
      << " seconds";

  // free memory.
  delete[] raw_command_line;
  delete[] processed_command_line;

  return (result);
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

    if (xpddefault_host_perfdata_fp == NULL) {
      logger(log_runtime_warning, basic)
        << "Warning: File '" << xpddefault_host_perfdata_fp
        << "' could not be opened - host performance data will not "
        "be written to file!";

      return (ERROR);
    }
  }

  return (OK);
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

    if (xpddefault_service_perfdata_fp == NULL) {
      logger(log_runtime_warning, basic)
        << "Warning: File '" << config->service_perfdata_file()
        << "' could not be opened - service performance data will not "
        "be written to file!";

      return (ERROR);
    }
  }

  return (OK);
}

// close the host performance data file.
int xpddefault_close_host_perfdata_file() {
  if (xpddefault_host_perfdata_fp != NULL)
    fclose(xpddefault_host_perfdata_fp);
  if (xpddefault_host_perfdata_fd >= 0) {
    close(xpddefault_host_perfdata_fd);
    xpddefault_host_perfdata_fd = -1;
  }

  return (OK);
}

// close the service performance data file.
int xpddefault_close_service_perfdata_file() {
  if (xpddefault_service_perfdata_fp != NULL)
    fclose(xpddefault_service_perfdata_fp);
  if (xpddefault_service_perfdata_fd >= 0) {
    close(xpddefault_service_perfdata_fd);
    xpddefault_service_perfdata_fd = -1;
  }

  return (OK);
}

// processes delimiter characters in templates.
void xpddefault_preprocess_file_templates(char* tmpl) {
  char* tempbuf;
  size_t x(0);
  size_t y(0);

  // allocate temporary buffer.
  tempbuf = new char[strlen(tmpl) + 1];
  strcpy(tempbuf, "");

  for (x = 0, y = 0; x < strlen(tmpl); x++, y++) {
    if (tmpl[x] == '\\') {
      if (tmpl[x + 1] == 't') {
        tempbuf[y] = '\t';
        x++;
      }
      else if (tmpl[x + 1] == 'r') {
        tempbuf[y] = '\r';
        x++;
      }
      else if (tmpl[x + 1] == 'n') {
        tempbuf[y] = '\n';
        x++;
      }
      else
        tempbuf[y] = tmpl[x];
    }
    else
      tempbuf[y] = tmpl[x];
  }
  tempbuf[y] = '\x0';

  strcpy(tmpl, tempbuf);
  delete[] tempbuf;
  return;
}

// updates service performance data file.
int xpddefault_update_service_performance_data_file(
      nagios_macros* mac,
      service* svc) {
  char* raw_output(NULL);
  char* processed_output(NULL);
  int result(OK);

  logger(dbg_functions, basic)
    << "update_service_performance_data_file()";

  if (svc == NULL)
    return (ERROR);

  // we don't have a file to write to.
  if (xpddefault_service_perfdata_fp == NULL
      || xpddefault_service_perfdata_file_template == NULL)
    return (OK);

  // get the raw line to write.
  raw_output = string::dup(xpddefault_service_perfdata_file_template);

  logger(dbg_perfdata, most)
    << "Raw service performance data file output: " << raw_output;

  // process any macros in the raw output line.
  process_macros_r(mac, raw_output, &processed_output, 0);
  if (processed_output == NULL)
    return (ERROR);

  logger(dbg_perfdata, most)
    << "Processed service performance data file output: "
    << processed_output;

  // lock, write to and unlock host performance data file.
  pthread_mutex_lock(&xpddefault_service_perfdata_fp_lock);
  fputs(processed_output, xpddefault_service_perfdata_fp);
  fputc('\n', xpddefault_service_perfdata_fp);
  fflush(xpddefault_service_perfdata_fp);
  pthread_mutex_unlock(&xpddefault_service_perfdata_fp_lock);

  // free memory.
  delete[] raw_output;
  delete[] processed_output;

  return (result);
}

// updates host performance data file.
int xpddefault_update_host_performance_data_file(
      nagios_macros* mac,
      host* hst) {
  char* raw_output(NULL);
  char* processed_output(NULL);
  int result(OK);

  logger(dbg_functions, basic)
    << "update_host_performance_data_file()";

  if (hst == NULL)
    return (ERROR);

  // we don't have a host perfdata file.
  if (xpddefault_host_perfdata_fp == NULL
      || xpddefault_host_perfdata_file_template == NULL)
    return (OK);

  // get the raw output.
  raw_output = string::dup(xpddefault_host_perfdata_file_template);

  logger(dbg_perfdata, most)
    << "Raw host performance file output: " << raw_output;

  // process any macros in the raw output.
  process_macros_r(mac, raw_output, &processed_output, 0);
  if (processed_output == NULL)
    return (ERROR);

  logger(dbg_perfdata, most)
    << "Processed host performance data file output: "
    << processed_output;

  // lock, write to and unlock host performance data file.
  pthread_mutex_lock(&xpddefault_host_perfdata_fp_lock);
  fputs(processed_output, xpddefault_host_perfdata_fp);
  fputc('\n', xpddefault_host_perfdata_fp);
  fflush(xpddefault_host_perfdata_fp);
  pthread_mutex_unlock(&xpddefault_host_perfdata_fp_lock);

  // free memory.
  delete[] raw_output;
  delete[] processed_output;

  return (result);
}

// periodically process the host perf data file.
int xpddefault_process_host_perfdata_file() {
  char* raw_command_line(NULL);
  char* processed_command_line(NULL);
  int early_timeout(false);
  double exectime(0.0);
  int result(OK);
  int macro_options(STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);
  nagios_macros mac;

  logger(dbg_functions, basic)
    << "process_host_perfdata_file()";

  // we don't have a command.
  if (config->host_perfdata_file_processing_command().empty())
    return (OK);

  // init macros.
  memset(&mac, 0, sizeof(mac));

  // get the raw command line.
  get_raw_command_line_r(
    &mac,
    xpddefault_host_perfdata_file_processing_command_ptr,
    config->host_perfdata_file_processing_command().c_str(),
    &raw_command_line,
    macro_options);
  if (raw_command_line == NULL) {
    clear_volatile_macros_r(&mac);
    return (ERROR);
  }

  logger(dbg_perfdata, most)
    << "Raw host performance data file processing command line: "
    << raw_command_line;

  // process any macros in the raw command line.
  process_macros_r(
    &mac,
    raw_command_line,
    &processed_command_line,
    macro_options);
  if (processed_command_line == NULL) {
    clear_volatile_macros_r(&mac);
    return (ERROR);
  }

  logger(dbg_perfdata, most)
    << "Processed host performance data file processing command "
    "line: " << processed_command_line;

  // lock and close the performance data file.
  pthread_mutex_lock(&xpddefault_host_perfdata_fp_lock);
  xpddefault_close_host_perfdata_file();

  // run the command.
  try {
    my_system_r(
      &mac,
      processed_command_line,
      config->perfdata_timeout(),
      &early_timeout,
      &exectime,
      NULL,
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

  // free memory.
  delete[] raw_command_line;
  delete[] processed_command_line;

  return (result);
}

// periodically process the service perf data file.
int xpddefault_process_service_perfdata_file() {
  char* raw_command_line(NULL);
  char* processed_command_line(NULL);
  int early_timeout(false);
  double exectime(0.0);
  int result(OK);
  int macro_options(STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);
  nagios_macros mac;

  logger(dbg_functions, basic)
    << "process_service_perfdata_file()";

  // we don't have a command.
  if (config->service_perfdata_file_processing_command().empty())
    return (OK);

  // init macros.
  memset(&mac, 0, sizeof(mac));

  // get the raw command line.
  get_raw_command_line_r(
    &mac,
    xpddefault_service_perfdata_file_processing_command_ptr,
    config->service_perfdata_file_processing_command().c_str(),
    &raw_command_line,
    macro_options);
  if (raw_command_line == NULL) {
    clear_volatile_macros_r(&mac);
    return (ERROR);
  }

  logger(dbg_perfdata, most)
    << "Raw service performance data file processing "
    "command line: " << raw_command_line;

  // process any macros in the raw command line.
  process_macros_r(
    &mac,
    raw_command_line,
    &processed_command_line,
    macro_options);
  if (processed_command_line == NULL) {
    clear_volatile_macros_r(&mac);
    return (ERROR);
  }

  logger(dbg_perfdata, most)
    << "Processed service performance data file processing "
    "command line: " << processed_command_line;

  // lock and close the performance data file.
  pthread_mutex_lock(&xpddefault_service_perfdata_fp_lock);
  xpddefault_close_service_perfdata_file();

  // run the command.
  try {
    my_system_r(
      &mac,
      processed_command_line,
      config->perfdata_timeout(),
      &early_timeout,
      &exectime,
      NULL,
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
  delete[] raw_command_line;
  delete[] processed_command_line;

  return (result);
}
