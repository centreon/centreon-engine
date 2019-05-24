/*
** Copyright 1999-2010      Ethan Galstad
** Copyright 2011-2013,2016 Centreon
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
static char* get_service_check_type(com::centreon::engine::service& svc, nagios_macros* mac) {
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
static char* get_service_group_names(com::centreon::engine::service& svc, nagios_macros* mac) {
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
template <int (com::centreon::engine::service::* member)>
static char* get_service_state(com::centreon::engine::service& svc, nagios_macros* mac) {
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

/**
 *  Extract the service id.
 *
 *  @param[in] svc  The service
 *  @param[in] mac  Unused.
 *
 *  @return  Newly allocated string with the service id.
 */
static char* get_service_id(com::centreon::engine::service& svc, nagios_macros* mac) {
  (void)mac;
  return string::dup(string::from(com::centreon::engine::get_service_id(
                                             svc.get_hostname(),
                                             svc.get_description())).c_str());
}

/**
 *  Get the timezone of a service.
 *
 *  @param[in] svc Service object.
 *  @param[in] mac Macro array.
 *
 *  @return Newly allocated string with requested value in plain text.
 */
static char* get_service_macro_timezone(com::centreon::engine::service& svc, nagios_macros* mac) {
  (void)mac;
  return (string::dup(get_service_timezone(svc.get_hostname(), svc.get_description())));
}

/**************************************
*                                     *
*         Redirection Object          *
*                                     *
**************************************/

