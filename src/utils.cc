/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2012 Icinga Development Team (http://www.icinga.org)
** Copyright 2011-2015 Merethis
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
#include "com/centreon/engine/commands/set.hh"
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
      char* cmd,
      int timeout,
      int* early_timeout,
      double* exectime,
      char** output,
      unsigned int max_output_length) {

  logger(dbg_functions, basic)
    << "my_system_r()";

  // initialize return variables.
  if (output != NULL) {
    *output = NULL;
  }
  *early_timeout = false;
  *exectime = 0.0;

  // if no command was passed, return with no error.
  if (cmd == NULL) {
    return (STATE_OK);
  }

  logger(dbg_commands, more)
    << "Running command '" << cmd << "'...";

  timeval start_time = timeval();
  timeval end_time = timeval();

  // time to start command.
  gettimeofday(&start_time, NULL);

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
    STATE_OK,
    cmd,
    NULL,
    NULL);

  commands::raw raw_cmd("system", cmd);
  commands::result res;
  raw_cmd.run(cmd, *mac, timeout, res);

  end_time.tv_sec = res.end_time.to_seconds();
  end_time.tv_usec
    = res.end_time.to_useconds() - end_time.tv_sec * 1000000ull;
  *exectime = (res.end_time - res.start_time).to_seconds();
  *early_timeout = res.exit_status == process::timeout;
  if (output && max_output_length > 0)
    *output = engine::string::dup(res.output.substr(0, max_output_length - 1));
  int result(res.exit_code);

  logger(dbg_commands, more)
    << com::centreon::logging::setprecision(3)
    << "Execution time=" << *exectime
    << " sec, early timeout=" << *early_timeout
    << ", result=" << result << ", output="
    << (output == NULL ? "(null)" : *output);

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
    cmd,
    (output == NULL ? NULL : *output),
    NULL);

  return (result);
}

// same like unix ctime without the '\n' at the end of the string.
char const* my_ctime(time_t const* t) {
  char* buf(ctime(t));
  if (buf != NULL)
    buf[strlen(buf) - 1] = 0;
  return (buf);
}

/* given a "raw" command, return the "expanded" or "whole" command line */
int get_raw_command_line_r(
      nagios_macros* mac,
      command* cmd_ptr,
      char const* cmd,
      char** full_command,
      int macro_options) {
  char temp_arg[MAX_COMMAND_BUFFER] = "";
  char* arg_buffer = NULL;
  unsigned int x = 0;
  unsigned int y = 0;
  int arg_index = 0;
  int escaped = false;

  logger(dbg_functions, basic)
    << "get_raw_command_line_r()";

  /* clear the argv macros */
  clear_argv_macros_r(mac);

  /* make sure we've got all the requirements */
  if (cmd_ptr == NULL) {
    return (ERROR);
  }

  logger(dbg_commands | dbg_checks | dbg_macros, most)
    << "Raw Command Input: " << cmd_ptr->command_line;

  /* get the full command line */
  if (full_command != NULL) {
    *full_command
      = string::dup(cmd_ptr->command_line ? cmd_ptr->command_line : "");
  }

  /* get the command arguments */
  if (cmd != NULL) {
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
      process_macros_r(mac, temp_arg, &arg_buffer, macro_options);

      mac->argv[x] = arg_buffer;
    }
  }

  if (full_command != NULL) {
    logger(dbg_commands | dbg_checks | dbg_macros, most)
      << "Expanded Command Output: " << *full_command;
  }

  return (OK);
}

/******************************************************************/
/******************** SIGNAL HANDLER FUNCTIONS ********************/
/******************************************************************/

/* trap signals so we can exit gracefully */
void setup_sighandler() {
  /* remove buffering from stderr, stdin, and stdout */
  setbuf(stdin, (char*)NULL);
  setbuf(stdout, (char*)NULL);
  setbuf(stderr, (char*)NULL);

  /* initialize signal handling */
  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, sighandler);
  signal(SIGHUP, sighandler);
  return;
}

/* handle signals */
void sighandler(int sig) {
  caught_signal = true;

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
  return;
}

/******************************************************************/
/************************* IPC FUNCTIONS **************************/
/******************************************************************/

