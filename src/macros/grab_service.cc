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
#include "com/centreon/engine/string.hh"

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
static std::string get_service_check_type(com::centreon::engine::service& svc, nagios_macros* mac) {
  (void)mac;
  return (checkable::check_passive == svc.get_check_type())
             ? "PASSIVE"
             : "ACTIVE";
}

/**
 *  Extract service group names.
 *
 *  @param[in] svc Target service.
 *  @param[in] mac Unused.
 *
 *  @return List of names of groups associated with this service.
 */
static std::string get_service_group_names(service& svc, nagios_macros* mac) {
  (void)mac;

  // Find all servicegroups this service is associated with.
  std::string buf;
  for (std::list<servicegroup*>::const_iterator
         it{svc.get_parent_groups().begin()},
         end{svc.get_parent_groups().end()};
       it != end;
       ++it) {
    if (*it) {
      if (!buf.empty())
        buf.append(",");
      buf.append((*it)->get_group_name());
    }
  }
  return buf;
}

/**
 *  Extract service state.
 *
 *  @param[in] svc Service object.
 *  @param[in] mac Unused.
 *
 *  @return Newly allocated string with host state as plain text.
 */
template <service::service_state ((com::centreon::engine::service::* member)() const)>
static std::string get_service_state(com::centreon::engine::service& svc, nagios_macros* mac) {
  (void)mac;
  char const* state;

  if (service::state_ok == (svc.*member)())
    state = "OK";
  else if (service::state_warning == (svc.*member)())
    state = "WARNING";
  else if (service::state_critical == (svc.*member)())
    state = "CRITICAL";
  else
    state = "UNKNOWN";
  return state;
}

/**
 *  Extract the service id.
 *
 *  @param[in] svc  The service
 *  @param[in] mac  Unused.
 *
 *  @return  Newly allocated string with the service id.
 */
static std::string get_service_id(com::centreon::engine::service& svc, nagios_macros* mac) {
  (void)mac;
  return string::from(com::centreon::engine::get_service_id(
                                             svc.get_hostname(),
                                             svc.get_description()));
}

/**
 *  Get the timezone of a service.
 *
 *  @param[in] svc Service object.
 *  @param[in] mac Macro array.
 *
 *  @return Newly allocated string with requested value in plain text.
 */
static std::string get_service_macro_timezone(com::centreon::engine::service& svc, nagios_macros* mac) {
  (void)mac;
  return svc.get_timezone();
}

/**************************************
*                                     *
*         Redirection Object          *
*                                     *
**************************************/