// Redirection object.
struct grab_service_redirection {
  typedef umap<
      unsigned int,
      std::pair<char* (*)(com::centreon::engine::service&, nagios_macros* mac),
                bool>>
      entry;
  entry routines{
      // Description.
      {MACRO_SERVICEDESC,
       {&get_member_as_string<com::centreon::engine::service,
                              std::string const&,
                              &com::centreon::engine::service::get_description>,
        true}},
      // Display name.
      {MACRO_SERVICEDISPLAYNAME,
       {&get_member_as_string<service,
                              std::string const&,
                              notifier,
                              &notifier::get_display_name>,
        true}},
      // Output.
      {MACRO_SERVICEOUTPUT,
       {&get_member_as_string<com::centreon::engine::service,
                              char*,
                              &com::centreon::engine::service::plugin_output>,
        true}},
      // Long output.
      {MACRO_LONGSERVICEOUTPUT,
       {&get_member_as_string<
            com::centreon::engine::service,
            char*,
            &com::centreon::engine::service::long_plugin_output>,
        true}},
      // Perfdata.
      {MACRO_SERVICEPERFDATA,
       {&get_member_as_string<com::centreon::engine::service,
                              char*,
                              &com::centreon::engine::service::perf_data>,
        true}},
      // Check command.
      {MACRO_SERVICECHECKCOMMAND,
       {&get_member_as_string<service,
                              std::string const&,
                              notifier,
                              &notifier::get_check_command>,
        true}},
      // Check type.
      {MACRO_SERVICECHECKTYPE, {&get_service_check_type, true}},
      // State type.
      {MACRO_SERVICESTATETYPE,
       {&get_state_type_old<com::centreon::engine::service>, true}},
      // State.
      {MACRO_SERVICESTATE,
       {&get_service_state<&com::centreon::engine::service::current_state>,
        true}},
      // State ID.
      {MACRO_SERVICESTATEID,
       {&get_member_as_string<com::centreon::engine::service,
                              int,
                              &com::centreon::engine::service::current_state>,
        true}},
      // Last state.
      {MACRO_LASTSERVICESTATE,
       {&get_service_state<&com::centreon::engine::service::last_state>, true}},
      // Last state ID.
      {MACRO_LASTSERVICESTATEID,
       {&get_member_as_string<com::centreon::engine::service,
                              int,
                              &com::centreon::engine::service::last_state>,
        true}},
      // Is volatile.
      {MACRO_SERVICEISVOLATILE,
       {&get_member_as_string<com::centreon::engine::service,
                              int,
                              &com::centreon::engine::service::is_volatile>,
        true}},
      // Attempt.
      {MACRO_SERVICEATTEMPT,
       {&get_member_as_string<com::centreon::engine::service,
                              int,
                              &com::centreon::engine::service::current_attempt>,
        true}},
      // Max attempts.
      {MACRO_MAXSERVICEATTEMPTS,
       {&get_member_as_string<com::centreon::engine::service,
                              int,
                              &com::centreon::engine::service::max_attempts>,
        true}},
      // Execution time.
      {MACRO_SERVICEEXECUTIONTIME,
       {&get_double<com::centreon::engine::service,
                    &com::centreon::engine::service::execution_time,
                    3>,
        true}},
      // Latency.
      {MACRO_SERVICELATENCY,
       {&get_double<com::centreon::engine::service,
                    &com::centreon::engine::service::latency,
                    3>,
        true}},
      // Last check.
      {MACRO_LASTSERVICECHECK,
       {&get_member_as_string<com::centreon::engine::service,
                              time_t,
                              &com::centreon::engine::service::last_check>,
        true}},
      // Last state change.
      {MACRO_LASTSERVICESTATECHANGE,
       {&get_member_as_string<
            com::centreon::engine::service,
            time_t,
            &com::centreon::engine::service::last_state_change>,
        true}},
      // Last time ok.
      {MACRO_LASTSERVICEOK,
       {&get_member_as_string<com::centreon::engine::service,
                              time_t,
                              &com::centreon::engine::service::last_time_ok>,
        true}},
      // Last time warning.
      {MACRO_LASTSERVICEWARNING,
       {&get_member_as_string<
            com::centreon::engine::service,
            time_t,
            &com::centreon::engine::service::last_time_warning>,
        true}},
      // Last time unknown.
      {MACRO_LASTSERVICEUNKNOWN,
       {&get_member_as_string<
            com::centreon::engine::service,
            time_t,
            &com::centreon::engine::service::last_time_unknown>,
        true}},
      // Last time critical.
      {MACRO_LASTSERVICECRITICAL,
       {&get_member_as_string<
            com::centreon::engine::service,
            time_t,
            &com::centreon::engine::service::last_time_critical>,
        true}},
      // Downtime.
      {MACRO_SERVICEDOWNTIME,
       {&get_member_as_string<
            com::centreon::engine::service,
            int,
            &com::centreon::engine::service::scheduled_downtime_depth>,
        true}},
      // Percent state change.
      {MACRO_SERVICEPERCENTCHANGE,
       {&get_double<com::centreon::engine::service,
                    &com::centreon::engine::service::percent_state_change,
                    2>,
        true}},
      // Duration.
      {MACRO_SERVICEDURATION,
       {&get_duration_old<com::centreon::engine::service>, true}},
      // Duration in seconds.
      {MACRO_SERVICEDURATIONSEC,
       {&get_duration_sec_old<com::centreon::engine::service>, true}},
      // Notification number.
      {MACRO_SERVICENOTIFICATIONNUMBER,
       {&get_member_as_string<
            com::centreon::engine::service,
            int,
            &com::centreon::engine::service::current_notification_number>,
        true}},
      // Notification ID.
      {MACRO_SERVICENOTIFICATIONID,
       {&get_member_as_string<
            com::centreon::engine::service,
            unsigned long,
            &com::centreon::engine::service::current_notification_id>,
        true}},
      // Event ID.
      {MACRO_SERVICEEVENTID,
       {&get_member_as_string<
            com::centreon::engine::service,
            unsigned long,
            &com::centreon::engine::service::current_event_id>,
        true}},
      // Last event ID.
      {MACRO_LASTSERVICEEVENTID,
       {&get_member_as_string<com::centreon::engine::service,
                              unsigned long,
                              &com::centreon::engine::service::last_event_id>,
        true}},
      // Problem ID.
      {MACRO_SERVICEPROBLEMID,
       {&get_member_as_string<
            com::centreon::engine::service,
            unsigned long,
            &com::centreon::engine::service::current_problem_id>,
        true}},
      // Last problem ID.
      {MACRO_LASTSERVICEPROBLEMID,
       {&get_member_as_string<com::centreon::engine::service,
                              unsigned long,
                              &com::centreon::engine::service::last_problem_id>,
        true}},
      // Action URL.
      {MACRO_SERVICEACTIONURL,
       {&get_recursive<com::centreon::engine::service,
                       &com::centreon::engine::service::action_url,
                       URL_ENCODE_MACRO_CHARS>,
        true}},
      // Notes URL.
      {MACRO_SERVICENOTESURL,
       {&get_recursive<com::centreon::engine::service,
                       &com::centreon::engine::service::notes_url,
                       URL_ENCODE_MACRO_CHARS>,
        true}},
      // Notes.
      {MACRO_SERVICENOTES,
       {&get_recursive<com::centreon::engine::service,
                       &com::centreon::engine::service::notes,
                       0>,
        true}},
      // Group names.
      {MACRO_SERVICEGROUPNAMES, {&get_service_group_names, true}},
      // Acknowledgement author.
      {MACRO_SERVICEACKAUTHOR,
       {&get_macro_copy<com::centreon::engine::service, MACRO_SERVICEACKAUTHOR>,
        true}},
      // Acknowledgement author name.
      {MACRO_SERVICEACKAUTHORNAME,
       {&get_macro_copy<com::centreon::engine::service,
                        MACRO_SERVICEACKAUTHORNAME>,
        true}},
      // Acknowledgement author alias.
      {MACRO_SERVICEACKAUTHORALIAS,
       {&get_macro_copy<com::centreon::engine::service,
                        MACRO_SERVICEACKAUTHORALIAS>,
        true}},
      // Acknowledgement comment.
      {MACRO_SERVICEACKCOMMENT,
       {&get_macro_copy<com::centreon::engine::service,
                        MACRO_SERVICEACKCOMMENT>,
        true}},
      // Service id.
      {MACRO_SERVICEID, {&get_service_id, true}},
      // Acknowledgement comment.
      {MACRO_SERVICEACKCOMMENT,
       {&get_macro_copy<com::centreon::engine::service,
                        MACRO_SERVICEACKCOMMENT>,
        true}},
      // Acknowledgement comment.
      {MACRO_SERVICETIMEZONE, {&get_service_macro_timezone, true}}};
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
      com::centreon::engine::service* svc,
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
      com::centreon::engine::service* svc,
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
int grab_service_macros_r(nagios_macros* mac, com::centreon::engine::service* svc) {
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
int grab_service_macros(com::centreon::engine::service* svc) {
  return (grab_service_macros_r(get_global_macros(), svc));
}

}