/* frees memory associated with a host/service check result */
int free_check_result(check_result* info) {
  if (info == NULL)
    return (OK);

  delete[] info->host_name;
  delete[] info->service_description;
  delete[] info->output;

  return (OK);
}

/* parse raw plugin output and return: short and long output, perf data */
int parse_check_output(
      char* buf,
      char** short_output,
      char** long_output,
      char** perf_data,
      int escape_newlines_please,
      int newlines_are_escaped) {
  int current_line = 0;
  bool found_newline = false;
  bool eof = false;
  int used_buf = 0;
  int dbuf_chunk = 1024;
  dbuf db1;
  dbuf db2;
  char* ptr = NULL;
  bool in_perf_data = false;
  char* tempbuf = NULL;
  int x = 0;
  int y = 0;

  /* initialize values */
  if (short_output)
    *short_output = NULL;
  if (long_output)
    *long_output = NULL;
  if (perf_data)
    *perf_data = NULL;

  /* nothing to do */
  if (buf == NULL || *buf == 0)
    return (OK);

  used_buf = strlen(buf) + 1;

  /* initialize dynamic buffers (1KB chunk size) */
  dbuf_init(&db1, dbuf_chunk);
  dbuf_init(&db2, dbuf_chunk);

  /* unescape newlines and escaped backslashes first */
  if (newlines_are_escaped) {
    for (x = 0, y = 0; buf[x] != '\x0'; x++) {
      if (buf[x] == '\\' && buf[x + 1] == '\\') {
        x++;
        buf[y++] = buf[x];
      }
      else if (buf[x] == '\\' && buf[x + 1] == 'n') {
        x++;
        buf[y++] = '\n';
      }
      else
        buf[y++] = buf[x];
    }
    buf[y] = '\x0';
  }

  /* process each line of input */
  for (x = 0; !eof; x++) {

    /* we found the end of a line */
    if (buf[x] == '\n')
      found_newline = true;
    else if (buf[x] == '\\' && buf[x + 1] == 'n'
             && newlines_are_escaped == true) {
      found_newline = true;
      buf[x] = '\x0';
      x++;
    }
    else if (buf[x] == '\x0') {
      found_newline = true;
      eof = true;
    }
    else
      found_newline = false;

    if (found_newline == true) {

      current_line++;

      /* handle this line of input */
      buf[x] = '\x0';
      tempbuf = string::dup(buf);

      /* first line contains short plugin output and optional perf data */
      if (current_line == 1) {

        /* get the short plugin output */
        if ((ptr = strtok(tempbuf, "|"))) {
          if (short_output)
            *short_output = string::dup(ptr);

          /* get the optional perf data */
          if ((ptr = strtok(NULL, "\n")))
            dbuf_strcat(&db2, ptr);
        }
      }

      /* additional lines contain long plugin output and optional perf data */
      else {

        /* rest of the output is perf data */
        if (in_perf_data) {
          dbuf_strcat(&db2, tempbuf);
          dbuf_strcat(&db2, " ");
        }

        /* we're still in long output */
        else {

          /* perf data separator has been found */
          if (strstr(tempbuf, "|")) {

            /* NOTE: strtok() causes problems if first character of tempbuf='|', so use my_strtok() instead */
            /* get the remaining long plugin output */
            if ((ptr = my_strtok(tempbuf, "|"))) {

              if (current_line > 2)
                dbuf_strcat(&db1, "\n");
              dbuf_strcat(&db1, ptr);

              /* get the perf data */
              if ((ptr = my_strtok(NULL, "\n"))) {
                dbuf_strcat(&db2, ptr);
                dbuf_strcat(&db2, " ");
              }
            }

            /* set the perf data flag */
            in_perf_data = true;
          }

          /* just long output */
          else {
            if (current_line > 2)
              dbuf_strcat(&db1, "\n");
            dbuf_strcat(&db1, tempbuf);
          }
        }
      }

      delete[] tempbuf;
      tempbuf = NULL;

      /* shift data back to front of buffer and adjust counters */
      memmove(
        (void*)&buf[0],
        (void*)&buf[x + 1],
        (size_t)((int)used_buf - x - 1));
      used_buf -= (x + 1);
      buf[used_buf] = '\x0';
      x = -1;
    }
  }

  /* save long output */
  if (long_output && (db1.buf && strcmp(db1.buf, ""))) {
    if (escape_newlines_please == false)
      *long_output = string::dup(db1.buf);
    else {
      /* escape newlines (and backslashes) in long output */
      tempbuf = new char[strlen(db1.buf) * 2 + 1];

      for (x = 0, y = 0; db1.buf[x] != '\x0'; x++) {

        if (db1.buf[x] == '\n') {
          tempbuf[y++] = '\\';
          tempbuf[y++] = 'n';
        }
        else if (db1.buf[x] == '\\') {
          tempbuf[y++] = '\\';
          tempbuf[y++] = '\\';
        }
        else
          tempbuf[y++] = db1.buf[x];
      }

      tempbuf[y] = '\x0';
      *long_output = string::dup(tempbuf);
      delete[] tempbuf;
    }
  }

  /* save perf data */
  if (perf_data && (db2.buf && strcmp(db2.buf, "")))
    *perf_data = string::dup(db2.buf);

  /* strip short output and perf data */
  if (short_output)
    strip(*short_output);
  if (perf_data)
    strip(*perf_data);

  /* free dynamic buffers */
  dbuf_free(&db1);
  dbuf_free(&db2);

  return (OK);
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
int contains_illegal_object_chars(char* name) {
  if (!name || !illegal_object_chars)
    return (false);
  return (strpbrk(name, illegal_object_chars) ? TRUE : FALSE);
}

/******************************************************************/
/******************** DYNAMIC BUFFER FUNCTIONS ********************/
/******************************************************************/

/* initializes a dynamic buffer */
int dbuf_init(dbuf* db, int chunk_size) {
  if (db == NULL)
    return (ERROR);

  db->buf = NULL;
  db->used_size = 0L;
  db->allocated_size = 0L;
  db->chunk_size = chunk_size;

  return (OK);
}

/* frees a dynamic buffer */
int dbuf_free(dbuf* db) {
  if (db == NULL)
    return (ERROR);

  if (db->buf != NULL)
    delete[] db->buf;
  db->buf = NULL;
  db->used_size = 0L;
  db->allocated_size = 0L;

  return (OK);
}

/* dynamically expands a string */
int dbuf_strcat(dbuf* db, char const* buf) {
  if (db == NULL || buf == NULL)
    return (ERROR);

  /* how much memory should we allocate (if any)? */
  unsigned long buflen(strlen(buf));
  unsigned long new_size(db->used_size + buflen + 1);

  /* we need more memory */
  if (db->allocated_size < new_size) {

    unsigned long memory_needed
      = static_cast<unsigned long>((ceil(new_size / db->chunk_size) + 1)
                                   * db->chunk_size);

    /* allocate memory to store old and new string */
    db->buf = resize_string(db->buf, memory_needed);

    /* update allocated size */
    db->allocated_size = memory_needed;

    /* terminate buffer */
    db->buf[db->used_size] = '\x0';
  }

  /* append the new string */
  strcat(db->buf, buf);

  /* update size allocated */
  db->used_size += buflen;

  return (OK);
}

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
    return (false);
  }
  while (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0) {
    if (errno == EINTR)
      continue;
    return (false);
  }
  return (true);
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
  return;
}

/**
 *  Free the memory allocated to the linked lists.
 *
 *  @param[in,out] mac Macros.
 */
void free_memory(nagios_macros* mac) {
  // Free memory for the high priority event list.
  for (timed_event* this_event(event_list_high); this_event;) {
    timed_event* next_event(this_event->next);
    delete this_event;
    this_event = next_event;
  }
  event_list_high = NULL;
  quick_timed_event.clear(hash_timed_event::high);

  // Free memory for the low priority event list.
  for (timed_event* this_event(event_list_low); this_event;) {
    timed_event* next_event(this_event->next);
    delete this_event;
    this_event = next_event;
  }
  event_list_low = NULL;
  quick_timed_event.clear(hash_timed_event::low);

  /*
  ** Free memory associated with macros. It's ok to only free the
  ** volatile ones, as the non-volatile are always free()'d before
  ** assignment if they're set. Doing a full free of them here means
  ** we'll wipe the constant macros when we get a reload or restart
  ** request through the command pipe, or when we receive a SIGHUP.
  */
  clear_volatile_macros_r(mac);
  free_macrox_names();
  return;
}
