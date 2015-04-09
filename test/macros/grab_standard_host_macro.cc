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
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/string.hh"
#include "test/macros/minimal_setup.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

// Stringification macros.
#define XSTR(x) #x
#define STR(x) XSTR(x)

// Group names.
#define GROUP1 group1
#define GROUP2 group2
#define GROUP3 group3

// Values that will be set in host.
#define NAME                    myhost
#define ALIAS                   LocalHost
#define ADDRESS                 127.0.0.1
#define STATE                   DOWN
#define STATE_ID                1
#define LAST_STATE              UNREACHABLE
#define LAST_STATE_ID           2
#define CHECK_TYPE              ACTIVE
#define STATE_TYPE              SOFT
#define OUTPUT                  my output
#define LONG_OUTPUT             my very long output
#define PERF_DATA               this is not a perfdata string
#define CHECK_COMMAND           check_this host
#define ATTEMPT                 5
#define MAX_ATTEMPTS            7
#define PERCENT_CHANGE          25.72
#define EXECUTION_TIME          14.27
#define LATENCY                 74.91
#define LAST_CHECK              147852369
#define LAST_STATE_CHANGE       169874123
#define LAST_UP                 121567890
#define LAST_DOWN               187456321
#define LAST_UNREACHABLE        115678925
#define EVENT_ID                6547985
#define LAST_EVENT_ID           1253887
#define PROBLEM_ID              447786
#define LAST_PROBLEM_ID         44597587
#define GROUP_NAMES             STR(GROUP1) "," STR(GROUP2) "," STR(GROUP3)
#define TOTAL_SERVICES          1
#define TOTAL_SERVICES_OK       1
#define TOTAL_SERVICES_WARNING  0
#define TOTAL_SERVICES_UNKNOWN  0
#define TOTAL_SERVICES_CRITICAL 0

