/*
** Copyright 1999-2010      Ethan Galstad
** Copyright 2011-2013,2015 Merethis
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
#include <utility>
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros/clear_service.hh"
#include "com/centreon/engine/macros/clear_servicegroup.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/macros/grab.hh"
#include "com/centreon/engine/macros/grab_service.hh"
#include "com/centreon/engine/macros/misc.hh"
#include "com/centreon/engine/objects/objectlist.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/unordered_hash.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::macros;
using namespace com::centreon::engine::logging;

/**************************************
*                                     *
*           Local Functions           *
*                                     *
**************************************/

/**
 *  Extract service check type.
 *
 *  @param[in] svc Service object.
 *  @param[in] mac Unused.
 *
 *  @return Newly allocated string containing either "PASSIVE" or
 *          "ACTIVE".
 */
static char* get_service_check_type(service& svc, nagios_macros* mac) {
  (void)mac;
  return (string::dup(
            (SERVICE_CHECK_PASSIVE == svc.check_type
             ? "PASSIVE"
             : "ACTIVE")));
}

/**
 *  Extract service group names.
 *
 *  @param[in] svc Target service.
 *  @param[in] mac Unused.
 *
 *  @return List of names of groups associated with this service.
 */
static char* get_service_group_names(service& svc, nagios_macros* mac) {
  (void)mac;

  // Find all servicegroups this service is associated with.
  std::string buf;
  for (objectlist* temp_objectlist = svc.servicegroups_ptr;
       temp_objectlist != NULL;
       temp_objectlist = temp_objectlist->next) {
    servicegroup* temp_servicegroup(
      static_cast<servicegroup*>(temp_objectlist->object_ptr));
    if (temp_servicegroup) {
      if (!buf.empty())
        buf.append(",");
      buf.append(temp_servicegroup->group_name);
    }
  }
  return (string::dup(buf));
}

/**
 *  Extract service state.
 *
 *  @param[in] svc Service object.
 *  @param[in] mac Unused.
 *
 *  @return Newly allocated string with host state as plain text.
 */
template <int (service::* member)>
static char* get_service_state(service& svc, nagios_macros* mac) {
  (void)mac;
  char const* state;
  if (STATE_OK == svc.*member)
    state = "OK";
  else if (STATE_WARNING == svc.*member)
    state = "WARNING";
  else if (STATE_CRITICAL == svc.*member)
    state = "CRITICAL";
  else
    state = "UNKNOWN";
  return (string::dup(state));
}

/**************************************
*                                     *
*         Redirection Object          *
*                                     *
**************************************/