// Redirection object.
struct grab_service_redirection {
  typedef std::unordered_map<
      unsigned int,
      std::pair<std::string (*)(com::centreon::engine::service&, nagios_macros* mac),
                bool>>
      entry;
  entry routines{
      // Description.
      {MACRO_SERVICEDESC,
       {&get_member_as_string<service, std::string const&, &service::get_description>,
        true}},
      // Display name.
      {MACRO_SERVICEDISPLAYNAME,
       {&get_member_as_string<service, std::string const&, checkable, &checkable::get_display_name>,
        true}},
      // Output.
      {MACRO_SERVICEOUTPUT,
       {&get_member_as_string<service, std::string const&, checkable, &checkable::get_plugin_output>,
        true}},
      // Long output.
      {MACRO_LONGSERVICEOUTPUT,
       {&get_member_as_string< service, std::string const&, checkable, &checkable::get_long_plugin_output>,
        true}},
      // Perfdata.
      {MACRO_SERVICEPERFDATA,
       {&get_member_as_string<service, std::string const&, checkable, &checkable::get_perf_data>,
        true}},
      // Check command.
      {MACRO_SERVICECHECKCOMMAND,
       {&get_member_as_string<service, std::string const&, checkable, &checkable::get_check_command>,
        true}},
      // Check type.
      {MACRO_SERVICECHECKTYPE, {&get_service_check_type, true}},
      // State type.
      {MACRO_SERVICESTATETYPE,
       {&get_state_type<service>, true}},
      // State.
      {MACRO_SERVICESTATE,
       {&get_service_state<&service::get_current_state>, true}},
      // State ID.
      {MACRO_SERVICESTATEID,
       {&get_member_as_string<service, service::service_state, &service::get_current_state>,
        true}},
      // Last state.
      {MACRO_LASTSERVICESTATE,
       {&get_service_state<&service::get_last_state>, true}},
      // Last state ID.
      {MACRO_LASTSERVICESTATEID,
       {&get_member_as_string<service, service::service_state, &service::get_last_state>,
        true}},
      // Is volatile.
      {MACRO_SERVICEISVOLATILE,
       {&get_member_as_string<service, bool, &service::get_is_volatile>,
        true}},
      // Attempt.
      {MACRO_SERVICEATTEMPT,
       {&get_member_as_string<service, int, checkable, &checkable::get_current_attempt>,
        true}},
      // Max attempts.
      {MACRO_MAXSERVICEATTEMPTS,
       {&get_member_as_string<service, int, checkable, &checkable::get_max_attempts>,
        true}},
      // Execution time.
      {MACRO_SERVICEEXECUTIONTIME,
       {&get_double<service, checkable,
                    &checkable::get_execution_time,
                    3>,
        true}},
      // Latency.
      {MACRO_SERVICELATENCY,
       {&get_double<service, checkable, &checkable::get_latency, 3>,
        true}},
      // Last check.
      {MACRO_LASTSERVICECHECK,
       {&get_member_as_string<service, time_t, checkable, &checkable::get_last_check>,
        true}},
      // Last state change.
      {MACRO_LASTSERVICESTATECHANGE,
       {&get_member_as_string<service, time_t, checkable, &checkable::get_last_state_change>,
        true}},
      // Last time ok.
      {MACRO_LASTSERVICEOK,
       {&get_member_as_string<service, time_t, &service::get_last_time_ok>,
        true}},
      // Last time warning.
      {MACRO_LASTSERVICEWARNING,
       {&get_member_as_string<service, time_t, &service::get_last_time_warning>,
        true}},
      // Last time unknown.
      {MACRO_LASTSERVICEUNKNOWN,
       {&get_member_as_string< service, time_t, &service::get_last_time_unknown>,
        true}},
      // Last time critical.
      {MACRO_LASTSERVICECRITICAL,
       {&get_member_as_string< service, time_t, &service::get_last_time_critical>,
        true}},
      // Downtime.
      {MACRO_SERVICEDOWNTIME,
       {&get_member_as_string<
            service,
            int,
            checkable,
            &checkable::get_scheduled_downtime_depth>,
        true}},
      // Percent state change.
      {MACRO_SERVICEPERCENTCHANGE,
       {&get_double<service, checkable, &checkable::get_percent_state_change, 2>,
        true}},
      // Duration.
      {MACRO_SERVICEDURATION,
       {&get_duration_old<service>, true}},
      // Duration in seconds.
      {MACRO_SERVICEDURATIONSEC,
       {&get_duration_sec_old<service>, true}},
      // Notification number.
      {MACRO_SERVICENOTIFICATIONNUMBER,
       {&get_member_as_string< service, int, notifier, &notifier::get_notification_number>,
        true}},
      // Notification ID.
      {MACRO_SERVICENOTIFICATIONID,
       {&get_member_as_string<service, uint64_t, notifier, &notifier::get_current_notification_id>,
        true}},
      // Event ID.
      {MACRO_SERVICEEVENTID,
       {&get_member_as_string<service, unsigned long, notifier, &notifier::get_current_event_id>,
        true}},
      // Last event ID.
      {MACRO_LASTSERVICEEVENTID,
        {&get_member_as_string<service, unsigned long, notifier, &notifier::get_last_event_id>,
        true}},
      // Problem ID.
      {MACRO_SERVICEPROBLEMID,
       {&get_member_as_string<
            service,
            unsigned long,
            notifier,
            &notifier::get_current_problem_id>,
        true}},
      // Last problem ID.
      {MACRO_LASTSERVICEPROBLEMID,
       {&get_member_as_string<service, unsigned long, notifier, &notifier::get_last_problem_id>,
        true}},
      // Action URL.
      {MACRO_SERVICEACTIONURL,
       {&get_recursive<service, checkable, &checkable::get_action_url, URL_ENCODE_MACRO_CHARS>,
        true}},
      // Notes URL.
      {MACRO_SERVICENOTESURL,
       {&get_recursive<service, checkable, &checkable::get_notes_url, URL_ENCODE_MACRO_CHARS>,
        true}},
      // Notes.
      {MACRO_SERVICENOTES,
       {&get_recursive<service, checkable, &checkable::get_notes,0>,
        true}},
      // Group names.
      {MACRO_SERVICEGROUPNAMES, {&get_service_group_names, true}},
      // Acknowledgement author.
      {MACRO_SERVICEACKAUTHOR,
       {&get_macro_copy<service, MACRO_SERVICEACKAUTHOR>,
        true}},
      // Acknowledgement author name.
      {MACRO_SERVICEACKAUTHORNAME,
       {&get_macro_copy<service, MACRO_SERVICEACKAUTHORNAME>,
        true}},
      // Acknowledgement author alias.
      {MACRO_SERVICEACKAUTHORALIAS,
       {&get_macro_copy<service, MACRO_SERVICEACKAUTHORALIAS>,
        true}},
      // Acknowledgement comment.
      {MACRO_SERVICEACKCOMMENT,
       {&get_macro_copy<service, MACRO_SERVICEACKCOMMENT>,
        true}},
      // Service id.
      {MACRO_SERVICEID, {&get_service_id, true}},
      // Acknowledgement comment.
      {MACRO_SERVICEACKCOMMENT,
       {&get_macro_copy<service, MACRO_SERVICEACKCOMMENT>,
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
      std::string& output,
      int* free_macro) {
  // Check that function was called with valid arguments.
  int retval;
  if (svc &&  free_macro) {
    grab_service_redirection::entry::const_iterator it(
      redirector.routines.find(macro_type));
    // Found matching routine.
    if (it != redirector.routines.end()) {
      // Call routine.
      output = (*it->second.first)(*svc, mac);

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

  return retval;
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
  mac->servicegroup_ptr = nullptr;

  if (svc == nullptr)
    return ERROR;

  // Save first/primary servicegroup pointer for later.
  if (!svc->get_parent_groups().empty())
    mac->servicegroup_ptr
      = svc->get_parent_groups().front();

  return OK;
}

}