/**
 *  Check that the grab_standard_host_macro function works properly.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Create minimal context.
  test::minimal_setup();

  // Service must be OK.
  service_list->current_state = 0;
  service_list->last_hard_state = 0;

  // Add host groups and link them to host.
  hostgroup* hg1(add_hostgroup(const_cast<char*>(STR(GROUP1)),
    NULL));
  hostgroup* hg2(add_hostgroup(const_cast<char*>(STR(GROUP2)),
    NULL));
  hostgroup* hg3(add_hostgroup(const_cast<char*>(STR(GROUP3)),
    NULL));
  add_host_to_hostgroup(hg1, host_list->name);
  add_host_to_hostgroup(hg2, host_list->name);
  add_host_to_hostgroup(hg3, host_list->name);
  add_object_to_objectlist(&host_list->hostgroups_ptr, hg3);
  add_object_to_objectlist(&host_list->hostgroups_ptr, hg2);
  add_object_to_objectlist(&host_list->hostgroups_ptr, hg1);

  // Set host values.
  delete [] host_list->name;
  host_list->name = string::dup(STR(NAME));
  delete [] host_list->alias;
  host_list->alias = string::dup(STR(ALIAS));
  delete [] host_list->address;
  host_list->address = string::dup(STR(ADDRESS));
  host_list->current_state = STATE_ID;
  host_list->last_state = LAST_STATE_ID;
  host_list->check_type = HOST_CHECK_ACTIVE;
  host_list->state_type = SOFT_STATE;
  delete [] host_list->plugin_output;
  host_list->plugin_output = string::dup(STR(OUTPUT));
  delete [] host_list->long_plugin_output;
  host_list->long_plugin_output = string::dup(STR(LONG_OUTPUT));
  delete [] host_list->perf_data;
  host_list->perf_data = string::dup(STR(PERF_DATA));
  delete [] host_list->host_check_command;
  host_list->host_check_command = string::dup(STR(CHECK_COMMAND));
  host_list->current_attempt = ATTEMPT;
  host_list->max_attempts = MAX_ATTEMPTS;
  host_list->percent_state_change = PERCENT_CHANGE;
  host_list->execution_time = EXECUTION_TIME;
  host_list->latency = LATENCY;
  host_list->last_check = LAST_CHECK;
  host_list->last_state_change = LAST_STATE_CHANGE;
  host_list->last_time_up = LAST_UP;
  host_list->last_time_down = LAST_DOWN;
  host_list->last_time_unreachable = LAST_UNREACHABLE;
  host_list->current_event_id = EVENT_ID;
  host_list->last_event_id = LAST_EVENT_ID;
  host_list->current_problem_id = PROBLEM_ID;
  host_list->last_problem_id = LAST_PROBLEM_ID;

  // macro object.
  nagios_macros mac;
  memset(&mac, 0, sizeof(mac));

  // Macro values table.
  struct {
    unsigned int macro_id;
    char const*  expected_value;
    bool         is_double;
  } static const macro_values[] = {
    { MACRO_HOSTNAME, STR(NAME), false },
    { MACRO_HOSTALIAS, STR(ALIAS), false },
    { MACRO_HOSTADDRESS, STR(ADDRESS), false },
    { MACRO_HOSTSTATE, STR(STATE), false },
    { MACRO_HOSTSTATEID, STR(STATE_ID), false },
    { MACRO_LASTHOSTSTATE, STR(LAST_STATE), false },
    { MACRO_LASTHOSTSTATEID, STR(LAST_STATE_ID), false },
    { MACRO_HOSTCHECKTYPE, STR(CHECK_TYPE), false },
    { MACRO_HOSTSTATETYPE, STR(STATE_TYPE), false },
    { MACRO_HOSTOUTPUT, STR(OUTPUT), false },
    { MACRO_LONGHOSTOUTPUT, STR(LONG_OUTPUT), false },
    { MACRO_HOSTPERFDATA, STR(PERF_DATA), false },
    { MACRO_HOSTCHECKCOMMAND, STR(CHECK_COMMAND), false },
    { MACRO_HOSTATTEMPT, STR(ATTEMPT), false },
    { MACRO_MAXHOSTATTEMPTS, STR(MAX_ATTEMPTS), false },
    { MACRO_HOSTPERCENTCHANGE, STR(PERCENT_CHANGE), true },
    { MACRO_HOSTEXECUTIONTIME, STR(EXECUTION_TIME), true },
    { MACRO_HOSTLATENCY, STR(LATENCY), true },
    { MACRO_LASTHOSTCHECK, STR(LAST_CHECK), false },
    { MACRO_LASTHOSTSTATECHANGE, STR(LAST_STATE_CHANGE), false },
    { MACRO_LASTHOSTUP, STR(LAST_UP), false },
    { MACRO_LASTHOSTDOWN, STR(LAST_DOWN), false },
    { MACRO_LASTHOSTUNREACHABLE, STR(LAST_UNREACHABLE), false },
    { MACRO_HOSTEVENTID, STR(EVENT_ID), false },
    { MACRO_LASTHOSTEVENTID, STR(LAST_EVENT_ID), false },
    { MACRO_HOSTPROBLEMID, STR(PROBLEM_ID), false },
    { MACRO_LASTHOSTPROBLEMID, STR(LAST_PROBLEM_ID), false },
    { MACRO_HOSTGROUPNAMES, GROUP_NAMES, false },
    { MACRO_TOTALHOSTSERVICES, STR(TOTAL_SERVICES), false },
    { MACRO_TOTALHOSTSERVICESOK, STR(TOTAL_SERVICES_OK), false },
    { MACRO_TOTALHOSTSERVICESWARNING, STR(TOTAL_SERVICES_WARNING), false },
    { MACRO_TOTALHOSTSERVICESUNKNOWN, STR(TOTAL_SERVICES_UNKNOWN), false },
    { MACRO_TOTALHOSTSERVICESCRITICAL, STR(TOTAL_SERVICES_CRITICAL), false }
  };

  // Compare macros with expected values.
  int retval(0);
  for (unsigned int i = 0;
       i < sizeof(macro_values) / sizeof(*macro_values);
       ++i) {
    char* output(NULL);
    int free_macro;
    if (grab_standard_host_macro_r(&mac,
          macro_values[i].macro_id,
          host_list,
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
    }
  }

  delete [] mac.x[MACRO_TOTALHOSTSERVICES];
  delete [] mac.x[MACRO_TOTALHOSTSERVICESOK];
  delete [] mac.x[MACRO_TOTALHOSTSERVICESWARNING];
  delete [] mac.x[MACRO_TOTALHOSTSERVICESUNKNOWN];
  delete [] mac.x[MACRO_TOTALHOSTSERVICESCRITICAL];

  return (retval);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
