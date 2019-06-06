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
#include "com/centreon/engine/macros/clear_host.hh"
#include "com/centreon/engine/macros/clear_hostgroup.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/macros/grab.hh"
#include "com/centreon/engine/macros/grab_host.hh"
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
 *  Generate total services macros.
 *
 *  @param[in]  hst    Host object.
 *  @param[out] mac    Macro array.
 */
static void generate_host_total_services(
              host& hst,
              nagios_macros* mac) {
  // Generate host service summary macros
  // (if they haven't already been computed).
  if (!mac->x[MACRO_TOTALHOSTSERVICES]) {
    unsigned long total_host_services(0);
    unsigned long total_host_services_ok(0);
    unsigned long total_host_services_warning(0);
    unsigned long total_host_services_unknown(0);
    unsigned long total_host_services_critical(0);
    for (service_map::iterator
           it(hst.services.begin()),
           end(hst.services.end());
         it != end;
         ++it) {
      service* temp_service(it->second.get());
      if (temp_service) {
        total_host_services++;
        switch (temp_service->current_state) {
         case STATE_OK:
          total_host_services_ok++;
          break;
         case STATE_WARNING:
          total_host_services_warning++;
          break;
         case STATE_UNKNOWN:
          total_host_services_unknown++;
          break;
         case STATE_CRITICAL:
          total_host_services_critical++;
          break;
        }
      }
    }

    // These macros are time-intensive to compute, and will likely be
    // used together, so save them all for future use.
    string::setstr(mac->x[MACRO_TOTALHOSTSERVICES], total_host_services);
    string::setstr(mac->x[MACRO_TOTALHOSTSERVICESOK], total_host_services_ok);
    string::setstr(mac->x[MACRO_TOTALHOSTSERVICESWARNING], total_host_services_warning);
    string::setstr(mac->x[MACRO_TOTALHOSTSERVICESUNKNOWN], total_host_services_unknown);
    string::setstr(mac->x[MACRO_TOTALHOSTSERVICESCRITICAL], total_host_services_critical);
  }
  return;
}

/**
 *  Extract host check type.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Unused.
 *
 *  @return Newly allocated string containing either "PASSIVE" or
 *          "ACTIVE".
 */
static char* get_host_check_type(host& hst, nagios_macros* mac) {
  (void)mac;
  return (string::dup(
           (check_passive == hst.get_check_type()
            ? "PASSIVE"
            : "ACTIVE")));
}

/**
 *  Extract host group names.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Unused.
 *
 *  @return Newly allocated string with group names.
 */
static char* get_host_group_names(host& hst, nagios_macros* mac) {
  (void)mac;

  std::string buf;
  // Find all hostgroups this host is associated with.
  for (objectlist* temp_objectlist = hst.hostgroups_ptr;
       temp_objectlist != NULL;
       temp_objectlist = temp_objectlist->next) {
    hostgroup* temp_hostgroup(
      static_cast<hostgroup*>(temp_objectlist->object_ptr));
    if (temp_hostgroup) {
      if (!buf.empty())
        buf.append(",");
      buf.append(temp_hostgroup->get_group_name());
    }
  }
  return (string::dup(buf));
}

/**
 *  Extract host state.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Unused.
 *
 *  @return Newly allocated string with host state in plain text.
 */
template <typename T, typename V, int (V::* member)() const>
static char* get_host_state(T& t, nagios_macros* mac) {
  (void)mac;
  V* v{&t};
  int current{(v->*member)()};
  return string::dup(com::centreon::engine::host::tab_host_states[current].second);
}

/**
 *  Extract services statistics.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Macro array.
 *
 *  @return Newly allocated string with requested value in plain text.
 */
template <unsigned int macro_id>
static char* get_host_total_services(host& hst, nagios_macros* mac) {
  generate_host_total_services(hst, mac);
  return (mac->x[macro_id]);
}