// Redirection object.
struct grab_service_redirection {
  typedef umap<unsigned int, std::pair<char* (*)(service&, nagios_macros* mac), bool> > entry;
  entry routines;
  grab_service_redirection() {
    // Description.
    routines[MACRO_SERVICEDESC].first = &get_member_as_string<service, char*, &service::description>;
    routines[MACRO_SERVICEDESC].second = true;
    // Display name.
    routines[MACRO_SERVICEDISPLAYNAME].first = &get_member_as_string<service, char*, &service::display_name>;
    routines[MACRO_SERVICEDISPLAYNAME].second = true;
    // Output.
    routines[MACRO_SERVICEOUTPUT].first = &get_member_as_string<service, char*, &service::plugin_output>;
    routines[MACRO_SERVICEOUTPUT].second = true;
    // Long output.
    routines[MACRO_LONGSERVICEOUTPUT].first = &get_member_as_string<service, char*, &service::long_plugin_output>;
    routines[MACRO_LONGSERVICEOUTPUT].second = true;
    // Perfdata.
    routines[MACRO_SERVICEPERFDATA].first = &get_member_as_string<service, char*, &service::perf_data>;
    routines[MACRO_SERVICEPERFDATA].second = true;
    // Check command.
    routines[MACRO_SERVICECHECKCOMMAND].first = &get_member_as_string<service, char*, &service::service_check_command>;
    routines[MACRO_SERVICECHECKCOMMAND].second = true;
    // Check type.
    routines[MACRO_SERVICECHECKTYPE].first = &get_service_check_type;
    routines[MACRO_SERVICECHECKTYPE].second = true;
    // State type.
    routines[MACRO_SERVICESTATETYPE].first = &get_state_type<service>;
    routines[MACRO_SERVICESTATETYPE].second = true;
    // State.
    routines[MACRO_SERVICESTATE].first = &get_service_state<&service::current_state>;
    routines[MACRO_SERVICESTATE].second = true;
    // State ID.
    routines[MACRO_SERVICESTATEID].first = &get_member_as_string<service, int, &service::current_state>;
    routines[MACRO_SERVICESTATEID].second = true;
    // Last state.
    routines[MACRO_LASTSERVICESTATE].first = &get_service_state<&service::last_state>;
    routines[MACRO_LASTSERVICESTATE].second = true;
    // Last state ID.
    routines[MACRO_LASTSERVICESTATEID].first = &get_member_as_string<service, int, &service::last_state>;
    routines[MACRO_LASTSERVICESTATEID].second = true;
    // Is volatile.
    routines[MACRO_SERVICEISVOLATILE].first = &get_member_as_string<service, int, &service::is_volatile>;
    routines[MACRO_SERVICEISVOLATILE].second = true;
    // Attempt.
    routines[MACRO_SERVICEATTEMPT].first = &get_member_as_string<service, int, &service::current_attempt>;
    routines[MACRO_SERVICEATTEMPT].second = true;
    // Max attempts.
    routines[MACRO_MAXSERVICEATTEMPTS].first = &get_member_as_string<service, int, &service::max_attempts>;
    routines[MACRO_MAXSERVICEATTEMPTS].second = true;
    // Execution time.
    routines[MACRO_SERVICEEXECUTIONTIME].first = &get_double<service, &service::execution_time, 3>;
    routines[MACRO_SERVICEEXECUTIONTIME].second = true;
    // Latency.
    routines[MACRO_SERVICELATENCY].first = &get_double<service, &service::latency, 3>;
    routines[MACRO_SERVICELATENCY].second = true;
    // Last check.
    routines[MACRO_LASTSERVICECHECK].first = &get_member_as_string<service, time_t, &service::last_check>;
    routines[MACRO_LASTSERVICECHECK].second = true;
    // Last state change.
    routines[MACRO_LASTSERVICESTATECHANGE].first = &get_member_as_string<service, time_t, &service::last_state_change>;
    routines[MACRO_LASTSERVICESTATECHANGE].second = true;
    // Last time ok.
    routines[MACRO_LASTSERVICEOK].first = &get_member_as_string<service, time_t, &service::last_time_ok>;
    routines[MACRO_LASTSERVICEOK].second = true;
    // Last time warning.
    routines[MACRO_LASTSERVICEWARNING].first = &get_member_as_string<service, time_t, &service::last_time_warning>;
    routines[MACRO_LASTSERVICEWARNING].second = true;
    // Last time unknown.
    routines[MACRO_LASTSERVICEUNKNOWN].first = &get_member_as_string<service, time_t, &service::last_time_unknown>;
    routines[MACRO_LASTSERVICEUNKNOWN].second = true;
    // Last time critical.
    routines[MACRO_LASTSERVICECRITICAL].first = &get_member_as_string<service, time_t, &service::last_time_critical>;
    routines[MACRO_LASTSERVICECRITICAL].second = true;
    // Downtime.
    routines[MACRO_SERVICEDOWNTIME].first = &get_member_as_string<service, int, &service::scheduled_downtime_depth>;
    routines[MACRO_SERVICEDOWNTIME].second = true;
    // Percent state change.
    routines[MACRO_SERVICEPERCENTCHANGE].first = &get_double<service, &service::percent_state_change, 2>;
    routines[MACRO_SERVICEPERCENTCHANGE].second = true;
    // Duration.
    routines[MACRO_SERVICEDURATION].first = &get_duration<service>;
    routines[MACRO_SERVICEDURATION].second = true;
    // Duration in seconds.
    routines[MACRO_SERVICEDURATIONSEC].first = &get_duration_sec<service>;
    routines[MACRO_SERVICEDURATIONSEC].second = true;
    // Notification number.
    routines[MACRO_SERVICENOTIFICATIONNUMBER].first = &get_member_as_string<service, int, &service::current_notification_number>;
    routines[MACRO_SERVICENOTIFICATIONNUMBER].second = true;
    // Notification ID.
    routines[MACRO_SERVICENOTIFICATIONID].first = &get_member_as_string<service, unsigned long, &service::current_notification_id>;
    routines[MACRO_SERVICENOTIFICATIONID].second = true;
    // Event ID.
    routines[MACRO_SERVICEEVENTID].first = &get_member_as_string<service, unsigned long, &service::current_event_id>;
    routines[MACRO_SERVICEEVENTID].second = true;
    // Last event ID.
    routines[MACRO_LASTSERVICEEVENTID].first = &get_member_as_string<service, unsigned long, &service::last_event_id>;
    routines[MACRO_LASTSERVICEEVENTID].second = true;
    // Problem ID.
    routines[MACRO_SERVICEPROBLEMID].first = &get_member_as_string<service, unsigned long, &service::current_problem_id>;
    routines[MACRO_SERVICEPROBLEMID].second = true;
    // Last problem ID.
    routines[MACRO_LASTSERVICEPROBLEMID].first = &get_member_as_string<service, unsigned long, &service::last_problem_id>;
    routines[MACRO_LASTSERVICEPROBLEMID].second = true;
    // Group names.
    routines[MACRO_SERVICEGROUPNAMES].first = &get_service_group_names;
    routines[MACRO_SERVICEGROUPNAMES].second = true;
    // Acknowledgement author.
    routines[MACRO_SERVICEACKAUTHOR].first = &get_macro_copy<service, MACRO_SERVICEACKAUTHOR>;
    routines[MACRO_SERVICEACKAUTHOR].second = true;
    // Acknowledgement author name.
    routines[MACRO_SERVICEACKAUTHORNAME].first = &get_macro_copy<service, MACRO_SERVICEACKAUTHORNAME>;
    routines[MACRO_SERVICEACKAUTHORNAME].second = true;
    // Acknowledgement author alias.
    routines[MACRO_SERVICEACKAUTHORALIAS].first = &get_macro_copy<service, MACRO_SERVICEACKAUTHORALIAS>;
    routines[MACRO_SERVICEACKAUTHORALIAS].second = true;
  }
} static const redirector;

