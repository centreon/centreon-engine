/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2012 Icinga Development Team (http://www.icinga.org)
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
#include "com/centreon/engine/comments.hh"
#include "com/centreon/engine/events/loop.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/nebmods.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
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
  if (output && max_output_length > 0) {
    *output = my_strdup(res.output.substr(
                                     0,
                                     max_output_length - 1)
                        .c_str());
  }
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

/*
 * For API compatibility, we must include a my_system() whose
 * signature doesn't include the Centreon Engine_macros variable.
 * NDOUtils uses this. Possibly other modules as well.
 */
int my_system(
      char* cmd,
      int timeout,
      int* early_timeout,
      double* exectime,
      char** output,
      int max_output_length) {
  return (my_system_r(
            get_global_macros(),
            cmd,
            timeout,
            early_timeout,
            exectime,
            output,
            max_output_length));
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
      = my_strdup(cmd_ptr->command_line ? cmd_ptr->command_line : "");
  }

  /* XXX: Crazy indent */
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

/*
 * This function modifies the global macro struct and is thus not
 * threadsafe
 */
int get_raw_command_line(
      command* cmd_ptr,
      char* cmd,
      char** full_command,
      int macro_options) {
  nagios_macros* mac = get_global_macros();
  return (get_raw_command_line_r(
            mac,
            cmd_ptr,
            cmd,
            full_command,
            macro_options));
}

/******************************************************************/
/******************** ENVIRONMENT FUNCTIONS ***********************/
/******************************************************************/

/* sets or unsets an environment variable */
int set_environment_var(char const* name, char const* value, int set) {
  /* we won't mess with null variable names */
  if (name == NULL)
    return (ERROR);

  /* set the environment variable */
  if (set) {
    setenv(name, (value ? value : ""), 1);

    /* needed for Solaris and systems that don't have setenv() */
    /* this will leak memory, but in a "controlled" way, since lost memory should be freed when the child process exits */
    std::string val(name);
    val.append("=").append(value ? value : "");
    char* env_string(my_strdup(val.c_str()));
    putenv(env_string);
  }
  /* clear the variable */
  else {
    unsetenv(name);
  }

  return (OK);
}

/******************************************************************/
/************************* TIME FUNCTIONS *************************/
/******************************************************************/

// #  define TEST_TIMEPERIODS_A 1
// #  define TEST_TIMEPERIODS_B 1

/**
 *  Internal struct time information.
 */
struct   time_info {
  time_t current_time;
  tm     curtime;
  time_t midnight;
  time_t preferred_time;
  tm     preftime;
};

// Checks if the given time is in daylight time saving period.
static int is_dst_time(time_t const* time) {
  tm t;
  localtime_r(time, &t);
  return (t.tm_isdst);
}

// Returns the shift in seconds if the given times are across the
// daylight time saving period change.
static int get_dst_shift(time_t const* start, time_t const* end) {
  int dst_start(is_dst_time(start));
  int dst_end(is_dst_time(end));
  if (dst_start < dst_end)
    return (3600);
  if (dst_start > dst_end)
    return (-3600);
  return (0);
}

/**
 *  Calculate start time and end time for date range calendar date.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_calendar_date_to_time_t(
              daterange const& r,
              time_info const& ti,
              time_t& start,
              time_t& end) {
  (void)ti;

  tm t;
  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  t.tm_isdst = -1;

  t.tm_mday = r.smday;
  t.tm_mon = r.smon;
  t.tm_year = r.syear - 1900;
  if (!(start = mktime(&t)))
    return (false);

  t.tm_mday = r.emday;
  t.tm_mon = r.emon;
  t.tm_year = r.eyear - 1900;
  if (!(end = mktime(&t)))
    return (false);
  return (true);
}

/**
 *  Calculate start time and end time for date range month date.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_month_date_to_time_t(
              daterange const& r,
              time_info const& ti,
              time_t& start,
              time_t& end) {
  // what year should we use?
  int year(std::max(ti.preftime.tm_year, ti.curtime.tm_year));
  // advance an additional year if we already passed
  // the end month date
  if (r.emon < ti.curtime.tm_mon
      || (r.emon == ti.curtime.tm_mon
          && r.emday < ti.curtime.tm_mday))
    ++year;
  start = calculate_time_from_day_of_month(year, r.smon, r.smday);

  // start date was bad.
  if (!start)
    return (false);

  // use same year as was calculated for start time above
  end = calculate_time_from_day_of_month(year, r.emon, r.emday);
  // advance a year if necessary: august 5 - february 2
  if (end < start)
    end = calculate_time_from_day_of_month( ++year, r.emon, r.emday);

  // end date was bad - see if we can handle the error
  if (!end) {
    // end date can't be helped, so skip it
    if (r.emday < 0)
      return (false);
    // else end date slipped past end of month, so use last
    // day of month as end date
    end = calculate_time_from_day_of_month(year, r.emon, -1);
  }
  return (true);
}

/**
 *  Calculate start time and end time for date range month day.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_month_day_to_time_t(
              daterange const& r,
              time_info const& ti,
              time_t& start,
              time_t& end) {
  // what year should we use?
  int year(std::max(ti.preftime.tm_year, ti.curtime.tm_year));
  // use current month
  int month(ti.curtime.tm_mon);
  // advance an additional month (and possibly the year) if
  // we already passed the end day of month
  if (r.emday < ti.curtime.tm_mday) {
    if (month != 11)
      ++month;
    else {
      month = 0;
      ++year;
    }
  }
  start = calculate_time_from_day_of_month(year, month, r.smday);

  // start date was bad.
  if (!start)
    return (false);

  // use same year and month as was calculated
  // for start time above
  end = calculate_time_from_day_of_month(year, month, r.emday + 1);

  // end date was bad - see if we can handle the error
  if (!end) {
    // end date can't be helped, so skip it
    if (r.emday < 0)
      return (false);

    // else end date slipped past end of month, so use last
    // day of month as end date
    end = calculate_time_from_day_of_month(year, month, -1);
  }
  return (true);
}

/**
 *  Calculate start time and end time for date range month week day.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_month_week_day_to_time_t(
              daterange const& r,
              time_info const& ti,
              time_t& start,
              time_t& end) {
  // what year should we use?
  int year(std::max(ti.preftime.tm_year, ti.curtime.tm_year));
  // calculate time of specified weekday of specific month
  start = calculate_time_from_weekday_of_month(
            year,
            r.smon,
            r.swday,
            r.swday_offset);
  // advance to next year if we've passed this month
  // weekday already this year
  if (start < ti.preferred_time)
    start = calculate_time_from_weekday_of_month(
              ++year,
              r.smon,
              r.swday,
              r.swday_offset);

  // start date was bad.
  if (!start)
    return (false);

  // use same year as was calculated for start time above
  end = calculate_time_from_weekday_of_month(
          year,
          r.emon,
          r.ewday,
          r.ewday_offset);
  // advance a year if necessary:
  // thursday 2 august - monday 3 february
  if (end < start)
    end = calculate_time_from_weekday_of_month(
            ++year,
            r.emon,
            r.ewday,
            r.ewday_offset);

  // end date was bad - see if we can handle the error
  if (!end) {
    // end date can't be helped, so skip it
    if (r.ewday_offset < 0)
      return (false);

    // else end date slipped past end of month, so use last day
    // of month as end date
    end = calculate_time_from_day_of_month(
            year,
            ti.preftime.tm_mon,
            -1);
  }
  return (true);
}

/**
 *  Calculate start time and end time for date range week day.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_week_day_to_time_t(
              daterange const& r,
              time_info const& ti,
              time_t& start,
              time_t& end) {
  // what year should we use?
  int year(std::max(ti.preftime.tm_year, ti.curtime.tm_year));

  // calculate time of specified weekday of month
  start = calculate_time_from_weekday_of_month(
            year,
            ti.preftime.tm_mon,
            r.swday,
            r.swday_offset);
  // advance to next month (or year) if we've passed
  // this weekday of this month already
  int month(0);
  if (start < ti.preferred_time) {
    month = ti.preftime.tm_mon;
    if (month != 11)
      ++month;
    else {
      month = 0;
      ++year;
    }
    start = calculate_time_from_weekday_of_month(
              year,
              month,
              r.swday,
              r.swday_offset);
  }

  // start date was bad.
  if (!start)
    return (false);

  // use same year and month as was calculated
  // for start time above
  end = calculate_time_from_weekday_of_month(
          year,
          month,
          r.ewday,
          r.ewday_offset);

  // end date was bad - see if we can handle the error
  if (!end) {
    // end date can't be helped, so skip it
    if (r.ewday_offset < 0)
      return (false);

    // else end date slipped past end of month, so use last day
    // of month as end date
    end = calculate_time_from_day_of_month(year, month, -1);
  }
  return (true);
}

/**
 *  Calculate start time and end time for date range.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  type   Date range type.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_to_time_t(
              daterange const& r,
              unsigned int type,
              time_info const& ti,
              time_t& start,
              time_t& end) {
  typedef bool (*pfunc)(
                   daterange const&,
                   time_info const&,
                   time_t&,
                   time_t&);
  static pfunc tabfunc[] = {
    &_daterange_calendar_date_to_time_t,   // 2009-08-11
    &_daterange_month_date_to_time_t,      // january 1
    &_daterange_month_day_to_time_t,       // day 3
    &_daterange_month_week_day_to_time_t,  // thursday 2 april
    &_daterange_week_day_to_time_t         // wednesday 1
  };

  if (type >= sizeof(tabfunc) / sizeof(*tabfunc))
    return (false);
  if (!(*tabfunc[type])(r, ti, start, end))
    return (false);

#ifdef TEST_TIMEPERIODS_B
  logger(dbg_events, most)
    << "STARTTIME: " << start << " = " << ctime(&start)
    << "ENDTIME1: " << end << " = " << ctime(&end);
#endif // TEST_TIMEPERIODS_B

  // if skipping days...
  if (r.skip_interval > 1) {
    // advance to the next possible skip date
    if (start < ti.preferred_time) {
      // check if interval is across dlst change and
      // gets the compensation
      int shift(get_dst_shift(&start, &ti.midnight));

      // how many days have passed between skip start date
      // and preferred time?
      unsigned long days((shift + ti.midnight
                          - (unsigned long)start) / (3600 * 24));

#ifdef TEST_TIMEPERIODS_B
      logger(dbg_events, most)
        << "MIDNIGHT: " << ti.midnight
        << " = " << ctime(&ti.midnight)
        << (ti.midnight - (unsigned long)start)
        << " SECONDS PASSED\n"
        << days << " DAYS PASSED\n"
        "REMAINDER: " << (days % r.skip_interval)
        << "\nSKIP INTERVAL: " << r.skip_interval
        << "\nDLST SHIFT: " << shift;
#endif // TEST_TIMEPERIODS_B

      // advance start date to next skip day
      if (!(days % r.skip_interval))
        start += (days * 3600 * 24);
      else
        start += ((days - (days % r.skip_interval)
                   + r.skip_interval) * 3600 * 24);
    }

    // if skipping has no end, use start date as end
    if (type == DATERANGE_CALENDAR_DATE
        && is_daterange_single_day(&r))
      end = start;
  }

#ifdef TEST_TIMEPERIODS_B
  logger(dbg_events, most)
    << "\nSTART: " << start << " = " << ctime(&start)
    << "END: " << end << " = " << ctime(&end)
    << "PREFERRED: " << ti.preferred_time
    << " = " << ctime(&ti.preferred_time)
    << "CURRENT: " << ti.current_time
    << " = " << ctime(&ti.current_time);
#endif // TEST_TIMEPERIODS_B

  return (true);
}

// see if the specified time falls into a valid time range in the
// given time period
int check_time_against_period(time_t test_time, timeperiod* tperiod) {
  logger(dbg_functions, basic)
    << "check_time_against_period()";

  // if no period was specified, assume the time is good
  if (!tperiod)
    return (OK);

  // test exclusions first - if exclusions match current time,
  // bail out with an error
  // clear exclusions list before recursing (and restore afterwards)
  // to prevent endless loops...
  timeperiodexclusion* first_exclusion(tperiod->exclusions);
  tperiod->exclusions = NULL;
  for (timeperiodexclusion* exclusion(first_exclusion);
       exclusion;
       exclusion = exclusion->next) {
    if (!check_time_against_period(
          test_time,
          exclusion->timeperiod_ptr)) {
      tperiod->exclusions = first_exclusion;
      return (ERROR);
    }
  }
  tperiod->exclusions = first_exclusion;

  // save values for later
  tm t;
  localtime_r(&test_time, &t);
  int test_time_year(t.tm_year);
  int test_time_mon(t.tm_mon);
  // int test_time_mday(t.tm_mday);
  int test_time_wday(t.tm_wday);

  // calculate the start of the day (midnight, 00:00 hours)
  // when the specified test time occurs
  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  time_t midnight(mktime(&t));

  bool found_match(false);

  // check exceptions first
  for (unsigned int daterange_type(0);
       daterange_type < DATERANGE_TYPES;
       ++daterange_type) {

    for (daterange* drange(tperiod->exceptions[daterange_type]);
         drange;
         drange = drange->next) {

#ifdef TEST_TIMEPERIODS_A
      logger(dbg_events, most)
        << "TYPE: " << daterange_type
        << "\nTEST: " << test_time << " = " << ctime(&test_time)
        << "MIDNIGHT: " << midnight << " = " << ctime(&midnight);
#endif

      time_t start_time(0);
      time_t end_time(0);
      int year(0);

      // get the start time and end time.
      switch (daterange_type) {
      case DATERANGE_CALENDAR_DATE:
        t.tm_sec = 0;
        t.tm_min = 0;
        t.tm_hour = 0;
        t.tm_wday = 0;
        t.tm_isdst = -1;

        t.tm_mday = drange->smday;
        t.tm_mon = drange->smon;
        t.tm_year = drange->syear - 1900;
        if (!(start_time = mktime(&t)))
          continue;

        t.tm_mday = drange->emday;
        t.tm_mon = drange->emon;
        t.tm_year = drange->eyear - 1900;
        if (!(end_time = mktime(&t)))
          continue;
        break;

      case DATERANGE_MONTH_DATE:
        start_time = calculate_time_from_day_of_month(
                       test_time_year,
                       drange->smon,
                       drange->smday);
        if (!start_time)
          continue;

        year = test_time_year;
        end_time = calculate_time_from_day_of_month(
                     year,
                     drange->emon,
                     drange->emday);
        // advance a year if necessary: august 2 - february 5
        if (end_time < start_time)
          end_time = calculate_time_from_day_of_month(
                       ++year,
                       drange->emon,
                       drange->emday);

        // end date was bad - see if we can handle the error
        if (!end_time) {
          // end date can't be helped, so skip it
          if (drange->emday < 0)
            continue;

          // else end date slipped past end of month, so use
          // last day of month as end date
          // use same year calculated above
          end_time = calculate_time_from_day_of_month(
                       year,
                       drange->emon,
                       -1);
        }
        break;

      case DATERANGE_MONTH_DAY:
        start_time = calculate_time_from_day_of_month(
                       test_time_year,
                       test_time_mon,
                       drange->smday);
        if (!start_time)
          continue;

        end_time = calculate_time_from_day_of_month(
                     test_time_year,
                     test_time_mon,
                     drange->emday);

        // end date was bad - see if we can handle the error
        if (!end_time) {
          // end date can't be helped, so skip it
          if (drange->emday < 0)
            continue;

          // else end date slipped past end of month, so use
          // last day of month as end date
          end_time = calculate_time_from_day_of_month(
                       test_time_year,
                       test_time_mon,
                       -1);
        }
        break;

      case DATERANGE_MONTH_WEEK_DAY:
        start_time = calculate_time_from_weekday_of_month(
                       test_time_year,
                       drange->smon,
                       drange->swday,
                       drange->swday_offset);
        if (!start_time)
          continue;

        year = test_time_year;
        end_time = calculate_time_from_weekday_of_month(
                     year,
                     drange->emon,
                     drange->ewday,
                     drange->ewday_offset);
        // advance a year if necessary:
        // thursday 2 august - monday 3 february
        if (end_time < start_time)
          end_time = calculate_time_from_weekday_of_month(
                       ++year,
                       drange->emon,
                       drange->ewday,
                       drange->ewday_offset);

        // end date was bad - see if we can handle the error
        if (!end_time) {
          // end date can't be helped, so skip it
          if (drange->ewday_offset < 0)
            continue;

          // else end date slipped past end of month, so use
          // last day of month as end date
          // use same year calculated above
          end_time = calculate_time_from_day_of_month(
                       year,
                       test_time_mon,
                       -1);
        }
        break;

      case DATERANGE_WEEK_DAY:
        start_time = calculate_time_from_weekday_of_month(
                       test_time_year,
                       test_time_mon,
                       drange->swday,
                       drange->swday_offset);
        if (!start_time)
          continue;

        end_time = calculate_time_from_weekday_of_month(
                     test_time_year,
                     test_time_mon,
                     drange->ewday,
                     drange->ewday_offset);

        // end date was bad - see if we can handle the error
        if (!end_time) {
          // end date can't be helped, so skip it
          if (drange->ewday_offset < 0)
            continue;

          // else end date slipped past end of month, so use
          // last day of month as end date
          end_time = calculate_time_from_day_of_month(
                       test_time_year,
                       test_time_mon,
                       -1);
        }
        break;

      default:
        continue;
      }

#ifdef TEST_TIMEPERIODS_A
      logger(dbg_events, most)
        << "START: " << start_time << " = " << ctime(&start_time)
        << "END: " << end_time << " = " << ctime(&end_time);
#endif

      // calculate skip date start (and end)
      if (drange->skip_interval > 1) {
        // skip start date must be before test time
        if (start_time > test_time)
          continue;

        // check if interval is accress dlst change
        // and gets the compensation.
        int shift(get_dst_shift(&start_time, &midnight));

        // how many days have passed between skip
        // start date and test time?
        unsigned long days((shift + (unsigned long)midnight
                            - (unsigned long)start_time) / (3600 * 24));

        // if test date doesn't fall on a skip
        // interval day, bail out early
        if ((days % drange->skip_interval))
          continue;

        // use midnight of test date as start time
        start_time = midnight;

        // if skipping range has no end, use test date as end
        if (daterange_type == DATERANGE_CALENDAR_DATE
            && is_daterange_single_day(drange))
          end_time = midnight;
      }

#ifdef TEST_TIMEPERIODS_A
      logger(dbg_events, most)
        << "NEW START: " << start_time << " = " << ctime(&start_time)
        << "NEW END: " << end_time << " = " << ctime(&end_time)
        << days << " DAYS PASSED\n"
        "DLST SHIFT: " << shift;
#endif

      // time falls into the range of days
      if (midnight >= start_time && midnight <= end_time)
        found_match = true;

      // found a day match, so see if time ranges are good
      if (found_match) {
        for (timerange* trange(drange->times);
             trange;
             trange = trange->next) {

          // ranges with start/end of zero mean exlude this day
          if (!trange->range_start && !trange->range_end) {
#ifdef TEST_TIMEPERIODS_A
            logger(dbg_events, most)
              << "0 MINUTE RANGE EXCLUSION";
#endif
            continue;
          }

          time_t day_range_start(midnight + trange->range_start);
          time_t day_range_end(midnight + trange->range_end);

#ifdef TEST_TIMEPERIODS_A
          logger(dbg_events, most)
            << "  RANGE START: " << trange->range_start
            << " (" << day_range_start << ") = "
            << ctime(&day_range_start)
            << "  RANGE END: " << trange->range_end
            << " (" << day_range_end << ") = "
            << ctime(&day_range_end);
#endif

          // if the user-specified time falls in this range,
          // return with a positive result
          if (test_time >= day_range_start
              && test_time <= day_range_end)
            return (OK);
        }

        // no match, so bail with error
        return (ERROR);
      }
    }
  }

  // check normal, weekly rotating schedule last

  // check weekday time ranges
  for (timerange* trange(tperiod->days[test_time_wday]);
       trange;
       trange = trange->next) {

    time_t day_range_start(midnight + trange->range_start);
    time_t day_range_end(midnight + trange->range_end);

    // if the user-specified time falls in this range,
    // return with a positive result
    if (test_time >= day_range_start && test_time <= day_range_end)
      return (OK);
  }

  return (ERROR);
}

/**
 *  This function is for timeperiod exclusions,
 *  Icinga special from #459
 *
 *  @param[in]  preferred_time  The preferred time to check.
 *  @param[out] valid_time      Variable to fill.
 *  @param[in]  current_time    The current time.
 *  @param[in]  tperiod         The time period to use.
 */
static void _get_min_invalid_time_per_timeperiod(
       time_t preferred_time,
       time_t* valid_time,
       time_t current_time,
       timeperiod* tperiod) {
  logger(dbg_functions, basic)
    << "get_min_valid_time_per_timeperiod()";

  // if no timeperiod, go with preferred time
  if (!tperiod) {
    *valid_time = preferred_time;
    return;
  }

  time_info ti;
  ti.preferred_time = preferred_time;
  ti.current_time = current_time;
  localtime_r(&current_time, &ti.curtime);

  // calculate the start of the day (midnight, 00:00 hours)
  // of preferred time
  localtime_r(&preferred_time, &ti.preftime);
  ti.preftime.tm_sec = 0;
  ti.preftime.tm_min = 0;
  ti.preftime.tm_hour = 0;
  ti.midnight = mktime(&ti.preftime);

#ifdef TEST_TIMEPERIODS_B
  logger(dbg_events, most)
    << "PREF TIME: " << preferred_time
    << " = " << ctime(&preferred_time)
    << "CURRENT TIME: " << current_time
    << " = " << ctime(&current_time)
    << "PREF YEAR: " << ti.preftime.tm_year
    << ", MON: " << ti.preftime.tm_mon
    << ", MDAY: " << ti.preftime.tm_mday
    << ", WDAY: " << ti.preftime.tm_wday
    << "\nCURRENT YEAR: " << ti.curtime.tm_year
    << ", MON: " << ti.curtime.tm_mon
    << ", MDAY: " << ti.curtime.tm_mday
    << ", WDAY: " << ti.curtime.tm_wday;
#endif // TEST_TIMEPERIODS_B

  bool have_latest_time(false);
  time_t latest_time(0);
  time_t earliest_day(0);

  // check exceptions (in this timeperiod definition) first
  for (unsigned int daterange_type(0);
       daterange_type < DATERANGE_TYPES;
       ++daterange_type) {

#ifdef TEST_TIMEPERIODS_B
    logger(dbg_events, most)
      << "TYPE: " << daterange_type;
#endif // TEST_TIMEPERIODS_B

    for (daterange* drange(tperiod->exceptions[daterange_type]);
         drange;
         drange = drange->next) {

      time_t start_time(0);
      time_t end_time(0);
      if (!_daterange_to_time_t(
             *drange,
             daterange_type,
             ti,
             start_time,
             end_time))
        continue;

      // skip this date range its out of bounds with what we want
      if (preferred_time > end_time)
        continue;

      // how many days at a time should we advance?
      unsigned long advance_interval(1);
      if (drange->skip_interval > 1)
        advance_interval = drange->skip_interval;

      // advance through the date range
      for (time_t day_start(start_time);
           day_start <= end_time;
           day_start += advance_interval * 3600 * 24) {

        // we already found a time from a higher-precendence
        // date range exception
        // here we use have_latest_time instead of have_earliest_time
        if (day_start >= earliest_day && have_latest_time)
          continue;

        for (timerange* trange(drange->times);
             trange;
             trange = trange->next) {

          // REMOVED
          // ranges with start/end of zero mean exlude this day
          // if (!trange->range_start && !trange->range_end)
          //    continue;

          time_t day_range_end(day_start + trange->range_end);
#ifdef TEST_TIMEPERIODS_B
          time_t day_range_start(day_start + trange->range_start);
          logger(dbg_events, most)
            << "  RANGE START: " << trange->range_start << " ("
            << day_range_start << ") = " << ctime(&day_range_start)
            << "  RANGE END: " << trange->range_end
            << " (" << day_range_end << ") = "
            << ctime(&day_range_end);
#endif // TEST_TIMEPERIODS_B

          // range is out of bounds
          if (day_range_end < preferred_time)
            continue;

          // is this the earliest time found thus far?
          if (!have_latest_time || day_range_end < latest_time) {
            // save it as latest_time instead of earliest_time
            have_latest_time = true;
            latest_time = day_range_end;
            earliest_day = day_start;
          }
        }
      }
    }
  }

  // find next available time from normal, weekly rotating schedule
  // (in this timeperiod definition)

  // check a one week rotation of time
  bool has_looped(false);
  for (int weekday(ti.preftime.tm_wday), days_into_the_future(0);
       ;
       ++weekday, ++days_into_the_future) {

    // break out of the loop if we have checked an entire week already
    if (has_looped && weekday >= ti.preftime.tm_wday)
      break;

    if (weekday >= 7) {
      weekday -= 7;
      has_looped = true;
    }

    // calculate start of this future weekday
    time_t day_start((time_t)(ti.midnight + (days_into_the_future * 3600 * 24)));

    // we already found a time from a higher-precendence
    // date range exception
    if (day_start == earliest_day)
      continue;

    // check all time ranges for this day of the week
    for (timerange* trange(tperiod->days[weekday]);
         trange;
         trange = trange->next) {

      // calculate day_range_end to assign to lastest_time again
      time_t day_range_end((time_t)(day_start + trange->range_end));

      if ((!have_latest_time
           || day_range_end < latest_time)
          && day_range_end >= preferred_time) {
        have_latest_time = true;
        latest_time = day_range_end;
        earliest_day = day_start;
      }
    }
  }

  // if we couldn't find a time period there must be none defined
  if (!have_latest_time || !latest_time)
    *valid_time = preferred_time;
  // else use the calculated time
  else
    *valid_time = latest_time;
  return;
}

/**
 *  Get the next valid time within a time period.
 *
 *  @param[in]  preferred_time  The preferred time to check.
 *  @param[out] valid_time      Variable to fill.
 *  @param[in]  current_time    The current time.
 *  @param[in]  tperiod         The time period to use.
 */
void _get_next_valid_time_per_timeperiod(
       time_t preferred_time,
       time_t* valid_time,
       time_t current_time,
       timeperiod* tperiod) {
  logger(dbg_functions, basic)
    << "get_next_valid_time_per_timeperiod()";

  // if no timeperiod, go with preferred time
  if (!tperiod) {
    *valid_time = preferred_time;
    return;
  }

  time_info ti;
  ti.preferred_time = preferred_time;
  ti.current_time = current_time;
  localtime_r(&current_time, &ti.curtime);

  // calculate the start of the day (midnight, 00:00 hours)
  // of preferred time
  localtime_r(&preferred_time, &ti.preftime);
  ti.preftime.tm_sec = 0;
  ti.preftime.tm_min = 0;
  ti.preftime.tm_hour = 0;
  ti.midnight = mktime(&ti.preftime);

#ifdef TEST_TIMEPERIODS_B
  logger(dbg_events, most)
    << "PREF TIME: " << preferred_time
    << " = " << ctime(&preferred_time)
    << "CURRENT TIME: " << current_time
    << " = " << ctime(&current_time)
    << "PREF YEAR: " << ti.preftime.tm_year
    << ", MON: " << ti.preftime.tm_mon
    << ", MDAY: " << ti.preftime.tm_mday
    << ", WDAY: " << ti.preftime.tm_wday
    << "\nCURRENT YEAR: " << ti.curtime.tm_year
    << ", MON: " << ti.curtime.tm_mon
    << ", MDAY: " << ti.curtime.tm_mday
    << ", WDAY: " << ti.curtime.tm_wday;
#endif // TEST_TIMEPERIODS_B

  bool have_earliest_time(false);
  time_t earliest_time(0);
  time_t earliest_day(0);

  // check exceptions (in this timeperiod definition) first
  for (unsigned int daterange_type(0);
       daterange_type < DATERANGE_TYPES;
       ++daterange_type) {

#ifdef TEST_TIMEPERIODS_B
    logger(dbg_events, most)
      << "TYPE: " << daterange_type;
#endif // TEST_TIMEPERIODS_B

    for (daterange* drange(tperiod->exceptions[daterange_type]);
         drange;
         drange = drange->next) {

      time_t start_time(0);
      time_t end_time(0);
      if (!_daterange_to_time_t(
             *drange,
             daterange_type,
             ti,
             start_time,
             end_time))
        continue;

      // skip this date range its out of bounds with what we want
      if (preferred_time > end_time)
        continue;

      // how many days at a time should we advance?
      unsigned long advance_interval(1);
      if (drange->skip_interval > 1)
        advance_interval = drange->skip_interval;

      // advance through the date range
      for (time_t day_start(start_time);
           day_start <= end_time;
           day_start += advance_interval * 3600 * 24) {

        // we already found a time from a
        // higher-precendence date range exception
        if (day_start >= earliest_day && have_earliest_time)
          continue;

        for (timerange* trange(drange->times);
             trange;
             trange = trange->next) {

          // ranges with start/end of zero mean exlude this day
          if (!trange->range_start && !trange->range_end)
            continue;

          time_t day_range_start(day_start + trange->range_start);
          time_t day_range_end(day_start + trange->range_end);

#ifdef TEST_TIMEPERIODS_B
          logger(dbg_events, most)
            << "  RANGE START: " << trange->range_start
            << " (" << day_range_start << ") = "
            << ctime(&day_range_start)
            << "  RANGE END: " << trange->range_end
            << " (" << day_range_end << ") = "
            << ctime(&day_range_end);
#endif

          // range is out of bounds
          if (day_range_end < preferred_time)
            continue;

          // preferred time occurs before range start, so use
          // range start time as earliest potential time
          time_t potential_time(0);
          if (day_range_start >= preferred_time)
            potential_time = day_range_start;
          // preferred time occurs between range start/end, so
          // use preferred time as earliest potential time
          else if (day_range_end >= preferred_time)
            potential_time = preferred_time;

          // is this the earliest time found thus far?
          if (!have_earliest_time || potential_time < earliest_time) {
            have_earliest_time = true;
            earliest_time = potential_time;
            earliest_day = day_start;
#ifdef TEST_TIMEPERIODS_B
            logger(dbg_events, most)
              << "    EARLIEST TIME: " << earliest_time
              << " = " << ctime(&earliest_time);
#endif
          }
        }
      }
    }
  }

  // find next available time from normal, weekly
  // rotating schedule (in this timeperiod definition)

  // check a one week rotation of time
  bool has_looped(false);
  for (int weekday(ti.preftime.tm_wday), days_into_the_future(0);
       ;
       ++weekday, ++days_into_the_future) {

    // break out of the loop if we have checked an entire week already
    if (has_looped && weekday >= ti.preftime.tm_wday)
      break;

    if (weekday >= 7) {
      weekday -= 7;
      has_looped = true;
    }

    // calculate start of this future weekday
    time_t day_start((time_t)(ti.midnight + (days_into_the_future * 3600 * 24)));

    // we already found a time from a higher-precendence
    // date range exception
    if (day_start == earliest_day)
      continue;

    // check all time ranges for this day of the week
    for (timerange* trange(tperiod->days[weekday]);
         trange;
         trange = trange->next) {

      // calculate the time for the start of this time range
      time_t day_range_start(day_start + trange->range_start);

      if ((!have_earliest_time
           || day_range_start < earliest_time)
          && day_range_start >= preferred_time) {
        have_earliest_time = true;
        earliest_time = day_range_start;
        earliest_day = day_start;
      }
    }
  }

  // if we couldn't find a time period there must be none defined
  if (!have_earliest_time || !earliest_time)
    *valid_time = preferred_time;
  // else use the calculated time
  else
    *valid_time = earliest_time;
  return;
}

/**
 *  Calculate the earliest time possible including checks
 *  for timepriod exclusion
 *
 *  @param[in]  preferred_time  The preferred time to check.
 *  @param[out] valid_time      Variable to fill.
 *  @param[in]  current_time    The current time.
 *  @param[in]  tperiod         The time period to use.
 *  @param[in]  level           The recursivity level.
 */
void get_earliest_time(
       time_t preferred_time,
       time_t* valid_time,
       time_t current_time,
       timeperiod* tperiod,
       int level) {
  time_t earliest_time(0);

  /*
   * function can get called recursivly, pushing alternating level
   * see below in timeperiod exclusion loop
   */
  if (!(level % 2)) {
    // initial hit here, and then alternating by 2 the timeperiod exclusions
    _get_next_valid_time_per_timeperiod(
      preferred_time,
      &earliest_time,
      current_time,
      tperiod);
    if (*valid_time == 0)
      *valid_time = earliest_time;
    else if (earliest_time < *valid_time)
      *valid_time = earliest_time;
  }
  else {
    // first timeperiod exclusion hits here, alternating by 2
    _get_min_invalid_time_per_timeperiod(
      preferred_time,
      &earliest_time,
      current_time,
      tperiod);
    if (*valid_time == 0)
      *valid_time = earliest_time;
    else if (earliest_time < *valid_time)
      *valid_time = earliest_time + 1;
  }

  // loop through all available exclusions in this timeperiod and alternate level
  timeperiodexclusion* first_exclusion(tperiod->exclusions);
  tperiod->exclusions = NULL;
  for (timeperiodexclusion* exclusion(first_exclusion);
       exclusion;
       exclusion = exclusion->next)
    get_earliest_time(
      preferred_time,
      valid_time,
      current_time,
      exclusion->timeperiod_ptr,
      level + 1);
  tperiod->exclusions = first_exclusion;
}

/**
 *  Internal functione to get next valid time.
 *
 *  @param[in]  preferred_time  The preferred time to check.
 *  @param[out] valid_time      Variable to fill.
 *  @param[in]  current_time    The current time.
 *  @param[in]  tperiod         The time period to use.
 */
void _get_next_valid_time(
       time_t pref_time,
       time_t current_time,
       time_t* valid_time,
       timeperiod* tperiod) {
  // Moving checks at the top with preferred time
  // in Nagios, all is done within _get_next_valid_time

  // preferred time must be now or in the future
  time_t preferred_time(std::max(pref_time, current_time));

  // if no timeperiod, go with the preferred time
  if (!tperiod) {
    *valid_time = preferred_time;
    return;
  }

  /*
   * if the preferred time is valid in timeperiod, go with it
   * ithis is necessary because the code below won't catch
   * exceptions where peferred day is last (or only) date in
   * timeperiod (date range) and last valid time has already
   * passed.
   * performing this check and bailing out early allows us to
   * skip having to check the next instance of a date range
   * exception or weekday to determine the next valid time
   */
  if (!check_time_against_period(preferred_time, tperiod)) {
#ifdef TEST_TIMEPERIODS_B
    logger(dbg_events, most)
      << "PREF TIME IS VALID";
#endif
    *valid_time = preferred_time;
    return;
  }

  // first check for possible timeperiod excuslions
  // before getting a valid_time
  get_earliest_time(
    preferred_time,
    valid_time,
    current_time,
    tperiod,
    0);
}

/**
 *  Given a preferred time, get the next valid time within a time period
 *
 *  @param[in]  preferred_time  The preferred time to check.
 *  @param[out] valid_time      Variable to fill.
 *  @param[in]  tperiod         The time period to use.
 */
void get_next_valid_time(
       time_t preferred_time,
       time_t* valid_time,
       timeperiod* tperiod) {
  logger(dbg_functions, basic)
    << "get_next_valid_time()";
  // get time right now, preferred time must be now or in the future
  _get_next_valid_time(preferred_time, time(NULL), valid_time, tperiod);
  return;
}

// tests if a date range covers just a single day
int is_daterange_single_day(daterange const* dr) {
  if (!dr)
    return (false);

  if (dr->syear != dr->eyear)
    return (false);
  if (dr->smon != dr->emon)
    return (false);
  if (dr->smday != dr->emday)
    return (false);
  if (dr->swday != dr->ewday)
    return (false);
  if (dr->swday_offset != dr->ewday_offset)
    return (false);

  return (true);
}

// returns a time (midnight) of particular
// (3rd, last) day in a given month
time_t calculate_time_from_day_of_month(
         int year,
         int month,
         int monthday) {
  time_t midnight;
  tm t;

#ifdef TEST_TIMEPERIODS
  logger(dbg_events, most)
    << "YEAR: " << year << ", MON: " << month
    << ", MDAY: " << monthday;
#endif

  // positive day (3rd day)
  if (monthday > 0) {
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_year = year;
    t.tm_mon = month;
    t.tm_mday = monthday;
    t.tm_isdst = -1;

    midnight = mktime(&t);

#ifdef TEST_TIMEPERIODS
    logger(dbg_events, most)
      << "MIDNIGHT CALC: " << ctime(&midnight);
#endif

    // if we rolled over to the next month, time is invalid
    // assume the user's intention is to keep it in the current month
    if (t.tm_mon != month)
      midnight = 0;
  }

  // negative offset (last day, 3rd to last day)
  else {
    // find last day in the month
    int day(32);
    do {
      // back up a day
      --day;

      // make the new time
      t.tm_mon = month;
      t.tm_year = year;
      t.tm_mday = day;
      t.tm_isdst = -1;
      midnight = mktime(&t);
    } while (t.tm_mon != month);

    // now that we know the last day, back up more
    // make the new time
    t.tm_mon = month;
    t.tm_year = year;
    // -1 means last day of month, so add one to
    // make this correct - Mike Bird
    t.tm_mday += (monthday < -30) ? -30 : monthday + 1;
    t.tm_isdst = -1;
    midnight = mktime(&t);

    // if we rolled over to the previous month, time is invalid
    // assume the user's intention is to keep it in the current month
    if (t.tm_mon != month)
      midnight = 0;
  }

  return (midnight);
}

// returns a time (midnight) of particular (3rd, last)
// weekday in a given month
time_t calculate_time_from_weekday_of_month(
         int year,
         int month,
         int weekday,
         int weekday_offset) {
  tm t;
  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  t.tm_year = year;
  t.tm_mon = month;
  t.tm_mday = 1;
  t.tm_isdst = -1;
  time_t midnight(mktime(&t));

  // how many days must we advance to reach the first instance
  // of the weekday this month?
  int days(weekday - (t.tm_wday));
  if (days < 0)
    days += 7;

  // positive offset (3rd thursday)
  int weeks(0);
  if (weekday_offset > 0) {
    // how many weeks must we advance (no more than 5 possible)
    weeks = (weekday_offset > 5) ? 5 : weekday_offset;
    days += ((weeks - 1) * 7);

    // make the new time
    t.tm_mon = month;
    t.tm_year = year;
    t.tm_mday = days + 1;
    t.tm_isdst = -1;
    midnight = mktime(&t);

    // if we rolled over to the next month, time is invalid
    // assume the user's intention is to keep it in the current month
    if (t.tm_mon != month)
      midnight = 0;
  }

  // negative offset (last thursday, 3rd to last tuesday)
  else {
    // find last instance of weekday in the month
    days += (5 * 7);
    do {
      // back up a week
      days -= 7;

      // make the new time
      t.tm_mon = month;
      t.tm_year = year;
      t.tm_mday = days + 1;
      t.tm_isdst = -1;
      midnight = mktime(&t);

    } while (t.tm_mon != month);

    // now that we know the last instance of the weekday, back up more
    weeks = (weekday_offset < -5) ? -5 : weekday_offset;
    days = ((weeks + 1) * 7);

    // make the new time
    t.tm_mon = month;
    t.tm_year = year;
    t.tm_mday += days;
    t.tm_isdst = -1;
    midnight = mktime(&t);

    // if we rolled over to the previous month, time is invalid
    // assume the user's intention is to keep it in the current month
    if (t.tm_mon != month)
      midnight = 0;
  }

  return (midnight);
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

/* reset signal handling... */
void reset_sighandler() {
  /* set signal handling to default actions */
  signal(SIGTERM, SIG_DFL);
  signal(SIGHUP, SIG_DFL);
  signal(SIGPIPE, SIG_DFL);
  return;
}

/* handle signals */
void sighandler(int sig) {
  caught_signal = true;

  if (sig < 0)
    sig = -sig;

  int x;
  for (x = 0; sigs[x]; ++x) {}
  sig %= x;

  sig_id = sig;

  /* we received a SIGHUP */
  if (sig == SIGHUP)
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
  delete[] info->output_file;
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
      tempbuf = my_strdup(buf);

      /* first line contains short plugin output and optional perf data */
      if (current_line == 1) {

        /* get the short plugin output */
        if ((ptr = strtok(tempbuf, "|"))) {
          if (short_output)
            *short_output = my_strdup(ptr);

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
      *long_output = my_strdup(db1.buf);
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
      *long_output = my_strdup(tempbuf);
      delete[] tempbuf;
    }
  }

  /* save perf data */
  if (perf_data && (db2.buf && strcmp(db2.buf, "")))
    *perf_data = my_strdup(db2.buf);

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

/* gets the next string from a buffer in memory - strings are terminated by newlines, which are removed */
char* get_next_string_from_buf(
        char* buf,
        int* start_index,
        int bufsize) {
  char* sptr = NULL;
  char const* nl = "\n";
  int x;

  if (buf == NULL || start_index == NULL)
    return (NULL);
  if (bufsize < 0)
    return (NULL);
  if (*start_index >= (bufsize - 1))
    return (NULL);

  sptr = buf + *start_index;

  /* end of buffer */
  if (sptr[0] == '\x0')
    return (NULL);

  x = strcspn(sptr, nl);
  sptr[x] = '\x0';

  *start_index += x + 1;

  return (sptr);
}

/* determines whether or not an object name (host, service, etc) contains illegal characters */
int contains_illegal_object_chars(char* name) {
  if (name == NULL)
    return (false);

  std::string tmp(name);
  std::string const& illegal_object_chars
    = config->illegal_object_chars();

  if (tmp.find_first_of(illegal_object_chars) == std::string::npos) {
    return (false);
  }
  return (true);
}

/* escapes newlines in a string */
char* escape_newlines(char* rawbuf) {
  char* newbuf = NULL;
  int x, y;

  if (rawbuf == NULL)
    return (NULL);

  /* allocate enough memory to escape all chars if necessary */
  newbuf = new char[strlen(rawbuf) * 2 + 1];

  for (x = 0, y = 0; rawbuf[x] != (char)'\x0'; x++) {

    /* escape backslashes */
    if (rawbuf[x] == '\\') {
      newbuf[y++] = '\\';
      newbuf[y++] = '\\';
    }

    /* escape newlines */
    else if (rawbuf[x] == '\n') {
      newbuf[y++] = '\\';
      newbuf[y++] = 'n';
    }

    else
      newbuf[y++] = rawbuf[x];
  }
  newbuf[y] = '\x0';

  return (newbuf);
}

/* compares strings */
int compare_strings(char* val1a, char* val2a) {
  /* use the compare_hashdata() function */
  return (compare_hashdata(val1a, NULL, val2a, NULL));
}

/******************************************************************/
/************************* FILE FUNCTIONS *************************/
/******************************************************************/

/* renames a file - works across filesystems (Mike Wiacek) */
int my_rename(char const* source, char const* dest) {
  int rename_result = 0;

  /* make sure we have something */
  if (source == NULL || dest == NULL)
    return (-1);

  /* first see if we can rename file with standard function */
  rename_result = rename(source, dest);

  /* handle any errors... */
  if (rename_result == -1) {

    /* an error occurred because the source and dest files are on different filesystems */
    if (errno == EXDEV) {

      /* try copying the file */
      if (my_fcopy(source, dest) == ERROR) {
        logger(log_runtime_error, basic)
          << "Error: Unable to rename file '" << source
          << "' to '" << dest << "': " << strerror(errno);
        return (-1);
      }

      /* delete the original file */
      unlink(source);

      /* reset result since we successfully copied file */
      rename_result = 0;
    }
    /* some other error occurred */
    else {
      logger(log_runtime_error, basic)
        << "Error: Unable to rename file '" << source
        << "' to '" << dest << "': " << strerror(errno);
      return (rename_result);
    }
  }

  return (rename_result);
}

/*
 * copy a file from the path at source to the already opened
 * destination file dest.
 * This is handy when creating tempfiles with mkstemp()
 */
int my_fdcopy(char const* source, char const* dest, int dest_fd) {
  int source_fd, rd_result = 0, wr_result = 0;
  long tot_written = 0, buf_size = 0;
  struct stat st;
  char* buf;

  /* open source file for reading */
  if ((source_fd = open(source, O_RDONLY, 0644)) < 0) {
    logger(log_runtime_error, basic)
      << "Error: Unable to open file '" << source
      << "' for reading: " << strerror(errno);
    return (ERROR);
  }

  /*
   * find out how large the source-file is so we can be sure
   * we've written all of it
   */
  if (fstat(source_fd, &st) < 0) {
    logger(log_runtime_error, basic)
      << "Error: Unable to stat source file '" << source
      << "' for my_fcopy(): " << strerror(errno);
    close(source_fd);
    return (ERROR);
  }

  /*
   * If the file is huge, read it and write it in chunks.
   * This value (128K) is the result of "pick-one-at-random"
   * with some minimal testing and may not be optimal for all
   * hardware setups, but it should work ok for most. It's
   * faster than 1K buffers and 1M buffers, so change at your
   * own peril. Note that it's useful to make it fit in the L2
   * cache, so larger isn't necessarily better.
   */
  buf_size = st.st_size > 128 << 10 ? 128 << 10 : st.st_size;
  try {
    buf = new char[buf_size];
  }
  catch (...) {
    close(source_fd);
    throw;
  }
  /* most of the times, this loop will be gone through once */
  while (tot_written < st.st_size) {
    int loop_wr = 0;

    rd_result = read(source_fd, buf, buf_size);
    if (rd_result < 0) {
      if (errno == EAGAIN || errno == EINTR)
        continue;
      logger(log_runtime_error, basic)
        << "Error: my_fcopy() failed to read from '" << source
        << "': " << strerror(errno);
      break;
    }

    while (loop_wr < rd_result) {
      wr_result = write(dest_fd, buf + loop_wr, rd_result - loop_wr);

      if (wr_result < 0) {
        if (errno == EAGAIN || errno == EINTR)
          continue;
        logger(log_runtime_error, basic)
          << "Error: my_fcopy() failed to write to '" << dest
          << "': " << strerror(errno);
        break;
      }
      loop_wr += wr_result;
    }
    if (wr_result < 0)
      break;
    tot_written += loop_wr;
  }

  /*
   * clean up irregardless of how things went. dest_fd comes from
   * our caller, so we mustn't close it.
   */
  close(source_fd);
  delete[] buf;

  if (rd_result < 0 || wr_result < 0) {
    /* don't leave half-written files around */
    unlink(dest);
    return (ERROR);
  }

  return (OK);
}

/* copies a file */
int my_fcopy(char const* source, char const* dest) {
  int dest_fd, result;

  /* make sure we have something */
  if (source == NULL || dest == NULL)
    return (ERROR);

  /* unlink destination file first (not doing so can cause problems on network file systems like CIFS) */
  unlink(dest);

  /* open destination file for writing */
  if ((dest_fd = open(
                   dest,
                   O_WRONLY | O_TRUNC | O_CREAT | O_APPEND,
                   0644)) < 0) {
    logger(log_runtime_error, basic)
      << "Error: Unable to open file '" << dest
      << "' for writing: " << strerror(errno);
    return (ERROR);
  }

  result = my_fdcopy(source, dest, dest_fd);
  close(dest_fd);
  return (result);
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
  // Free all allocated memory for the object definitions.
  free_object_data();

  // Free memory allocated to comments.
  free_comment_data();

  // Free memory allocated to downtimes.
  free_downtime_data();

  // Free memory for the high priority event list.
  for (timed_event* this_event(event_list_high); this_event;) {
    timed_event* next_event(this_event->next);
    if (this_event->event_type == EVENT_SCHEDULED_DOWNTIME) {
      delete static_cast<unsigned long*>(this_event->event_data);
      this_event->event_data = NULL;
    }
    delete this_event;
    this_event = next_event;
  }
  event_list_high = NULL;
  quick_timed_event.clear(hash_timed_event::high);

  // Free memory for the low priority event list.
  for (timed_event* this_event(event_list_low); this_event;) {
    timed_event* next_event(this_event->next);
    if (this_event->event_type == EVENT_SCHEDULED_DOWNTIME) {
      delete static_cast<unsigned long*>(this_event->event_data);
      this_event->event_data = NULL;
    }
    delete this_event;
    this_event = next_event;
  }
  event_list_low = NULL;
  quick_timed_event.clear(hash_timed_event::low);

  // Free any notification list that may have been overlooked.
  free_notification_list();

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

/* free a notification list that was created */
void free_notification_list() {
  notification* temp_notification = NULL;
  notification* next_notification = NULL;

  temp_notification = notification_list;
  while (temp_notification != NULL) {
    next_notification = temp_notification->next;
    delete temp_notification;
    temp_notification = next_notification;
  }

  /* reset notification list pointer */
  notification_list = NULL;
  return;
}

/* reset all system-wide variables, so when we've receive a SIGHUP we can restart cleanly */
int reset_variables() {
  // XXX: config->reset();

  logging_options =
    log_runtime_error | log_runtime_warning |
    log_verification_error | log_verification_warning |
    log_config_error | log_config_warning | log_process_info |
    log_host_notification | log_service_notification |
    log_event_handler | log_external_command | log_passive_check |
    log_host_up | log_host_down | log_host_unreachable |
    log_service_ok | log_service_warning | log_service_unknown |
    log_service_critical | log_info_message;

  syslog_options =
    log_runtime_error | log_runtime_warning |
    log_verification_error | log_verification_warning |
    log_config_error | log_config_warning | log_process_info |
    log_host_notification | log_service_notification |
    log_event_handler | log_external_command | log_passive_check |
    log_host_up | log_host_down | log_host_unreachable |
    log_service_ok | log_service_warning | log_service_unknown |
    log_service_critical | log_info_message;

  modified_host_process_attributes = MODATTR_NONE;
  modified_service_process_attributes = MODATTR_NONE;

  last_command_check = 0L;
  last_command_status_update = 0L;
  last_log_rotation = 0L;

  currently_running_service_checks = 0;

  next_comment_id = 0L;         /* comment and downtime id get initialized to nonzero elsewhere */
  next_downtime_id = 0L;
  next_event_id = 1;
  next_notification_id = 1;

  /* initialize macros */
  init_macros();

  global_host_event_handler_ptr = NULL;
  global_service_event_handler_ptr = NULL;

  ocsp_command_ptr = NULL;
  ochp_command_ptr = NULL;

  /* reset umask */
  umask(S_IWGRP | S_IWOTH);
  return (OK);
}
