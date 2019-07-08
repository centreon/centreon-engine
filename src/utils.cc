/*
** Copyright 1999-2009      Ethan Galstad
** Copyright 2009-2012      Icinga Development Team (http://www.icinga.org)
** Copyright 2011-2014,2016 Centreon
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

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/broker/compatibility.hh"
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/loop.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/nebmods.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::events;
using namespace com::centreon::engine::logging;

/******************************************************************/
/******************** SYSTEM COMMAND FUNCTIONS ********************/
/******************************************************************/

/* executes a system command - used for notifications, event handlers, etc. */
int my_system_r(
      nagios_macros* mac,
      std::string const& cmd,
      int timeout,
      int* early_timeout,
      double* exectime,
      std::string& output,
      unsigned int max_output_length) {

  logger(dbg_functions, basic)
    << "my_system_r()";

  // initialize return variables.
  *early_timeout = false;
  *exectime = 0.0;

  // if no command was passed, return with no error.
  if (cmd.empty()) {
    return notifier::ok;
  }

  logger(dbg_commands, more)
    << "Running command '" << cmd << "'...";

  timeval start_time = timeval();
  timeval end_time = timeval();

  // time to start command.
  gettimeofday(&start_time, nullptr);

  // send event broker.
  broker_system_command(
    NEBTYPE_SYSTEM_COMMAND_START,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    start_time,
    end_time,
    *exectime,
    timeout,
    *early_timeout,
    notifier::ok,
    const_cast<char *>(cmd.c_str()),
    nullptr,
    nullptr);

  commands::raw raw_cmd("system", cmd);
  commands::result res;
  raw_cmd.run(cmd, *mac, timeout, res);

  end_time.tv_sec = res.end_time.to_seconds();
  end_time.tv_usec
    = res.end_time.to_useconds() - end_time.tv_sec * 1000000ull;
  *exectime = (res.end_time - res.start_time).to_seconds();
  *early_timeout = res.exit_status == process::timeout;
  if (max_output_length > 0)
    output = res.output.substr(0, max_output_length - 1);
  else
    output = res.output;
  int result(res.exit_code);

  logger(dbg_commands, more)
    << com::centreon::logging::setprecision(3)
    << "Execution time=" << *exectime
    << " sec, early timeout=" << *early_timeout
    << ", result=" << result << ", output="
    << output;

  // send event broker.
  broker_system_command(
    NEBTYPE_SYSTEM_COMMAND_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    start_time,
    end_time,
    *exectime,
    timeout,
    *early_timeout,
    result,
    const_cast<char *>(cmd.c_str()),
    const_cast<char *>(output.c_str()),
    nullptr);

  return result;
}

// same like unix ctime without the '\n' at the end of the string.
char const* my_ctime(time_t const* t) {
  char* buf(ctime(t));
  if (buf != nullptr)
    buf[strlen(buf) - 1] = 0;
  return buf;
}

/* given a "raw" command, return the "expanded" or "whole" command line */
int get_raw_command_line_r(
      nagios_macros* mac,
      commands::command* cmd_ptr,
      char const* cmd,
      std::string& full_command,
      int macro_options) {
  char temp_arg[MAX_COMMAND_BUFFER] = "";
  std::string arg_buffer;
  unsigned int x = 0;
  unsigned int y = 0;
  int arg_index = 0;
  int escaped = false;

  logger(dbg_functions, basic)
    << "get_raw_command_line_r()";

  /* clear the argv macros */
  clear_argv_macros_r(mac);

  /* make sure we've got all the requirements */
  if (cmd_ptr == nullptr) {
    return ERROR;
  }

  logger(dbg_commands | dbg_checks | dbg_macros, most)
    << "Raw Command Input: " << cmd_ptr->get_command_line();

  /* get the full command line */
  full_command = cmd_ptr->get_command_line();

  /* get the command arguments */
  if (cmd != nullptr) {
    /* skip the command name (we're about to get the arguments)... */
    for (arg_index = 0;; arg_index++) {
      if (cmd[arg_index] == '!' || cmd[arg_index] == '\x0')
        break;
    }

    /* get each command argument */
    for (x = 0; x < MAX_COMMAND_ARGUMENTS; x++) {
      /* we reached the end of the arguments... */
      if (cmd[arg_index] == '\x0')
        break;

      /* get the next argument */
      /* can't use strtok(), as that's used in process_macros... */
      for (arg_index++, y = 0; y < sizeof(temp_arg) - 1; arg_index++) {

        /* backslashes escape */
        if (cmd[arg_index] == '\\' && escaped == false) {
          escaped = true;
          continue;
        }

        /* end of argument */
        if ((cmd[arg_index] == '!'
             && escaped == false)
            || cmd[arg_index] == '\x0')
          break;

        /* normal of escaped char */
        temp_arg[y] = cmd[arg_index];
        y++;

        /* clear escaped flag */
        escaped = false;
      }
      temp_arg[y] = '\x0';

      /* ADDED 01/29/04 EG */
      /* process any macros we find in the argument */
      process_macros_r(mac, temp_arg, arg_buffer, macro_options);

      mac->argv[x] = arg_buffer;
    }
  }

  logger(dbg_commands | dbg_checks | dbg_macros, most)
      << "Expanded Command Output: " << full_command;

  return OK;
}

