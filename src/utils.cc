/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2011-2012 Merethis
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

#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include "engine.hh"
#include "comments.hh"
#include "globals.hh"
#include "broker.hh"
#include "nebmods.hh"
#include "notifications.hh"
#include "shared.hh"
#include "utils.hh"
#include "commands/raw.hh"
#include "commands/set.hh"
#include "checks/checker.hh"
#include "broker/compatibility.hh"
#include "broker/loader.hh"
#include "logging/engine.hh"
#include "logging/logger.hh"
#include "events/loop.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/******************************************************************/
/******************** SYSTEM COMMAND FUNCTIONS ********************/
/******************************************************************/

/* executes a system command - used for notifications, event handlers, etc. */
int my_system_r(nagios_macros const* mac,
		char* cmd,
		int timeout,
                int* early_timeout,
		double* exectime,
		char** output,
                unsigned int max_output_length) {

  logger(dbg_functions, basic) << "my_system_r()";

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

  logger(dbg_commands, more) << "Running command '" << cmd << "'...";

  timeval start_time = timeval();
  timeval end_time = timeval();

  // time to start command.
  gettimeofday(&start_time, NULL);

  // send event broker.
  broker_system_command(NEBTYPE_SYSTEM_COMMAND_START,
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
  commands::result cmd_result;
  raw_cmd.run(cmd, *mac, timeout, cmd_result);

  end_time = cmd_result.get_end_time();
  *exectime = cmd_result.get_execution_time();
  *early_timeout = cmd_result.get_is_timeout();
  if (output != NULL && max_output_length > 0) {
    if (cmd_result.get_stdout() != "") {
      *output = my_strdup(qPrintable(cmd_result.get_stdout().left(max_output_length - 1)));
    }
    else if (cmd_result.get_stderr() != "") {
      *output = my_strdup(qPrintable(cmd_result.get_stderr().left(max_output_length - 1)));
    }
  }

  int result = cmd_result.get_exit_code();

  logger(dbg_commands, more)
    << fixed << setprecision(3)
    << "Execution time=" << *exectime
    << " sec, early timeout=" << *early_timeout
    << ", result=" << result << ", output="
    << (output == NULL ? "(null)" : *output);

  // send event broker.
  broker_system_command(NEBTYPE_SYSTEM_COMMAND_END,
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
int my_system(char* cmd,
	      int timeout,
	      int* early_timeout,
              double* exectime,
	      char** output,
	      int max_output_length) {
  return (my_system_r(get_global_macros(),
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
int get_raw_command_line_r(nagios_macros* mac,
			   command* cmd_ptr,
                           char const* cmd,
			   char** full_command,
                           int macro_options) {
  char temp_arg[MAX_COMMAND_BUFFER] = "";
  char* arg_buffer = NULL;
  unsigned int x = 0;
  unsigned int y = 0;
  int arg_index = 0;
  int escaped = FALSE;

  logger(dbg_functions, basic) << "get_raw_command_line_r()";

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
    *full_command = my_strdup(cmd_ptr->command_line == NULL ? "" : cmd_ptr->command_line);
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
        if (cmd[arg_index] == '\\' && escaped == FALSE) {
          escaped = TRUE;
          continue;
        }

        /* end of argument */
        if ((cmd[arg_index] == '!' && escaped == FALSE) || cmd[arg_index] == '\x0')
          break;

        /* normal of escaped char */
        temp_arg[y] = cmd[arg_index];
        y++;

        /* clear escaped flag */
        escaped = FALSE;
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
int get_raw_command_line(command* cmd_ptr,
			 char* cmd,
                         char** full_command,
			 int macro_options) {
  nagios_macros *mac;

  mac = get_global_macros();
  return (get_raw_command_line_r(mac, cmd_ptr, cmd, full_command, macro_options));
}

/******************************************************************/
/******************** ENVIRONMENT FUNCTIONS ***********************/
/******************************************************************/

/* sets or unsets an environment variable */
int set_environment_var(char const* name, char const* value, int set) {
  char* env_string = NULL;

  /* we won't mess with null variable names */
  if (name == NULL)
    return (ERROR);

  /* set the environment variable */
  if (set == TRUE) {

    setenv(name, (value == NULL) ? "" : value, 1);

    /* needed for Solaris and systems that don't have setenv() */
    /* this will leak memory, but in a "controlled" way, since lost memory should be freed when the child process exits */
    std::ostringstream oss;
    oss << name << '=' << (value ? value : "");
    env_string = my_strdup(oss.str().c_str());
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

/* Checks if the given time is in daylight time saving period. */
int is_dst_time(time_t* time) {
  struct tm my_tm;
  localtime_r(time, &my_tm);
  return (my_tm.tm_isdst);
}

/* Returns the shift in seconds if the given times are across the daylight time saving period change. */
int get_dst_shift(time_t* start, time_t* end) {
  int shift(0);
  int dst_start(is_dst_time(start));
  int dst_end(is_dst_time(end));
  if (dst_start < dst_end) {
    shift = 3600;
  }
  else if (dst_start > dst_end) {
    shift = -3600;
  }
  return (shift);
}

/*#define TEST_TIMEPERIODS_A 1*/

/* see if the specified time falls into a valid time range in the given time period */
int check_time_against_period(time_t test_time, timeperiod* tperiod) {
  timeperiodexclusion* temp_timeperiodexclusion = NULL;
  timeperiodexclusion* first_timeperiodexclusion = NULL;
  daterange* temp_daterange = NULL;
  timerange* temp_timerange = NULL;
  time_t midnight = (time_t)0L;
  time_t start_time = (time_t)0L;
  time_t end_time = (time_t)0L;
  int found_match = FALSE;
  struct tm* t, tm_s;
  int daterange_type = 0;
  unsigned long days = 0L;
  time_t day_range_start = (time_t)0L;
  time_t day_range_end = (time_t)0L;
  int test_time_year = 0;
  int test_time_mon = 0;
  // int test_time_mday = 0;
  int test_time_wday = 0;
  int year = 0;
  int shift;

  logger(dbg_functions, basic) << "check_time_against_period()";

  /* if no period was specified, assume the time is good */
  if (tperiod == NULL)
    return (OK);

  /* test exclusions first - if exclusions match current time, bail out with an error */
  /* clear exclusions list before recursing (and restore afterwards) to prevent endless loops... */
  first_timeperiodexclusion = tperiod->exclusions;
  tperiod->exclusions = NULL;
  for (temp_timeperiodexclusion = first_timeperiodexclusion;
       temp_timeperiodexclusion != NULL;
       temp_timeperiodexclusion = temp_timeperiodexclusion->next) {
    if (check_time_against_period(test_time, temp_timeperiodexclusion->timeperiod_ptr) == OK) {
      tperiod->exclusions = first_timeperiodexclusion;
      return (ERROR);
    }
  }
  tperiod->exclusions = first_timeperiodexclusion;

  /* save values for later */
  t = localtime_r((time_t*)&test_time, &tm_s);
  test_time_year = t->tm_year;
  test_time_mon = t->tm_mon;
  // test_time_mday = t->tm_mday;
  test_time_wday = t->tm_wday;

  /* calculate the start of the day (midnight, 00:00 hours) when the specified test time occurs */
  t->tm_sec = 0;
  t->tm_min = 0;
  t->tm_hour = 0;
  midnight = mktime(t);

  /**** check exceptions first ****/
  for (daterange_type = 0;
       daterange_type < DATERANGE_TYPES;
       daterange_type++) {

    for (temp_daterange = tperiod->exceptions[daterange_type];
         temp_daterange != NULL;
         temp_daterange = temp_daterange->next) {

#ifdef TEST_TIMEPERIODS_A
      printf("TYPE: %d\n", daterange_type);
      printf("TEST:     %lu = %s", (unsigned long)test_time, ctime(&test_time));
      printf("MIDNIGHT: %lu = %s", (unsigned long)midnight, ctime(&midnight));
#endif

      /* get the start time */
      switch (daterange_type) {
      case DATERANGE_CALENDAR_DATE:
        t->tm_sec = 0;
        t->tm_min = 0;
        t->tm_hour = 0;
        t->tm_wday = 0;
        t->tm_mday = temp_daterange->smday;
        t->tm_mon = temp_daterange->smon;
        t->tm_year = (temp_daterange->syear - 1900);
        t->tm_isdst = -1;
        start_time = mktime(t);
        break;

      case DATERANGE_MONTH_DATE:
        start_time = calculate_time_from_day_of_month(test_time_year,
						      temp_daterange->smon,
						      temp_daterange->smday);
        break;

      case DATERANGE_MONTH_DAY:
        start_time = calculate_time_from_day_of_month(test_time_year,
						      test_time_mon,
						      temp_daterange->smday);
        break;

      case DATERANGE_MONTH_WEEK_DAY:
        start_time = calculate_time_from_weekday_of_month(test_time_year,
							  temp_daterange->smon,
							  temp_daterange->swday,
							  temp_daterange->swday_offset);
        break;

      case DATERANGE_WEEK_DAY:
        start_time = calculate_time_from_weekday_of_month(test_time_year,
							  test_time_mon,
							  temp_daterange->swday,
							  temp_daterange->swday_offset);
        break;

      default:
        continue;
      }

      /* get the end time */
      switch (daterange_type) {
      case DATERANGE_CALENDAR_DATE:
        t->tm_sec = 0;
        t->tm_min = 0;
        t->tm_hour = 0;
        t->tm_wday = 0;
        t->tm_mday = temp_daterange->emday;
        t->tm_mon = temp_daterange->emon;
        t->tm_year = (temp_daterange->eyear - 1900);
        t->tm_isdst = -1;
        end_time = mktime(t);
        break;

      case DATERANGE_MONTH_DATE:
        year = test_time_year;
        end_time = calculate_time_from_day_of_month(year,
						    temp_daterange->emon,
						    temp_daterange->emday);
        /* advance a year if necessary: august 2 - february 5 */
        if (end_time < start_time) {
          year++;
          end_time = calculate_time_from_day_of_month(year,
						      temp_daterange->emon,
						      temp_daterange->emday);
        }
        break;

      case DATERANGE_MONTH_DAY:
        end_time = calculate_time_from_day_of_month(test_time_year,
						    test_time_mon,
						    temp_daterange->emday);
        break;

      case DATERANGE_MONTH_WEEK_DAY:
        year = test_time_year;
        end_time = calculate_time_from_weekday_of_month(year,
							temp_daterange->emon,
							temp_daterange->ewday,
							temp_daterange->ewday_offset);
        /* advance a year if necessary: thursday 2 august - monday 3 february */
        if (end_time < start_time) {
          year++;
          end_time = calculate_time_from_weekday_of_month(year,
							  temp_daterange->emon,
							  temp_daterange->ewday,
							  temp_daterange->ewday_offset);
        }
        break;

      case DATERANGE_WEEK_DAY:
        end_time = calculate_time_from_weekday_of_month(test_time_year,
							test_time_mon,
							temp_daterange->ewday,
							temp_daterange->ewday_offset);
        break;

      default:
        continue;
      }

#ifdef TEST_TIMEPERIODS_A
      printf("START:    %lu = %s", (unsigned long)start_time, ctime(&start_time));
      printf("END:      %lu = %s", (unsigned long)end_time, ctime(&end_time));
#endif

      /* start date was bad, so skip this date range */
      if ((unsigned long)start_time == 0L)
        continue;

      /* end date was bad - see if we can handle the error */
      if ((unsigned long)end_time == 0L) {
        switch (daterange_type) {
        case DATERANGE_CALENDAR_DATE:
          continue;
          break;

        case DATERANGE_MONTH_DATE:
          /* end date can't be helped, so skip it */
          if (temp_daterange->emday < 0)
            continue;

          /* else end date slipped past end of month, so use last day of month as end date */
          /* use same year calculated above */
          end_time = calculate_time_from_day_of_month(year, temp_daterange->emon, -1);
          break;

        case DATERANGE_MONTH_DAY:
          /* end date can't be helped, so skip it */
          if (temp_daterange->emday < 0)
            continue;

          /* else end date slipped past end of month, so use last day of month as end date */
          end_time = calculate_time_from_day_of_month(test_time_year, test_time_mon, -1);
          break;

        case DATERANGE_MONTH_WEEK_DAY:
          /* end date can't be helped, so skip it */
          if (temp_daterange->ewday_offset < 0)
            continue;

          /* else end date slipped past end of month, so use last day of month as end date */
          /* use same year calculated above */
          end_time = calculate_time_from_day_of_month(year, test_time_mon, -1);
          break;

        case DATERANGE_WEEK_DAY:
          /* end date can't be helped, so skip it */
          if (temp_daterange->ewday_offset < 0)
            continue;

          /* else end date slipped past end of month, so use last day of month as end date */
          end_time = calculate_time_from_day_of_month(test_time_year, test_time_mon, -1);
          break;

        default:
          continue;
        }
      }

      /* calculate skip date start (and end) */
      if (temp_daterange->skip_interval > 1) {

        /* skip start date must be before test time */
        if (start_time > test_time)
          continue;

        /* Check if interval is accress dlst change and gets the compensation. */
        shift = get_dst_shift(&start_time, &midnight);

        /* how many days have passed between skip start date and test time? */
        days = (shift + (unsigned long)midnight - (unsigned long)start_time) / (3600 * 24);

        /* if test date doesn't fall on a skip interval day, bail out early */
        if ((days % temp_daterange->skip_interval) != 0)
          continue;

        /* use midnight of test date as start time */
        else
          start_time = midnight;

        /* if skipping range has no end, use test date as end */
        if ((daterange_type == DATERANGE_CALENDAR_DATE)
            && (is_daterange_single_day(temp_daterange) == TRUE))
          end_time = midnight;
      }

#ifdef TEST_TIMEPERIODS_A
      printf("NEW START:    %lu = %s", (unsigned long)start_time, ctime(&start_time));
      printf("NEW END:      %lu = %s", (unsigned long)end_time, ctime(&end_time));
      printf("%d DAYS PASSED\n", days);
      printf("DLST SHIFT: %d", shift);
#endif

      /* time falls into the range of days */
      if (midnight >= start_time && midnight <= end_time)
        found_match = TRUE;

      /* found a day match, so see if time ranges are good */
      if (found_match == TRUE) {

        for (temp_timerange = temp_daterange->times;
             temp_timerange != NULL;
             temp_timerange = temp_timerange->next) {

          /* ranges with start/end of zero mean exlude this day */
          if (temp_timerange->range_start == 0
              && temp_timerange->range_end == 0) {
#ifdef TEST_TIMEPERIODS_A
            printf("0 MINUTE RANGE EXCLUSION\n");
#endif
            continue;
          }

          day_range_start = (time_t)(midnight + temp_timerange->range_start);
          day_range_end = (time_t)(midnight + temp_timerange->range_end);

#ifdef TEST_TIMEPERIODS_A
          printf("  RANGE START: %lu (%lu) = %s",
		 temp_timerange->range_start,
                 (unsigned long)day_range_start,
                 ctime(&day_range_start));
          printf("  RANGE END:   %lu (%lu) = %s",
                 temp_timerange->range_end,
                 (unsigned long)day_range_end, ctime(&day_range_end));
#endif

          /* if the user-specified time falls in this range, return with a positive result */
          if (test_time >= day_range_start
              && test_time <= day_range_end)
            return (OK);
        }

        /* no match, so bail with error */
        return (ERROR);
      }
    }
  }

  /**** check normal, weekly rotating schedule last ****/

  /* check weekday time ranges */
  for (temp_timerange = tperiod->days[test_time_wday];
       temp_timerange != NULL;
       temp_timerange = temp_timerange->next) {

    day_range_start = (time_t)(midnight + temp_timerange->range_start);
    day_range_end = (time_t)(midnight + temp_timerange->range_end);

    /* if the user-specified time falls in this range, return with a positive result */
    if (test_time >= day_range_start && test_time <= day_range_end)
      return (OK);
  }

  return (ERROR);
}

/*#define TEST_TIMEPERIODS_B 1*/

/* Separate this out from public get_next_valid_time for testing, so we can mock current_time */
static void _get_next_valid_time(time_t pref_time,
				 time_t current_time,
                                 time_t* valid_time,
                                 timeperiod* tperiod) {
  time_t preferred_time = (time_t)0L;
  timerange* temp_timerange;
  daterange* temp_daterange;
  time_t midnight = (time_t)0L;
  struct tm* t, tm_s;
  time_t day_start = (time_t)0L;
  time_t day_range_start = (time_t)0L;
  time_t day_range_end = (time_t)0L;
  time_t start_time = (time_t)0L;
  time_t end_time = (time_t)0L;
  int have_earliest_time = FALSE;
  time_t earliest_time = (time_t)0L;
  time_t earliest_day = (time_t)0L;
  time_t potential_time = (time_t)0L;
  int weekday = 0;
  int has_looped = FALSE;
  int days_into_the_future = 0;
  int daterange_type = 0;
  unsigned long days = 0L;
  unsigned long advance_interval = 0L;
  int year = 0;                 /* new */
  int month = 0;                /* new */
  int pref_time_year = 0;
  int pref_time_mon = 0;
#ifdef TEST_TIMEPERIODS_B
  int pref_time_mday = 0;
#endif
  int pref_time_wday = 0;
  int current_time_year = 0;
  int current_time_mon = 0;
  int current_time_mday = 0;
#ifdef TEST_TIMEPERIODS_B
  int current_time_wday = 0;
#endif
  int shift;

  /* preferred time must be now or in the future */
  preferred_time = (pref_time < current_time) ? current_time : pref_time;

  /* if no timeperiod, go with preferred time */
  if (tperiod == NULL) {
    *valid_time = preferred_time;
    return;
  }

  /* if the preferred time is valid in timeperiod, go with it */
  /* this is necessary because the code below won't catch exceptions where preferred day is last (or only) date in timeperiod (date range) and last valid time has already passed */
  /* performing this check and bailing out early allows us to skip having to check the next instance of a date range exception or weekday to determine the next valid time */
  if (check_time_against_period(preferred_time, tperiod) == OK) {
#ifdef TEST_TIMEPERIODS_B
    printf("PREF TIME IS VALID\n");
#endif
    *valid_time = preferred_time;
    return;
  }

  /* calculate the start of the day (midnight, 00:00 hours) of preferred time */
  t = localtime_r(&preferred_time, &tm_s);
  t->tm_sec = 0;
  t->tm_min = 0;
  t->tm_hour = 0;
  midnight = mktime(t);

  /* save pref time values for later */
  pref_time_year = t->tm_year;
  pref_time_mon = t->tm_mon;
#ifdef TEST_TIMEPERIODS_B
  pref_time_mday = t->tm_mday;
#endif
  pref_time_wday = t->tm_wday;

  /* save current time values for later */
  t = localtime_r(&current_time, &tm_s);
  current_time_year = t->tm_year;
  current_time_mon = t->tm_mon;
  current_time_mday = t->tm_mday;
#ifdef TEST_TIMEPERIODS_B
  current_time_wday = t->tm_wday;
#endif

#ifdef TEST_TIMEPERIODS_B
  printf("PREF TIME:    %lu = %s", (unsigned long)preferred_time, ctime(&preferred_time));
  printf("CURRENT TIME: %lu = %s", (unsigned long)current_time, ctime(&current_time));
  printf("PREF YEAR:    %d, MON: %d, MDAY: %d, WDAY: %d\n", pref_time_year, pref_time_mon, pref_time_mday, pref_time_wday);
  printf("CURRENT YEAR: %d, MON: %d, MDAY: %d, WDAY: %d\n", current_time_year, current_time_mon, current_time_mday, current_time_wday);
#endif

  /**** check exceptions (in this timeperiod definition) first ****/
  for (daterange_type = 0; daterange_type < DATERANGE_TYPES; daterange_type++) {

#ifdef TEST_TIMEPERIODS_B
    printf("TYPE: %d\n", daterange_type);
#endif

    for (temp_daterange = tperiod->exceptions[daterange_type];
         temp_daterange != NULL;
         temp_daterange = temp_daterange->next) {

      /* get the start time */
      switch (daterange_type) {
      case DATERANGE_CALENDAR_DATE:    /* 2009-08-11 */
        t->tm_sec = 0;
        t->tm_min = 0;
        t->tm_hour = 0;
        t->tm_mday = temp_daterange->smday;
        t->tm_mon = temp_daterange->smon;
        t->tm_year = (temp_daterange->syear - 1900);
        t->tm_isdst = -1;
        start_time = mktime(t);
        break;

      case DATERANGE_MONTH_DATE:       /* january 1 */
        /* what year should we use? */
        year = (pref_time_year < current_time_year) ? current_time_year : pref_time_year;
        /* advance an additional year if we already passed the end month date */
        if ((temp_daterange->emon < current_time_mon)
            || ((temp_daterange->emon == current_time_mon)
                && temp_daterange->emday < current_time_mday))
          year++;
        start_time = calculate_time_from_day_of_month(year,
						      temp_daterange->smon,
						      temp_daterange->smday);
        break;

      case DATERANGE_MONTH_DAY:        /* day 3 */
        /* what year should we use? */
        year = (pref_time_year < current_time_year) ? current_time_year : pref_time_year;
        /* use current month */
        month = current_time_mon;
        /* advance an additional month (and possibly the year) if we already passed the end day of month */
        if (temp_daterange->emday < current_time_mday) {
          /*if(month==1){ */
          if (month == 11) {
            month = 0;
            year++;
          }
          else
            month++;
        }
        start_time = calculate_time_from_day_of_month(year,
						      month,
						      temp_daterange->smday);
        break;

      case DATERANGE_MONTH_WEEK_DAY:   /* thursday 2 april */
        /* what year should we use? */
        year = (pref_time_year < current_time_year) ? current_time_year : pref_time_year;
        /* calculate time of specified weekday of specific month */
        start_time = calculate_time_from_weekday_of_month(year,
							  temp_daterange->smon,
							  temp_daterange->swday,
							  temp_daterange->swday_offset);
        /* advance to next year if we've passed this month weekday already this year */
        if (start_time < preferred_time) {
          year++;
          start_time = calculate_time_from_weekday_of_month(year,
							    temp_daterange->smon,
							    temp_daterange->swday,
							    temp_daterange->swday_offset);
        }
        break;

      case DATERANGE_WEEK_DAY: /* wednesday 1 */
        /* what year should we use? */
        year = (pref_time_year < current_time_year) ? current_time_year : pref_time_year;
        /* calculate time of specified weekday of month */
        start_time = calculate_time_from_weekday_of_month(year,
							  pref_time_mon,
							  temp_daterange->swday,
							  temp_daterange->swday_offset);
        /* advance to next month (or year) if we've passed this weekday of this month already */
        if (start_time < preferred_time) {
          month = pref_time_mon;
          if (month == 11) {
            month = 0;
            year++;
          }
          else
            month++;
          start_time = calculate_time_from_weekday_of_month(year,
							    month,
							    temp_daterange->swday,
							    temp_daterange->swday_offset);
        }
        break;

      default:
        continue;
      }

#ifdef TEST_TIMEPERIODS_B
      printf("START TIME: %lu = %s", start_time, ctime(&start_time));
#endif

      /* get the end time */
      switch (daterange_type) {
      case DATERANGE_CALENDAR_DATE:
        t->tm_sec = 0;
        t->tm_min = 0;
        t->tm_hour = 0;
        t->tm_mday = temp_daterange->emday;
        t->tm_mon = temp_daterange->emon;
        t->tm_year = (temp_daterange->eyear - 1900);
        t->tm_isdst = -1;
        end_time = mktime(t);
        break;

      case DATERANGE_MONTH_DATE:
        /* use same year as was calculated for start time above */
        end_time = calculate_time_from_day_of_month(year,
						    temp_daterange->emon,
						    temp_daterange->emday);
        /* advance a year if necessary: august 5 - feburary 2 */
        if (end_time < start_time) {
          year++;
          end_time = calculate_time_from_day_of_month(year,
						      temp_daterange->emon,
						      temp_daterange->emday);
        }
        break;

      case DATERANGE_MONTH_DAY:
        /* use same year and month as was calculated for start time above */
        end_time = calculate_time_from_day_of_month(year,
						    month,
						    temp_daterange->emday);
        break;

      case DATERANGE_MONTH_WEEK_DAY:
        /* use same year as was calculated for start time above */
        end_time = calculate_time_from_weekday_of_month(year,
							temp_daterange->emon,
							temp_daterange->ewday,
							temp_daterange->ewday_offset);
        /* advance a year if necessary: thursday 2 august - monday 3 february */
        if (end_time < start_time) {
          year++;
          end_time = calculate_time_from_weekday_of_month(year,
							  temp_daterange->emon,
							  temp_daterange->ewday,
							  temp_daterange->ewday_offset);
        }
        break;

      case DATERANGE_WEEK_DAY:
        /* use same year and month as was calculated for start time above */
        end_time = calculate_time_from_weekday_of_month(year,
							month,
							temp_daterange->ewday,
							temp_daterange->ewday_offset);
        break;

      default:
        continue;
      }

#ifdef TEST_TIMEPERIODS_B
      printf("STARTTIME: %lu = %s", (unsigned long)start_time, ctime(&start_time));
      printf("ENDTIME1: %lu = %s", (unsigned long)end_time, ctime(&end_time));
#endif

      /* start date was bad, so skip this date range */
      if ((unsigned long)start_time == 0L)
        continue;

      /* end date was bad - see if we can handle the error */
      if ((unsigned long)end_time == 0L) {
        switch (daterange_type) {
        case DATERANGE_CALENDAR_DATE:
          continue;

        case DATERANGE_MONTH_DATE:
          /* end date can't be helped, so skip it */
          if (temp_daterange->emday < 0)
            continue;

          /* else end date slipped past end of month, so use last day of month as end date */
          end_time = calculate_time_from_day_of_month(year, temp_daterange->emon, -1);
          break;

        case DATERANGE_MONTH_DAY:
          /* end date can't be helped, so skip it */
          if (temp_daterange->emday < 0)
            continue;

          /* else end date slipped past end of month, so use last day of month as end date */
          end_time = calculate_time_from_day_of_month(year, month, -1);
          break;

        case DATERANGE_MONTH_WEEK_DAY:
          /* end date can't be helped, so skip it */
          if (temp_daterange->ewday_offset < 0)
            continue;

          /* else end date slipped past end of month, so use last day of month as end date */
          end_time = calculate_time_from_day_of_month(year, pref_time_mon, -1);
          break;

        case DATERANGE_WEEK_DAY:
          /* end date can't be helped, so skip it */
          if (temp_daterange->ewday_offset < 0)
            continue;

          /* else end date slipped past end of month, so use last day of month as end date */
          end_time = calculate_time_from_day_of_month(year, month, -1);
          break;

        default:
          continue;
        }
      }

#ifdef TEST_TIMEPERIODS_B
      printf("ENDTIME2: %lu = %s", (unsigned long)end_time, ctime(&end_time));
#endif

      /* if skipping days... */
      if (temp_daterange->skip_interval > 1) {

        /* advance to the next possible skip date */
        if (start_time < preferred_time) {

          /* Check if the interval is across dlst change and gets the compensation. */
          shift = get_dst_shift(&start_time, &midnight);

          /* how many days have passed between skip start date and preferred time? */
          days = (shift + (unsigned long)midnight - (unsigned long)start_time) / (3600 * 24);

#ifdef TEST_TIMEPERIODS_B
          printf("MIDNIGHT: %lu = %s", (unsigned long)midnight, ctime(&midnight));
          printf("%lu SECONDS PASSED\n", (unsigned long)(midnight - start_time));
          printf("%d DAYS PASSED\n", days);
          printf("REMAINDER: %d\n", (days % temp_daterange->skip_interval));
          printf("SKIP INTERVAL: %d\n", temp_daterange->skip_interval);
          printf("DLST SHIFT: %d", shift);
#endif

          /* advance start date to next skip day */
          if ((days % temp_daterange->skip_interval) == 0)
            start_time += (days * 3600 * 24);
          else
            start_time += ((days - (days % temp_daterange->skip_interval)
			    + temp_daterange->skip_interval) * 3600 * 24);
        }

        /* if skipping has no end, use start date as end */
        if ((daterange_type == DATERANGE_CALENDAR_DATE)
            && is_daterange_single_day(temp_daterange) == TRUE)
          end_time = start_time;
      }

#ifdef TEST_TIMEPERIODS_B
      printf("\nSTART:     %lu = %s", (unsigned long)start_time, ctime(&start_time));
      printf("END:       %lu = %s", (unsigned long)end_time, ctime(&end_time));
      printf("PREFERRED: %lu = %s", (unsigned long)preferred_time, ctime(&preferred_time));
      printf("CURRENT:   %lu = %s", (unsigned long)current_time, ctime(&current_time));
#endif

      /* skip this date range its out of bounds with what we want */
      if (preferred_time > end_time)
        continue;

      /* how many days at a time should we advance? */
      if (temp_daterange->skip_interval > 1)
        advance_interval = temp_daterange->skip_interval;
      else
        advance_interval = 1;

      /* advance through the date range */
      for (day_start = start_time;
	   day_start <= end_time;
           day_start += (advance_interval * 3600 * 24)) {

        /* we already found a time from a higher-precendence date range exception */
        if (day_start >= earliest_day && have_earliest_time == TRUE)
          continue;

        for (temp_timerange = temp_daterange->times;
             temp_timerange != NULL;
             temp_timerange = temp_timerange->next) {

          /* ranges with start/end of zero mean exlude this day */
          if (temp_timerange->range_start == 0 && temp_timerange->range_end == 0)
            continue;

          day_range_start = (time_t)(day_start + temp_timerange->range_start);
          day_range_end = (time_t)(day_start + temp_timerange->range_end);

#ifdef TEST_TIMEPERIODS_B
          printf("  RANGE START: %lu (%lu) = %s",
                 temp_timerange->range_start,
                 (unsigned long)day_range_start,
                 ctime(&day_range_start));
          printf("  RANGE END:   %lu (%lu) = %s",
                 temp_timerange->range_end,
                 (unsigned long)day_range_end, ctime(&day_range_end));
#endif

          /* range is out of bounds */
          if (day_range_end < preferred_time)
            continue;

          /* preferred time occurs before range start, so use range start time as earliest potential time */
          if (day_range_start >= preferred_time)
            potential_time = day_range_start;
          /* preferred time occurs between range start/end, so use preferred time as earliest potential time */
          else if (day_range_end >= preferred_time)
            potential_time = preferred_time;

          /* is this the earliest time found thus far? */
          if (have_earliest_time == FALSE || potential_time < earliest_time) {
            have_earliest_time = TRUE;
            earliest_time = potential_time;
            earliest_day = day_start;
#ifdef TEST_TIMEPERIODS_B
            printf("    EARLIEST TIME: %lu = %s", (unsigned long)earliest_time, ctime(&earliest_time));
#endif
          }
        }
      }
    }
  }

  /**** find next available time from normal, weekly rotating schedule (in this timeperiod definition) ****/

  /* check a one week rotation of time */
  has_looped = FALSE;
  for (weekday = pref_time_wday, days_into_the_future = 0; ; weekday++, days_into_the_future++) {
    /* break out of the loop if we have checked an entire week already */
    if (has_looped == TRUE && weekday >= pref_time_wday)
      break;

    if (weekday >= 7) {
      weekday -= 7;
      has_looped = TRUE;
    }

    /* calculate start of this future weekday */
    day_start = (time_t)(midnight + (days_into_the_future * 3600 * 24));

    /* we already found a time from a higher-precendence date range exception */
    if (day_start == earliest_day)
      continue;

    /* check all time ranges for this day of the week */
    for (temp_timerange = tperiod->days[weekday];
         temp_timerange != NULL;
         temp_timerange = temp_timerange->next) {

      /* calculate the time for the start of this time range */
      day_range_start = (time_t)(day_start + temp_timerange->range_start);

      if ((have_earliest_time == FALSE
           || day_range_start < earliest_time)
          && day_range_start >= preferred_time) {
        have_earliest_time = TRUE;
        earliest_time = day_range_start;
        earliest_day = day_start;
      }
    }
  }

  /* if we couldn't find a time period there must be none defined */
  if (have_earliest_time == FALSE || earliest_time == (time_t)0)
    *valid_time = (time_t)preferred_time;
  /* else use the calculated time */
  else
    *valid_time = earliest_time;
}

/* given a preferred time, get the next valid time within a time period */
void get_next_valid_time(time_t pref_time, time_t* valid_time, timeperiod* tperiod) {
  time_t current_time = (time_t)0L;

  logger(dbg_functions, basic) << "get_next_valid_time()";

  /* get time right now, preferred time must be now or in the future */
  time(&current_time);

  _get_next_valid_time(pref_time, current_time, valid_time, tperiod);
}

/* tests if a date range covers just a single day */
int is_daterange_single_day(daterange* dr) {
  if (dr == NULL)
    return (FALSE);

  if (dr->syear != dr->eyear)
    return (FALSE);
  if (dr->smon != dr->emon)
    return (FALSE);
  if (dr->smday != dr->emday)
    return (FALSE);
  if (dr->swday != dr->ewday)
    return (FALSE);
  if (dr->swday_offset != dr->ewday_offset)
    return (FALSE);

  return (TRUE);
}

/* returns a time (midnight) of particular (3rd, last) day in a given month */
time_t calculate_time_from_day_of_month(int year, int month, int monthday) {
  time_t midnight;
  int day = 0;
  struct tm t;

#ifdef TEST_TIMEPERIODS
  printf("YEAR: %d, MON: %d, MDAY: %d\n", year, month, monthday);
#endif

  /* positive day (3rd day) */
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
    printf("MIDNIGHT CALC: %s", ctime(&midnight));
#endif

    /* if we rolled over to the next month, time is invalid */
    /* assume the user's intention is to keep it in the current month */
    if (t.tm_mon != month)
      midnight = (time_t)0L;
  }

  /* negative offset (last day, 3rd to last day) */
  else {
    /* find last day in the month */
    day = 32;
    do {
      /* back up a day */
      day--;

      /* make the new time */
      t.tm_mon = month;
      t.tm_year = year;
      t.tm_mday = day;
      t.tm_isdst = -1;
      midnight = mktime(&t);

    } while (t.tm_mon != month);

    /* now that we know the last day, back up more */
    /* make the new time */
    t.tm_mon = month;
    t.tm_year = year;
    /* -1 means last day of month, so add one to to make this correct - Mike Bird */
    t.tm_mday += (monthday < -30) ? -30 : monthday + 1;
    t.tm_isdst = -1;
    midnight = mktime(&t);

    /* if we rolled over to the previous month, time is invalid */
    /* assume the user's intention is to keep it in the current month */
    if (t.tm_mon != month)
      midnight = (time_t)0L;
  }

  return (midnight);
}

/* returns a time (midnight) of particular (3rd, last) weekday in a given month */
time_t calculate_time_from_weekday_of_month(int year,
					    int month,
                                            int weekday,
                                            int weekday_offset) {
  time_t midnight;
  int days = 0;
  int weeks = 0;
  struct tm t;

  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  t.tm_year = year;
  t.tm_mon = month;
  t.tm_mday = 1;
  t.tm_isdst = -1;

  midnight = mktime(&t);

  /* how many days must we advance to reach the first instance of the weekday this month? */
  days = weekday - (t.tm_wday);
  if (days < 0)
    days += 7;

  /* positive offset (3rd thursday) */
  if (weekday_offset > 0) {

    /* how many weeks must we advance (no more than 5 possible) */
    weeks = (weekday_offset > 5) ? 5 : weekday_offset;
    days += ((weeks - 1) * 7);

    /* make the new time */
    t.tm_mon = month;
    t.tm_year = year;
    t.tm_mday = days + 1;
    t.tm_isdst = -1;
    midnight = mktime(&t);

    /* if we rolled over to the next month, time is invalid */
    /* assume the user's intention is to keep it in the current month */
    if (t.tm_mon != month)
      midnight = (time_t)0L;
  }

  /* negative offset (last thursday, 3rd to last tuesday) */
  else {
    /* find last instance of weekday in the month */
    days += (5 * 7);
    do {
      /* back up a week */
      days -= 7;

      /* make the new time */
      t.tm_mon = month;
      t.tm_year = year;
      t.tm_mday = days + 1;
      t.tm_isdst = -1;
      midnight = mktime(&t);

    } while (t.tm_mon != month);

    /* now that we know the last instance of the weekday, back up more */
    weeks = (weekday_offset < -5) ? -5 : weekday_offset;
    days = ((weeks + 1) * 7);

    /* make the new time */
    t.tm_mon = month;
    t.tm_year = year;
    t.tm_mday += days;
    t.tm_isdst = -1;
    midnight = mktime(&t);

    /* if we rolled over to the previous month, time is invalid */
    /* assume the user's intention is to keep it in the current month */
    if (t.tm_mon != month)
      midnight = (time_t)0L;
  }

  return (midnight);
}

/* get the next time to schedule a log rotation */
time_t get_next_log_rotation_time(void) {
  time_t current_time;
  struct tm* t, tm_s;
  int is_dst_now = FALSE;
  time_t run_time;

  time(&current_time);
  t = localtime_r(&current_time, &tm_s);
  t->tm_min = 0;
  t->tm_sec = 0;
  is_dst_now = (t->tm_isdst > 0) ? TRUE : FALSE;

  switch (config.get_log_rotation_method()) {
  case LOG_ROTATION_HOURLY:
    t->tm_hour++;
    run_time = mktime(t);
    break;

  case LOG_ROTATION_DAILY:
    t->tm_mday++;
    t->tm_hour = 0;
    run_time = mktime(t);
    break;

  case LOG_ROTATION_WEEKLY:
    t->tm_mday += (7 - t->tm_wday);
    t->tm_hour = 0;
    run_time = mktime(t);
    break;

  case LOG_ROTATION_MONTHLY:
  default:
    t->tm_mon++;
    t->tm_mday = 1;
    t->tm_hour = 0;
    run_time = mktime(t);
    break;
  }

  if (is_dst_now == TRUE && t->tm_isdst == 0)
    run_time += 3600;
  else if (is_dst_now == FALSE && t->tm_isdst > 0)
    run_time -= 3600;

  return (run_time);
}

/******************************************************************/
/******************** SIGNAL HANDLER FUNCTIONS ********************/
/******************************************************************/

/* trap signals so we can exit gracefully */
void setup_sighandler(void) {
  /* reset the shutdown flag */
  sigshutdown = FALSE;

  /* remove buffering from stderr, stdin, and stdout */
  setbuf(stdin, (char*)NULL);
  setbuf(stdout, (char*)NULL);
  setbuf(stderr, (char*)NULL);

  /* initialize signal handling */
  signal(SIGPIPE, SIG_IGN);
  signal(SIGQUIT, sighandler);
  signal(SIGTERM, sighandler);
  signal(SIGHUP, sighandler);
}

/* reset signal handling... */
void reset_sighandler(void) {
  /* set signal handling to default actions */
  signal(SIGQUIT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  signal(SIGHUP, SIG_DFL);
  signal(SIGPIPE, SIG_DFL);
}

/* handle signals */
void sighandler(int sig) {
  int x = 0;

  caught_signal = TRUE;

  if (sig < 0)
    sig = -sig;

  for (x = 0; sigs[x] != (char*)NULL; x++);
  sig %= x;

  sig_id = sig;

  /* we received a SIGHUP, so restart... */
  if (sig == SIGHUP)
    sigrestart = TRUE;
  /* else begin shutting down... */
  else if (sig < 16)
    sigshutdown = TRUE;
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
int parse_check_output(char* buf,
		       char** short_output,
                       char** long_output,
		       char** perf_data,
                       int escape_newlines_please,
                       int newlines_are_escaped) {
  int current_line = 0;
  int found_newline = FALSE;
  int eof = FALSE;
  int used_buf = 0;
  int dbuf_chunk = 1024;
  dbuf db1;
  dbuf db2;
  char* ptr = NULL;
  int in_perf_data = FALSE;
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
  if (buf == NULL || !strcmp(buf, ""))
    return (OK);

  used_buf = strlen(buf) + 1;

  /* initialize dynamic buffers (1KB chunk size) */
  dbuf_init(&db1, dbuf_chunk);
  dbuf_init(&db2, dbuf_chunk);

  /* unescape newlines and escaped backslashes first */
  if (newlines_are_escaped == TRUE) {
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
  for (x = 0; eof == FALSE; x++) {

    /* we found the end of a line */
    if (buf[x] == '\n')
      found_newline = TRUE;
    else if (buf[x] == '\\' && buf[x + 1] == 'n'
             && newlines_are_escaped == TRUE) {
      found_newline = TRUE;
      buf[x] = '\x0';
      x++;
    }
    else if (buf[x] == '\x0') {
      found_newline = TRUE;
      eof = TRUE;
    }
    else
      found_newline = FALSE;

    if (found_newline == TRUE) {

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
        if (in_perf_data == TRUE) {
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
            in_perf_data = TRUE;
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
      memmove((void*)&buf[0], (void*)&buf[x + 1], (size_t)((int)used_buf - x - 1));
      used_buf -= (x + 1);
      buf[used_buf] = '\x0';
      x = -1;
    }
  }

  /* save long output */
  if (long_output && (db1.buf && strcmp(db1.buf, ""))) {
    if (escape_newlines_please == FALSE)
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
char* get_next_string_from_buf(char* buf, int* start_index, int bufsize) {
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
    return (FALSE);

  std::string tmp(name);
  std::string const& illegal_object_chars = config.get_illegal_object_chars().toStdString();

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
  catch(...) {
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
  if ((dest_fd = open(dest, O_WRONLY | O_TRUNC | O_CREAT | O_APPEND, 0644)) < 0) {
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
  unsigned long buflen = 0L;
  unsigned long new_size = 0L;
  unsigned long memory_needed = 0L;

  if (db == NULL || buf == NULL)
    return (ERROR);

  /* how much memory should we allocate (if any)? */
  buflen = strlen(buf);
  new_size = db->used_size + buflen + 1;

  /* we need more memory */
  if (db->allocated_size < new_size) {

    memory_needed = static_cast<unsigned long>((ceil(new_size / db->chunk_size) + 1) * db->chunk_size);

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

/******************************************************************/
/********************** CHECK STATS FUNCTIONS *********************/
/******************************************************************/

/* initialize check statistics data structures */
int init_check_stats(void) {
  int x = 0;
  int y = 0;

  for (x = 0; x < MAX_CHECK_STATS_TYPES; x++) {
    check_statistics[x].current_bucket = 0;
    for (y = 0; y < CHECK_STATS_BUCKETS; y++)
      check_statistics[x].bucket[y] = 0;
    check_statistics[x].overflow_bucket = 0;
    for (y = 0; y < 3; y++)
      check_statistics[x].minute_stats[y] = 0;
    check_statistics[x].last_update = (time_t)0L;
  }

  return (OK);
}

/* records stats for a given type of check */
int update_check_stats(int check_type, time_t check_time) {
  time_t current_time;
  unsigned long minutes = 0L;
  int new_current_bucket = 0;
  int this_bucket = 0;
  int x = 0;

  if (check_type < 0 || check_type >= MAX_CHECK_STATS_TYPES)
    return (ERROR);

  time(&current_time);

  if ((unsigned long)check_time == 0L) {
#ifdef DEBUG_CHECK_STATS
    printf("TYPE[%d] CHECK TIME==0!\n", check_type);
#endif
    check_time = current_time;
  }

  /* do some sanity checks on the age of the stats data before we start... */
  /* get the new current bucket number */
  minutes = ((unsigned long)check_time - (unsigned long)program_start) / 60;
  new_current_bucket = minutes % CHECK_STATS_BUCKETS;

  /* its been more than 15 minutes since stats were updated, so clear the stats */
  if ((((unsigned long)current_time - (unsigned long)check_statistics[check_type].last_update) / 60) > CHECK_STATS_BUCKETS) {
    for (x = 0; x < CHECK_STATS_BUCKETS; x++)
      check_statistics[check_type].bucket[x] = 0;
    check_statistics[check_type].overflow_bucket = 0;
#ifdef DEBUG_CHECK_STATS
    printf("CLEARING ALL: TYPE[%d], CURRENT=%lu, LASTUPDATE=%lu\n",
	   check_type,
	   (unsigned long)current_time,
	   (unsigned long)check_statistics[check_type].last_update);
#endif
  }
  /* different current bucket number than last time */
  else if (new_current_bucket != check_statistics[check_type].current_bucket) {
    /* clear stats in buckets between last current bucket and new current bucket - stats haven't been updated in a while */
    for (x = check_statistics[check_type].current_bucket; x < (CHECK_STATS_BUCKETS * 2); x++) {
      this_bucket = (x + CHECK_STATS_BUCKETS + 1) % CHECK_STATS_BUCKETS;
      if (this_bucket == new_current_bucket)
        break;

#ifdef DEBUG_CHECK_STATS
      printf("CLEARING BUCKET %d, (NEW=%d, OLD=%d)\n",
	     this_bucket,
             new_current_bucket,
             check_statistics[check_type].current_bucket);
#endif

      /* clear old bucket value */
      check_statistics[check_type].bucket[this_bucket] = 0;
    }

    /* update the current bucket number, push old value to overflow bucket */
    check_statistics[check_type].overflow_bucket = check_statistics[check_type].bucket[new_current_bucket];
    check_statistics[check_type].current_bucket = new_current_bucket;
    check_statistics[check_type].bucket[new_current_bucket] = 0;
  }
#ifdef DEBUG_CHECK_STATS
  else
    printf("NO CLEARING NEEDED\n");
#endif

  /* increment the value of the current bucket */
  check_statistics[check_type].bucket[new_current_bucket]++;

#ifdef DEBUG_CHECK_STATS
  printf("TYPE[%d].BUCKET[%d]=%d\n",
	 check_type,
	 new_current_bucket,
	 check_statistics[check_type].bucket[new_current_bucket]);
  printf("   ");
  for (x = 0; x < CHECK_STATS_BUCKETS; x++)
    printf("[%d] ", check_statistics[check_type].bucket[x]);
  printf(" (%d)\n", check_statistics[check_type].overflow_bucket);
#endif

  /* record last update time */
  check_statistics[check_type].last_update = current_time;

  return (OK);
}

/* generate 1/5/15 minute stats for a given type of check */
int generate_check_stats(void) {
  time_t current_time;
  int x = 0;
  int new_current_bucket = 0;
  int this_bucket = 0;
  int last_bucket = 0;
  int this_bucket_value = 0;
  int last_bucket_value = 0;
  int bucket_value = 0;
  int seconds = 0;
  int minutes = 0;
  int check_type = 0;
  float this_bucket_weight = 0.0;
  float last_bucket_weight = 0.0;
#ifdef DEBUG_CHECK_STATS2
  int left_value = 0;
  int right_value = 0;
#endif

  time(&current_time);

  /* do some sanity checks on the age of the stats data before we start... */
  /* get the new current bucket number */
  minutes = ((unsigned long)current_time - (unsigned long)program_start) / 60;
  new_current_bucket = minutes % CHECK_STATS_BUCKETS;
  for (check_type = 0; check_type < MAX_CHECK_STATS_TYPES; check_type++) {

    /* its been more than 15 minutes since stats were updated, so clear the stats */
    if ((((unsigned long)current_time - (unsigned long)check_statistics[check_type].last_update) / 60) > CHECK_STATS_BUCKETS) {
      for (x = 0; x < CHECK_STATS_BUCKETS; x++)
        check_statistics[check_type].bucket[x] = 0;
      check_statistics[check_type].overflow_bucket = 0;
#ifdef DEBUG_CHECK_STATS
      printf("GEN CLEARING ALL: TYPE[%d], CURRENT=%lu, LASTUPDATE=%lu\n",
	     check_type,
	     (unsigned long)current_time,
	     (unsigned long)check_statistics[check_type].last_update);
#endif
    }
    /* different current bucket number than last time */
    else if (new_current_bucket != check_statistics[check_type].current_bucket) {
      /* clear stats in buckets between last current bucket and new current bucket - stats haven't been updated in a while */
      for (x = check_statistics[check_type].current_bucket; x < CHECK_STATS_BUCKETS * 2; x++) {
        this_bucket = (x + CHECK_STATS_BUCKETS + 1) % CHECK_STATS_BUCKETS;
        if (this_bucket == new_current_bucket)
          break;

#ifdef DEBUG_CHECK_STATS
        printf("GEN CLEARING BUCKET %d, (NEW=%d, OLD=%d), CURRENT=%lu, LASTUPDATE=%lu\n",
	       this_bucket, new_current_bucket,
	       check_statistics[check_type].current_bucket,
	       (unsigned long)current_time,
	       (unsigned long)check_statistics[check_type].last_update);
#endif
        /* clear old bucket value */
        check_statistics[check_type].bucket[this_bucket] = 0;
      }

      /* update the current bucket number, push old value to overflow bucket */
      check_statistics[check_type].overflow_bucket = check_statistics[check_type].bucket[new_current_bucket];
      check_statistics[check_type].current_bucket = new_current_bucket;
      check_statistics[check_type].bucket[new_current_bucket] = 0;
    }
#ifdef DEBUG_CHECK_STATS
    else
      printf("GEN NO CLEARING NEEDED: TYPE[%d], CURRENT=%lu, LASTUPDATE=%lu\n",
	     check_type,
	     (unsigned long)current_time,
	     (unsigned long)check_statistics[check_type].last_update);
#endif
    /* update last check time */
    check_statistics[check_type].last_update = current_time;
  }

  /* determine weights to use for this/last buckets */
  seconds = ((unsigned long)current_time - (unsigned long)program_start) % 60;
  this_bucket_weight = (seconds / 60.0);
  last_bucket_weight = ((60 - seconds) / 60.0);

  /* update statistics for all check types */
  for (check_type = 0; check_type < MAX_CHECK_STATS_TYPES; check_type++) {
    /* clear the old statistics */
    for (x = 0; x < 3; x++)
      check_statistics[check_type].minute_stats[x] = 0;

    /* loop through each bucket */
    for (x = 0; x < CHECK_STATS_BUCKETS; x++) {
      /* which buckets should we use for this/last bucket? */
      this_bucket = (check_statistics[check_type].current_bucket + CHECK_STATS_BUCKETS - x) % CHECK_STATS_BUCKETS;
      last_bucket = (this_bucket + CHECK_STATS_BUCKETS - 1) % CHECK_STATS_BUCKETS;

      /* raw/unweighted value for this bucket */
      this_bucket_value = check_statistics[check_type].bucket[this_bucket];

      /* raw/unweighted value for last bucket - use overflow bucket if last bucket is current bucket */
      if (last_bucket == check_statistics[check_type].current_bucket)
        last_bucket_value = check_statistics[check_type].overflow_bucket;
      else
        last_bucket_value = check_statistics[check_type].bucket[last_bucket];

      /* determine value by weighting this/last buckets... */
      /* if this is the current bucket, use its full value + weighted % of last bucket */
      if (x == 0) {
#ifdef DEBUG_CHECK_STATS2
        right_value = this_bucket_value;
        left_value = (int)floor(last_bucket_value * last_bucket_weight);
#endif
        bucket_value = (int)(this_bucket_value + floor(last_bucket_value * last_bucket_weight));
      }
      /* otherwise use weighted % of this and last bucket */
      else {
#ifdef DEBUG_CHECK_STATS2
        right_value = (int)ceil(this_bucket_value * this_bucket_weight);
        left_value = (int)floor(last_bucket_value * last_bucket_weight);
#endif
        bucket_value = (int)(ceil(this_bucket_value * this_bucket_weight) + floor(last_bucket_value * last_bucket_weight));
      }

      /* 1 minute stats */
      if (x == 0)
        check_statistics[check_type].minute_stats[0] = bucket_value;

      /* 5 minute stats */
      if (x < 5)
        check_statistics[check_type].minute_stats[1] += bucket_value;

      /* 15 minute stats */
      if (x < 15)
        check_statistics[check_type].minute_stats[2] += bucket_value;

#ifdef DEBUG_CHECK_STATS2
      printf("X=%d, THIS[%d]=%d, LAST[%d]=%d, 1/5/15=%d,%d,%d  L=%d R=%d\n",
	     x,
	     this_bucket,
	     this_bucket_value,
	     last_bucket,
	     last_bucket_value,
	     check_statistics[check_type].minute_stats[0],
	     check_statistics[check_type].minute_stats[1],
	     check_statistics[check_type].minute_stats[2],
	     left_value,
	     right_value);
#endif
      /* record last update time */
      check_statistics[check_type].last_update = current_time;
    }

#ifdef DEBUG_CHECK_STATS
    printf("TYPE[%d]   1/5/15 = %d, %d, %d (seconds=%d, this_weight=%f, last_weight=%f)\n",
	   check_type,
	   check_statistics[check_type].minute_stats[0],
	   check_statistics[check_type].minute_stats[1],
	   check_statistics[check_type].minute_stats[2],
	   seconds,
	   this_bucket_weight,
	   last_bucket_weight);
#endif
  }

  return (OK);
}

/******************************************************************/
/*********************** CLEANUP FUNCTIONS ************************/
/******************************************************************/

/**
 *  Do some cleanup before we exit.
 */
void cleanup() {
  // Unload modules.
  if ((FALSE == test_scheduling) && (FALSE == verify_config)) {
    neb_free_callback_list();
    neb_unload_all_modules(
      NEBMODULE_FORCE_UNLOAD,
      (TRUE == sigshutdown)
      ? NEBMODULE_NEB_SHUTDOWN
      : NEBMODULE_NEB_RESTART);
    neb_free_module_list();
    neb_deinit_modules();
  }

  // Free all allocated memory - including macros.
  free_memory(get_global_macros());

  // Unload singletons.
  broker::compatibility::unload();
  broker::loader::unload();
  events::loop::unload();
  checks::checker::unload();
  commands::set::unload();

  return ;
}

/* free the memory allocated to the linked lists */
void free_memory(nagios_macros* mac) {
  timed_event* this_event = NULL;
  timed_event* next_event = NULL;

  /* free all allocated memory for the object definitions */
  free_object_data();

  /* free memory allocated to comments */
  free_comment_data();

  /* free check result list */
  // free_check_result_list(); // XXX: keep for compatibility layer.

  /* free memory for the high priority event list */
  this_event = event_list_high;
  while (this_event != NULL) {
    next_event = this_event->next;
    delete this_event;
    this_event = next_event;
  }

  /* reset the event pointer */
  event_list_high = NULL;
  quick_timed_event.clear(hash_timed_event::high);

  /* free memory for the low priority event list */
  this_event = event_list_low;
  while (this_event != NULL) {
    next_event = this_event->next;
    delete this_event;
    this_event = next_event;
  }

  /* reset the event pointer */
  event_list_low = NULL;
  quick_timed_event.clear(hash_timed_event::low);

  /* free any notification list that may have been overlooked */
  free_notification_list();

  /*
   * free memory associated with macros.
   * It's ok to only free the volatile ones, as the non-volatile
   * are always free()'d before assignment if they're set.
   * Doing a full free of them here means we'll wipe the constant
   * macros when we get a reload or restart request through the
   * command pipe, or when we receive a SIGHUP.
   */
  clear_volatile_macros_r(mac);

  free_macrox_names();

  /* free illegal char strings */
  // my_free(illegal_object_chars);
  // my_free(illegal_output_chars);
}

/* free a notification list that was created */
void free_notification_list(void) {
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
}

/* reset all system-wide variables, so when we've receive a SIGHUP we can restart cleanly */
int reset_variables(void) {
  config.reset();

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