/**
 *  Get the parents of a host.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Macro array.
 *
 *  @return Newly allocated string with requested value in plain text.
 */
static char* get_host_parents(host& hst, nagios_macros* mac) {
  (void)mac;
  std::string retval;
  for (host_map::iterator
         it(hst.parent_hosts.begin()),
         end(hst.parent_hosts.end());
       it != end;
       it++) {
    if (!retval.empty())
      retval.append(it->first);
    retval.append(",");
  }
  return (string::dup(retval.c_str()));
}

/**
 *  Get the children of a host.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Macro array.
 *
 *  @return Newly allocated string with requested value in plain text.
 */
static char* get_host_children(host& hst, nagios_macros* mac) {
  (void)mac;
  std::string retval;
  for (host_map::iterator
         it(hst.child_hosts.begin()),
         end(hst.child_hosts.end());
       it != end;
       it++) {
    if (!retval.empty())
      retval.append(it->first);
    retval.append(",");
  }
  return (string::dup(retval.c_str()));
}

/**
 *  Extract the host id.
 *
 *  @param[in] hst  The host
 *  @param[in] mac  Unused.
 *
 *  @return  Newly allocated string with the host id.
 */
static char* get_host_id(host& hst, nagios_macros* mac) {
  (void)mac;
  return (string::dup(string::from(
            com::centreon::engine::get_host_id(hst.get_name())).c_str()));
}

/**
 *  Get the timezone of a host.
 *
 *  @param[in] hst Host object.
 *  @param[in] mac Macro array.
 *
 *  @return Newly allocated string with requested value in plain text.
 */
static char* get_host_macro_timezone(host& hst, nagios_macros* mac) {
  (void)mac;
  return string::dup(hst.get_timezone());
}

/**************************************
*                                     *
*         Redirection Object          *
*                                     *
**************************************/

