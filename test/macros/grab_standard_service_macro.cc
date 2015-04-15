/*
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

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros/grab_service.hh"
#include "com/centreon/engine/string.hh"
#include "test/macros/minimal_setup.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

// Stringification macros.
#define XSTR(x) #x
#define STR(x) XSTR(x)

// Values that will be set in service.
#define DESCRIPTION myservice
#define OUTPUT my output
#define LONG_OUTPUT my output can be much longer than this
#define PERF_DATA this is absolutely not a perfdata string
#define CHECK_COMMAND check this out!
#define CHECK_TYPE PASSIVE
#define STATE_TYPE SOFT
#define STATE CRITICAL
#define STATE_ID 2
#define LAST_STATE WARNING
#define LAST_STATE_ID 1
#define IS_VOLATILE 1
#define ATTEMPT 4
#define MAX_ATTEMPTS 32
#define EXECUTION_TIME 54.89
#define LATENCY 89.13
#define LAST_CHECK 147852369
#define LAST_STATE_CHANGE 165478236
#define LAST_OK 198771254
#define LAST_WARNING 132569874
#define LAST_UNKNOWN 102565478
#define LAST_CRITICAL 15478632
#define PERCENT_CHANGE 42.26
#define EVENT_ID 2348972
#define LAST_EVENT_ID 21384723
#define PROBLEM_ID 123900
#define LAST_PROBLEM_ID 927834

/**
 *  Check that the grab_standard_service_macro function works properly.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Create minimal context.
  test::minimal_setup();

  // Set service values.
  delete [] service_list->description;
  service_list->description = string::dup(STR(DESCRIPTION));
  delete [] service_list->plugin_output;
  service_list->plugin_output = string::dup(STR(OUTPUT));
  delete [] service_list->long_plugin_output;
  service_list->long_plugin_output = string::dup(STR(LONG_OUTPUT));
  delete [] service_list->perf_data;
  service_list->perf_data = string::dup(STR(PERF_DATA));
  delete [] service_list->service_check_command;
  service_list->service_check_command = string::dup(STR(CHECK_COMMAND));
  service_list->check_type = SERVICE_CHECK_PASSIVE;
  service_list->state_type = SOFT_STATE;
  service_list->current_state = STATE_ID;
  service_list->last_state = LAST_STATE_ID;
  service_list->is_volatile = IS_VOLATILE;
  service_list->current_attempt = ATTEMPT;
  service_list->max_attempts = MAX_ATTEMPTS;
  service_list->execution_time = EXECUTION_TIME;
  service_list->latency = LATENCY;
  service_list->last_check = LAST_CHECK;
  service_list->last_state_change = LAST_STATE_CHANGE;
  service_list->last_time_ok = LAST_OK;
  service_list->last_time_warning = LAST_WARNING;
  service_list->last_time_unknown = LAST_UNKNOWN;
  service_list->last_time_critical = LAST_CRITICAL;
  service_list->percent_state_change = PERCENT_CHANGE;
  service_list->current_event_id = EVENT_ID;
  service_list->last_event_id = LAST_EVENT_ID;
  service_list->current_problem_id = PROBLEM_ID;
  service_list->last_problem_id = LAST_PROBLEM_ID;

  // Macro object.
  nagios_macros mac;
  memset(&mac, 0, sizeof(mac));

  // Macro values table.
  struct {
    unsigned int macro_id;
    char const*  expected_value;
    bool         is_double;
  } static const macro_values[] = {
    { MACRO_SERVICEDESC, STR(DESCRIPTION), false },
    { MACRO_SERVICEOUTPUT, STR(OUTPUT), false },
    { MACRO_LONGSERVICEOUTPUT, STR(LONG_OUTPUT), false },
    { MACRO_SERVICEPERFDATA, STR(PERF_DATA), false },
    { MACRO_SERVICECHECKCOMMAND, STR(CHECK_COMMAND), false },
    { MACRO_SERVICECHECKTYPE, STR(CHECK_TYPE), false },
    { MACRO_SERVICESTATETYPE, STR(STATE_TYPE), false },
    { MACRO_SERVICESTATE, STR(STATE), false },
    { MACRO_SERVICESTATEID, STR(STATE_ID), false },
    { MACRO_LASTSERVICESTATE, STR(LAST_STATE), false },
    { MACRO_LASTSERVICESTATEID, STR(LAST_STATE_ID), false },
    { MACRO_SERVICEISVOLATILE, STR(IS_VOLATILE), false },
    { MACRO_SERVICEATTEMPT, STR(ATTEMPT), false },
    { MACRO_MAXSERVICEATTEMPTS, STR(MAX_ATTEMPTS), false },
    { MACRO_SERVICEEXECUTIONTIME, STR(EXECUTION_TIME), true },
    { MACRO_SERVICELATENCY, STR(LATENCY), true },
    { MACRO_LASTSERVICECHECK, STR(LAST_CHECK), false },
    { MACRO_LASTSERVICESTATECHANGE, STR(LAST_STATE_CHANGE), false },
    { MACRO_LASTSERVICEOK, STR(LAST_OK), false },
    { MACRO_LASTSERVICEWARNING, STR(LAST_WARNING), false },
    { MACRO_LASTSERVICEUNKNOWN, STR(LAST_UNKNOWN), false },
    { MACRO_LASTSERVICECRITICAL, STR(LAST_CRITICAL), false },
    { MACRO_SERVICEPERCENTCHANGE, STR(PERCENT_CHANGE), true },
    { MACRO_SERVICEEVENTID, STR(EVENT_ID), false },
    { MACRO_LASTSERVICEEVENTID, STR(LAST_EVENT_ID), false },
    { MACRO_SERVICEPROBLEMID, STR(PROBLEM_ID), false },
    { MACRO_LASTSERVICEPROBLEMID, STR(LAST_PROBLEM_ID), false }
  };

  // Compare macros with expected values.
  int retval(0);
  for (unsigned int i = 0;
       i < sizeof(macro_values) / sizeof(*macro_values);
       ++i) {
    char* output(NULL);
    int free_macro;
    if (grab_standard_service_macro_r(&mac,
                                      macro_values[i].macro_id,
                                      service_list,
                                      &output,
                                      &free_macro)
        != OK) {
      retval |= 1;
      std::cout << "failing macro: " << macro_values[i].macro_id
                << std::endl;
    }
    else {
      if (macro_values[i].is_double) {
        if (fabs(strtod(macro_values[i].expected_value, NULL)
                 - strtod(output, NULL))
            > 0.1) {
          retval |= 1;
          std::cout << "failing macro: " << macro_values[i].macro_id
                    << std::endl;
        }
      }
      else if (strcmp(output, macro_values[i].expected_value)) {
        retval |= 1;
        std::cout << "failing macro: " << macro_values[i].macro_id
                  << " (" << output << " != "
                  << macro_values[i].expected_value << ")"
                  << std::endl;
      }
      if (free_macro)
        delete [] output;
      else
        std::cout << macro_values[i].macro_id << std::endl;
    }
  }

  return (retval);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