/******************************************************************/
/******************** SIGNAL HANDLER FUNCTIONS ********************/
/******************************************************************/

/* trap signals so we can exit gracefully */
void setup_sighandler() {
  /* remove buffering from stderr, stdin, and stdout */
  setbuf(stdin, (char*)nullptr);
  setbuf(stdout, (char*)nullptr);
  setbuf(stderr, (char*)nullptr);

  /* initialize signal handling */
  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, sighandler);
  signal(SIGHUP, sighandler);
}


/* handle signals */
void sighandler(int sig) {
  if (sig < 0)
    sig = -sig;

  int const sigs_size(sizeof(sigs) / sizeof(sigs[0]) - 1);
  sig_id = sig % sigs_size;

  /* we received a SIGHUP */
  if (sig_id == SIGHUP)
    sighup = true;
  /* else begin shutting down... */
  else
    sigshutdown = true;
}

/******************************************************************/
/************************* IPC FUNCTIONS **************************/
/******************************************************************/

/**
 * @brief Parse buffer and fill the three strings given as references:
 *    * short_output
 *    * long_output
 *    * perf_data
 *
 * @param[in] buffer
 * @param[out] short_output
 * @param[out] long_output
 * @param[out] perf_data
 * @param[in] escape_newlines_please To escape new lines in the returned strings
 * @param[in] newlines_are_escaped To consider input newlines as escaped.
 *
 */
void parse_check_output(
      std::string const& buffer,
      std::string& short_buffer,
      std::string& long_buffer,
      std::string& pd_buffer,
      bool escape_newlines_please,
      bool newlines_are_escaped) {
  bool long_pipe{false};
  bool perfdata_already_filled{false};

  bool eof{false};
  std::string line;
  /* pos_line is used to cut a line
   * start_line is the position of the line begin
   * end_line is the position of the line end. */
  size_t start_line{0}, end_line, pos_line;
  int line_number{1};
  while (!eof) {
    if (newlines_are_escaped && (pos_line = buffer.find("\\n", start_line)) != std::string::npos) {
      end_line = pos_line;
      pos_line += 2;
    }
    else if ((pos_line = buffer.find("\n", start_line)) != std::string::npos) {
      end_line = pos_line;
      pos_line++;
    }
    else {
      end_line = buffer.size();
      eof = true;
    }
    line = buffer.substr(start_line, end_line - start_line);
    size_t pipe;
    if (!long_pipe)
      pipe = line.find_last_of('|');
    else
      pipe = std::string::npos;

    if (pipe != std::string::npos) {
      end_line = pipe;
      /* Let's trim the output */
      while (end_line > 1 && std::isspace(line[end_line - 1]))
        end_line--;

      /* Let's trim the output */
      pipe++;
      while (pipe < line.size() - 1 && std::isspace(line[pipe]))
        pipe++;

      if (line_number == 1) {
        short_buffer.append(line.substr(0, end_line));
        pd_buffer.append(line.substr(pipe));
        perfdata_already_filled = true;
      }
      else {
        if (line_number > 2)
          long_buffer.append(escape_newlines_please ? "\\n" : "\n");
        long_buffer.append(line.substr(0, end_line));
        if (perfdata_already_filled)
          pd_buffer.append(" ");
        pd_buffer.append(line.substr(pipe));
        // Now, all new lines contain perfdata.
        long_pipe = true;
      }
    }
    else {
      /* Let's trim the output */
      end_line = line.size();
      while (end_line > 1 && std::isspace(line[end_line - 1]))
        end_line--;
      line.erase(end_line);
      if (line_number == 1)
        short_buffer.append(line);
      else {
        if (!long_pipe) {
          if (line_number > 2)
            long_buffer.append(escape_newlines_please ? "\\n" : "\n");
          long_buffer.append(line);
        }
        else {
          if (perfdata_already_filled)
            pd_buffer.append(" ");
          pd_buffer.append(line);
        }
      }
    }
    start_line = pos_line;
    line_number++;
  }
}