/**************************************
*                                     *
*          Global Functions           *
*                                     *
**************************************/

extern "C" {
/**
 *  Grab a standard service macro.
 *
 *  @param[out] mac        Macro array.
 *  @param[in]  macro_type Macro to dump.
 *  @param[in]  svc        Target service.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer should be free
 *                         by caller.
 *
 *  @return OK on success.
 */
int grab_standard_service_macro_r(
      nagios_macros* mac,
      int macro_type,
      service* svc,
      char** output,
      int* free_macro) {
  // Check that function was called with valid arguments.
  int retval;
  if (svc && output && free_macro) {
    grab_service_redirection::entry::const_iterator it(
      redirector.routines.find(macro_type));
    // Found matching routine.
    if (it != redirector.routines.end()) {
      // Call routine.
      *output = (*it->second.first)(*svc, mac);

      // Set the free macro flag.
      *free_macro = it->second.second;

      // Successful execution.
      retval = OK;
    }
    // Non-existent macro.
    else {
      logger(dbg_macros, basic)
        << "UNHANDLED SERVICE MACRO #" << macro_type
        << "! THIS IS A BUG!";
      retval = ERROR;
    }
  }
  else
    retval = ERROR;

  return (retval);
}

/**
 *  Grab a standard service macro for global macros.
 *
 *  @param[in]  macro_type Macro to dump.
 *  @param[in]  svc        Target service.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer should be free
 *                         by caller.
 *
 *  @return OK on success.
 *
 *  @see grab_standard_service_macro_r
 */
int grab_standard_service_macro(
      int macro_type,
      service* svc,
      char** output,
      int* free_macro) {
  return (grab_standard_service_macro_r(
            get_global_macros(),
            macro_type,
            svc,
            output,
            free_macro));
}

/**
 *  Grab macros that are specific to a service.
 *
 *  @param[in] mac Macros object.
 *  @param[in] svc Service pointer.
 *
 *  @return OK on success.
 */
int grab_service_macros_r(nagios_macros* mac, service* svc) {
  // Clear service-related macros.
  clear_service_macros_r(mac);
  clear_servicegroup_macros_r(mac);

  // Save pointer for later.
  mac->service_ptr = svc;
  mac->servicegroup_ptr = NULL;

  if (svc == NULL)
    return (ERROR);

  // Save first/primary servicegroup pointer for later.
  if (svc->servicegroups_ptr)
    mac->servicegroup_ptr
      = static_cast<servicegroup*>(svc->servicegroups_ptr->object_ptr);

  return (OK);
}

/**
 *  Grab macros that are specific to a service.
 *
 *  @param[in] svc Service pointer.
 *
 *  @return OK on success.
 *
 *  @see grab_service_macros_r
 */
int grab_service_macros(service* svc) {
  return (grab_service_macros_r(get_global_macros(), svc));
}

}