// Redirection object.
struct grab_host_redirection {
  typedef umap<unsigned int, std::pair<char* (*)(host&, nagios_macros*), bool>>
      entry;
  entry routines{
      {MACRO_HOSTNAME,
       {&get_member_as_string<host, std::string const&, &host::get_name>,
        true}},
      {MACRO_HOSTDISPLAYNAME,
       {&get_member_as_string<host,
                              std::string const&,
                              notifier,
                              &notifier::get_display_name>,
        true}},
      {MACRO_HOSTALIAS,
       {&get_member_as_string<host, std::string const&, &host::get_alias>,
        true}},
      {MACRO_HOSTADDRESS,
       {&get_member_as_string<host, std::string const&, &host::get_address>,
        true}},
      {MACRO_HOSTSTATE, {&get_host_state<host, notifier, &notifier::get_current_state>, true}},
      {MACRO_HOSTSTATEID,
       {&get_member_as_string<host, int, notifier, &notifier::get_current_state>, true}},
      {MACRO_LASTHOSTSTATE, {&get_host_state<host, host, &host::get_last_state>, true}},
      {MACRO_LASTHOSTSTATEID,
       {&get_member_as_string<host, int, &host::get_last_state>, true}},
      {MACRO_HOSTCHECKTYPE, {&get_host_check_type, true}},
      {MACRO_HOSTSTATETYPE, {&get_state_type<host>, true}},
      {MACRO_HOSTOUTPUT,
       {&get_member_as_string<host,
                              std::string const&, notifier,
                              &notifier::get_plugin_output>,
        true}},
      {MACRO_LONGHOSTOUTPUT,
       {&get_member_as_string<host,
                              std::string const&, notifier,
                              &notifier::get_long_plugin_output>,
        true}},
      {MACRO_HOSTPERFDATA,
       {&get_member_as_string<host, std::string const&, notifier, &notifier::get_perf_data>,
        true}},
      {MACRO_HOSTCHECKCOMMAND,
       {&get_member_as_string<host,
                              std::string const&,
                              notifier,
                              &notifier::get_check_command>,
        true}},
      {MACRO_HOSTATTEMPT,
       {&get_member_as_string<host,
                              int,
                              notifier,
                              &notifier::get_current_attempt>,
        true}},
      {MACRO_MAXHOSTATTEMPTS,
       {&get_member_as_string<host, int, notifier, &notifier::get_max_attempts>, true}},
      {MACRO_HOSTDOWNTIME,
       {&get_member_as_string<host, int, &host::get_scheduled_downtime_depth>,
        true}},
      {MACRO_HOSTPERCENTCHANGE,
       {&get_double<host, &host::get_percent_state_change, 2>, true}},
      {MACRO_HOSTDURATION, {&get_duration<host>, true}},
      {MACRO_HOSTDURATIONSEC, {&get_duration_sec<host>, true}},
      {MACRO_HOSTEXECUTIONTIME,
       {&get_double<host, &host::get_execution_time, 3>, true}},
      {MACRO_HOSTLATENCY, {&get_double<host, &host::get_latency, 3>, true}},
      {MACRO_LASTHOSTCHECK,
       {&get_member_as_string<host, time_t, &host::get_last_check>, true}},
      {MACRO_LASTHOSTSTATECHANGE,
       {&get_member_as_string<host, time_t, notifier, &notifier::get_last_state_change>,
        true}},
      {MACRO_LASTHOSTUP,
       {&get_member_as_string<host, time_t, &host::get_last_time_up>, true}},
      {MACRO_LASTHOSTDOWN,
       {&get_member_as_string<host, time_t, &host::get_last_time_down>, true}},
      {MACRO_LASTHOSTUNREACHABLE,
       {&get_member_as_string<host, time_t, &host::get_last_time_unreachable>,
        true}},
      {MACRO_HOSTNOTIFICATIONNUMBER,
       {&get_member_as_string<host,
                              int,
                              &host::get_current_notification_number>,
        true}},
      {MACRO_HOSTNOTIFICATIONID,
       {&get_member_as_string<host,
                              uint64_t, notifier,
                              &notifier::get_current_notification_id>,
        true}},
      {MACRO_HOSTEVENTID,
       {&get_member_as_string<host, unsigned long, notifier, &notifier::get_current_event_id>,
        true}},
      {MACRO_LASTHOSTEVENTID,
       {&get_member_as_string<host, unsigned long, notifier, &notifier::get_last_event_id>,
        true}},
      {MACRO_HOSTPROBLEMID,
       {&get_member_as_string<host,
                              unsigned long,
                              notifier,
                              &notifier::get_current_problem_id>,
        true}},
      {MACRO_LASTHOSTPROBLEMID,
       {&get_member_as_string<host, unsigned long, notifier, &notifier::get_last_problem_id>,
        true}},
      {MACRO_HOSTACTIONURL,
       {&get_recursive<host, notifier, &notifier::get_action_url, URL_ENCODE_MACRO_CHARS>,
        true}},
      {MACRO_HOSTNOTESURL,
       {&get_recursive<host, notifier, &notifier::get_notes_url, URL_ENCODE_MACRO_CHARS>,
        true}},
      {MACRO_HOSTNOTES, {&get_recursive<host, notifier, &notifier::get_notes, 0>, true}},
      {MACRO_HOSTGROUPNAMES, {&get_host_group_names, true}},
      {MACRO_TOTALHOSTSERVICES,
       {&get_host_total_services<MACRO_TOTALHOSTSERVICES>, true}},
      {MACRO_TOTALHOSTSERVICESOK,
       {&get_host_total_services<MACRO_TOTALHOSTSERVICESOK>, true}},
      {MACRO_TOTALHOSTSERVICESWARNING,
       {&get_host_total_services<MACRO_TOTALHOSTSERVICESWARNING>, true}},
      {MACRO_TOTALHOSTSERVICESUNKNOWN,
       {&get_host_total_services<MACRO_TOTALHOSTSERVICESUNKNOWN>, true}},
      {MACRO_TOTALHOSTSERVICESCRITICAL,
       {&get_host_total_services<MACRO_TOTALHOSTSERVICESCRITICAL>, true}},
      {MACRO_HOSTACKAUTHOR, {&get_macro_copy<host, MACRO_HOSTACKAUTHOR>, true}},
      {MACRO_HOSTACKAUTHORNAME,
       {&get_macro_copy<host, MACRO_HOSTACKAUTHORNAME>, true}},
      {MACRO_HOSTACKAUTHORALIAS,
       {&get_macro_copy<host, MACRO_HOSTACKAUTHORALIAS>, true}},
      {MACRO_HOSTACKCOMMENT,
       {&get_macro_copy<host, MACRO_HOSTACKCOMMENT>, true}},
      {MACRO_HOSTPARENTS, {&get_host_parents, true}},
      {MACRO_HOSTCHILDREN, {&get_host_children, true}},
      {MACRO_HOSTID, {&get_host_id, true}},
      {MACRO_HOSTTIMEZONE, {&get_host_macro_timezone, true}}};
} static const redirector;