/******************************************************************/
/************************ STRING FUNCTIONS ************************/
/******************************************************************/

/**
 *  @brief Determines whether or not an object name (host, service, etc)
 *  contains illegal characters.
 *
 *  This function uses the global illegal_object_chars variable. This is
 *  caused by the configuration reload mechanism which does not set the
 *  global configuration object itself until the end of the reload.
 *  However during the configuration reload, objects are still checked
 *  for invalid characters.
 *
 *  @param[in] name  Object name.
 *
 *  @return True if the object name contains an illegal character, false
 *          otherwise.
 */
bool contains_illegal_object_chars(char const* name) {
  if (!name || !illegal_object_chars)
    return false;
  return strpbrk(name, illegal_object_chars) ? true : false;
}

/* compares strings */
int compare_strings(char* val1a, char* val2a) {
  /* use the compare_hashdata() function */
  return compare_hashdata(val1a, nullptr, val2a, nullptr);
}

/******************************************************************/
/************************* FILE FUNCTIONS *************************/
/******************************************************************/


/**
 *  Set the close-on-exec flag on the file descriptor.
 *
 *  @param[in] fd The file descriptor to set close on exec.
 *
 *  @return True on succes, otherwise false.
 */
bool set_cloexec(int fd) {
  int flags(0);
  while ((flags = fcntl(fd, F_GETFD)) < 0) {
    if (errno == EINTR)
      continue;
    return false;
  }
  while (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0) {
    if (errno == EINTR)
      continue;
    return false;
  }
  return true;
}

/******************************************************************/
/*********************** CLEANUP FUNCTIONS ************************/
/******************************************************************/

/**
 *  Do some cleanup before we exit.
 */
void cleanup() {
  // Unload modules.
  if (!test_scheduling && !verify_config) {
    neb_free_callback_list();
    neb_unload_all_modules(
      NEBMODULE_FORCE_UNLOAD,
      sigshutdown ? NEBMODULE_NEB_SHUTDOWN : NEBMODULE_NEB_RESTART);
    neb_free_module_list();
    neb_deinit_modules();
  }

  // Free all allocated memory - including macros.
  free_memory(get_global_macros());
}

/**
 *  Free the memory allocated to the linked lists.
 *
 *  @param[in,out] mac Macros.
 */
void free_memory(nagios_macros* mac) {
  // Free memory allocated to comments.
  comment::comments.clear();

  // Free memory allocated to downtimes.
  downtimes::downtime_manager::instance().clear_scheduled_downtimes();

  // Free memory for the high priority event list.
  for (timed_event_list::iterator
         it(timed_event::event_list_high.begin()),
         end(timed_event::event_list_high.end());
       it != end;) {
    //timed_event* next_event(this_event->next);
    if ((*it)->event_type == EVENT_SCHEDULED_DOWNTIME) {
      delete static_cast<unsigned long *>((*it)->event_data);
      (*it)->event_data = nullptr;
    }
    it = timed_event::event_list_high.erase(it);
  }
  quick_timed_event.clear(timed_event::high);

  // Free memory for the low priority event list.
  for (timed_event_list::iterator
         it(timed_event::event_list_low.begin()),
         end(timed_event::event_list_low.end());
       it != end;) {
    if ((*it)->event_type == EVENT_SCHEDULED_DOWNTIME) {
      delete static_cast<unsigned long *>((*it)->event_data);
      (*it)->event_data = nullptr;
    }
    it = timed_event::event_list_low.erase(it);
  }
  quick_timed_event.clear(timed_event::low);

  /*
  ** Free memory associated with macros. It's ok to only free the
  ** volatile ones, as the non-volatile are always free()'d before
  ** assignment if they're set. Doing a full free of them here means
  ** we'll wipe the constant macros when we get a reload or restart
  ** request through the command pipe, or when we receive a SIGHUP.
  */
  clear_volatile_macros_r(mac);
  free_macrox_names();
}