/**************************************
*                                     *
*           Global Functions          *
*                                     *
**************************************/

extern "C" {
/**
 *  Grab a standard host macro.
 *
 *  @param[out] mac        Macro array.
 *  @param[in]  macro_type Macro to dump.
 *  @param[in]  hst        Target host.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer should be free
 *                         by caller.
 *
 *  @return OK on success.
 */
int grab_standard_host_macro_r(
      nagios_macros* mac,
      int macro_type,
      host* hst,
      char** output,
      int* free_macro) {
  // Check that function was called with valid arguments.
  int retval;
  if (hst && output && free_macro) {
    grab_host_redirection::entry::const_iterator it(
      redirector.routines.find(macro_type));
    // Found matching routine.
    if (it != redirector.routines.end()) {
      // Call routine.
      *output = (*it->second.first)(*hst, mac);

      // Set the free macro flag.
      *free_macro = it->second.second;

      // Successful execution.
      retval = OK;
    }
    // Non-existent macro.
    else {
      logger(dbg_macros, basic)
        << "UNHANDLED HOST MACRO #" << macro_type << "! THIS IS A BUG!";
      retval = ERROR;
    }
  }
  else
    retval = ERROR;

  return (retval);
}

/**
 *  Grab a standard host macro for global macros.
 *
 *  @param[in]  macro_type Macro to dump.
 *  @param[in]  hst        Target host.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer should be free
 *                         by caller.
 *
 *  @return OK on success.
 *
 *  @see grab_standard_host_macro_r
 */
int grab_standard_host_macro(
      int macro_type,
      host* hst,
      char** output,
      int* free_macro) {
  return (grab_standard_host_macro_r(
            get_global_macros(),
            macro_type,
            hst,
            output,
            free_macro));
}

/**
 *  Grab macros that are specific to a host.
 *
 *  @param[in] mac Macros object.
 *  @param[in] hst Host pointer.
 *
 *  @return OK on success.
 */
int grab_host_macros_r(nagios_macros* mac, host* hst) {
  // Clear host-related macros.
  clear_host_macros_r(mac);
  clear_hostgroup_macros_r(mac);

  // Save pointer to host.
  mac->host_ptr = hst;
  mac->hostgroup_ptr = NULL;

  if (hst == NULL)
    return (ERROR);

  // Save pointer to host's first/primary hostgroup.
  if (hst->hostgroups_ptr)
    mac->hostgroup_ptr
      = static_cast<hostgroup*>(hst->hostgroups_ptr->object_ptr);

  return (OK);
}

/**
 *  Grab macros that are specific to a host.
 *
 *  @param[in] hst Host pointer.
 *
 *  @return OK on success.
 *
 *  @see grab_host_macros_r
 */
int grab_host_macros(host* hst) {
  return (grab_host_macros_r(get_global_macros(), hst));
}

}
