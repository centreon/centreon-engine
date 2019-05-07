/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2019 Centreon
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

#include <cstdlib>
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros/grab_value.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/configuration/applier/state.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/**************************************
*                                     *
*           Local Functions           *
*                                     *
**************************************/

/**
 *  Get host macro.
 *
 *  @param[out] mac        Macro object.
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer must be freed by
 *                         caller.
 *
 *  @return OK on success.
 */
static int handle_host_macro(
             nagios_macros* mac,
             int macro_type,
             char const* arg1,
             char const* arg2,
             char** output,
             int* free_macro) {
  int retval;
  if (arg2 == NULL) {
    // Find the host for on-demand macros
    // or use saved host pointer.
    host* hst(arg1 ? find_host(arg1) : mac->host_ptr);
    if (hst)
      // Get the host macro value.
      retval = grab_standard_host_macro_r(
                 mac,
                 macro_type,
                 hst,
                 output,
                 free_macro);
    else
      retval = ERROR;
  }
  // A host macro with a hostgroup name and delimiter.
  else {
    hostgroup* hg(find_hostgroup(arg1));
    if (hg) {
      size_t delimiter_len(strlen(arg2));

      // Concatenate macro values for all hostgroup members.
      for (hostsmember* temp_hostsmember = hg->members;
           temp_hostsmember != NULL;
           temp_hostsmember = temp_hostsmember->next) {
        host* hst(temp_hostsmember->host_ptr);
        if (hst) {
          // Get the macro value for this host.
          char* buffer(NULL);
          int free_sub_macro(false);
          grab_standard_host_macro_r(
            mac,
            macro_type,
            hst,
            &buffer,
            &free_sub_macro);
          if (buffer) {
            // Add macro value to already running macro.
            if (*output == NULL)
              *output = string::dup(buffer);
            else {
              *output
                = resize_string(
                    *output,
                    strlen(*output) + strlen(buffer) + delimiter_len + 1);
              strcat(*output, arg2);
              strcat(*output, buffer);
            }
            if (free_sub_macro == true)
              delete[] buffer;
            *free_macro = true;
          }
        }
      }
      retval = OK;
    }
    else
      retval = ERROR;
  }
  return (retval);
}

/**
 *  Get hostgroup macro.
 *
 *  @param[out] mac        Macro object.
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer must be freed by
 *                         caller.
 *
 *  @return OK on success.
 */
static int handle_hostgroup_macro(
             nagios_macros* mac,
             int macro_type,
             char const* arg1,
             char const* arg2,
             char** output,
             int* free_macro) {
  (void)arg2;

  // Return value.
  int retval;

  // Use the saved hostgroup pointer
  // or find the hostgroup for on-demand macros.
  hostgroup* hg(arg1 ? find_hostgroup(arg1) : mac->hostgroup_ptr);
  if (hg) {
    // Get the hostgroup macro value.
    retval = grab_standard_hostgroup_macro_r(
               mac,
               macro_type,
               hg,
               output);
    if (OK == retval)
      *free_macro = true;
  }
  else
    retval = ERROR;
  return (retval);
}

/**
 *  Get service macro.
 *
 *  @param[out] mac        Macro object.
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if macro must be freed by caller.
 *
 *  @return OK on success.
 */
static int handle_service_macro(
             nagios_macros* mac,
             int macro_type,
             char const* arg1,
             char const* arg2,
             char** output,
             int* free_macro) {
  // Return value.
  int retval;

  // Use saved service pointer.
  if (!arg1 && !arg2) {
    if (!mac->service_ptr)
      retval = ERROR;
    else
      retval = grab_standard_service_macro_r(
                 mac,
                 macro_type,
                 mac->service_ptr,
                 output,
                 free_macro);
  }
  // Else and ondemand macro...
  else {
    // If first arg is blank, it means use the current host name.
    if (!arg1 || (arg1[0] == '\0')) {
      if (!mac->host_ptr)
        retval = ERROR;
      else if (arg2) {
        service* svc(find_service(mac->host_ptr->name, arg2));
        if (!svc)
          retval = ERROR;
        else
          // Get the service macro value.
          retval = grab_standard_service_macro_r(
                     mac,
                     macro_type,
                     svc,
                     output,
                     free_macro);
      }
      else
        retval = ERROR;
    }
    else if (arg1 && arg2) {
      // On-demand macro with both host and service name.
      service* svc(find_service(arg1, arg2));
      if (svc)
        // Get the service macro value.
        retval = grab_standard_service_macro_r(
                   mac,
                   macro_type,
                   svc,
                   output,
                   free_macro);
      // Else we have a service macro with a
      // servicegroup name and a delimiter...
      else {
        servicegroup* sg(find_servicegroup(arg1));
        if (!sg)
          retval = ERROR;
        else {
          size_t delimiter_len(strlen(arg2));

          // Concatenate macro values for all servicegroup members.
          for (servicesmember* temp_servicesmember = sg->members;
               temp_servicesmember != NULL;
               temp_servicesmember = temp_servicesmember->next) {
            svc = temp_servicesmember->service_ptr;
            if (svc) {
              // Get the macro value for this service.
              char* buffer(NULL);
              int free_sub_macro(false);
              grab_standard_service_macro_r(
                mac,
                macro_type,
                svc,
                &buffer,
                &free_sub_macro);
              if (buffer) {
                // Add macro value to already running macro.
                if (*output == NULL)
                  *output = string::dup(buffer);
                else {
                  *output = resize_string(
                              *output,
                              strlen(*output)
                              + strlen(buffer)
                              + delimiter_len
                              + 1);
                  strcat(*output, arg2);
                  strcat(*output, buffer);
                }
                if (free_sub_macro != false) {
                  delete[] buffer;
                  buffer = NULL;
                }
              }
            }
          }
          *free_macro = true;
          retval = OK;
        }
      }
    }
    else
      retval = ERROR;
  }
  return (retval);
}

/**
 *  Get servicegroup macro.
 *
 *  @param[out] mac        Macro object.
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer must be freed by
 *                         caller.
 *
 *  @return OK on success.
 */
static int handle_servicegroup_macro(
             nagios_macros* mac,
             int macro_type,
             char const* arg1,
             char const* arg2,
             char** output,
             int* free_macro) {
  (void)arg2;

  // Return value.
  int retval;

  // Use the saved servicegroup pointer
  // or find the servicegroup for on-demand macros.
  servicegroup* sg(arg1 ? find_servicegroup(arg1) : mac->servicegroup_ptr);
  if (!sg)
    retval = ERROR;
  else {
    // Get the servicegroup macro value.
    retval = grab_standard_servicegroup_macro_r(
               mac,
               macro_type,
               sg,
               output);
    if (OK == retval)
      *free_macro = true;
  }

  return (retval);
}

/**
 *  Get contact macro.
 *
 *  @param[out] mac        Macro object.
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer must be freed by
 *                         caller.
 *
 *  @return OK on success.
 */
static int handle_contact_macro(
             nagios_macros* mac,
             int macro_type,
             char const* arg1,
             char const* arg2,
             char** output,
             int* free_macro) {
  // Return value.
  int retval;

  if (arg2 == NULL) {
    // Find the contact for on-demand macros
    // or use saved contact pointer.
    contact* cntct(arg1 ? configuration::applier::state::instance().find_contact(arg1) : mac->contact_ptr);
    if (!cntct)
      retval = ERROR;
    else {
      // Get the contact macro value.
      retval = grab_standard_contact_macro_r(
                 mac,
                 macro_type,
                 cntct,
                 output);
      if (OK == retval)
        *free_macro = true;
    }
  }
  // A contact macro with a contactgroup name and delimiter.
  else if (arg1 && arg2) {
    contactgroup* cg(configuration::applier::state::instance().find_contactgroup(arg1));
    if (!cg)
      retval = ERROR;
    else {
      size_t delimiter_len(strlen(arg2));

      // Concatenate macro values for all contactgroup members.
      for(std::unordered_map<std::string, contact *>::const_iterator
            it(cg->get_members().begin()),
            end(cg->get_members().end());
            it != end;
            ++it) {
        contact* cntct(it->second);
        if (cntct) {
          // Get the macro value for this contact.
          char* buffer(NULL);
          grab_standard_contact_macro_r(
            mac,
            macro_type,
            cntct,
            &buffer);
          if (buffer) {
            // Add macro value to already running macro.
            if (*output == NULL)
              *output = string::dup(buffer);
            else {
              *output = resize_string(
                          *output,
                          strlen(*output)
                          + strlen(buffer)
                          + delimiter_len
                          + 1);
              strcat(*output, arg2);
              strcat(*output, buffer);
            }
            delete[] buffer;
            buffer = NULL;
          }
        }
      }
      *free_macro = true;
      retval = OK;
    }
  }
  else
    retval = ERROR;
  return (retval);
}

/**
 *  Get contactgroup macro.
 *
 *  @param[out] mac        Macro object.
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer must be freed by
 *                         caller.
 *
 *  @return OK on success.
 */
static int handle_contactgroup_macro(
             nagios_macros* mac,
             int macro_type,
             char const* arg1,
             char const* arg2,
             char** output,
             int* free_macro) {
  (void)arg2;

  // Return value.
  int retval;

  // Use the saved contactgroup pointer.
  // or find the contactgroup for on-demand macros.
  contactgroup* cg(arg1 ? configuration::applier::state::instance().find_contactgroup(arg1) : mac->contactgroup_ptr);
  if (!cg)
    retval = ERROR;
  else {
    // Get the contactgroup macro value.
    retval = grab_standard_contactgroup_macro(macro_type, cg, output);
    if (OK == retval)
      *free_macro = true;
  }

  return (retval);
}

/**
 *  Get a notification macro.
 *
 *  @param[out] mac        Macro object.
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to false.
 *
 *  @return OK on success.
 */
static int handle_notification_macro(
             nagios_macros* mac,
             int macro_type,
             char const* arg1,
             char const* arg2,
             char** output,
             int* free_macro) {
  (void)arg1;
  (void)arg2;

  // Notification macros have already been pre-computed.
  *output = mac->x[macro_type];
  *free_macro = false;

  return (OK);
}

/**
 *  Get a date/time macro.
 *
 *  @param[out] mac        Macro object.
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer must be freed by
 *                         caller.
 *
 *  @return OK on success.
 */
static int handle_datetime_macro(
             nagios_macros* mac,
             int macro_type,
             char const* arg1,
             char const* arg2,
             char** output,
             int* free_macro) {
  // Calculate macros.
  int retval(grab_datetime_macro_r(
               mac,
               macro_type,
               arg1,
               arg2,
               output));
  if (OK == retval)
    *free_macro = true;
  return (retval);
}

/**
 *  Get a static macro.
 *
 *  @param[out] mac        Macro object.
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to false.
 *
 *  @return OK on success.
 */
static int handle_static_macro(
             nagios_macros* mac,
             int macro_type,
             char const* arg1,
             char const* arg2,
             char** output,
             int* free_macro) {
  (void)mac;
  (void)arg1;
  (void)arg2;

  // No need to do any more work - these are already precomputed for us.
  *output = get_global_macros()->x[macro_type];
  *free_macro = false;
  return (OK);
}

/**
 *  Get a summary macro.
 *
 *  @param[out] mac        Macro object.
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer must be freed by
 *                         caller.
 *
 *  @return OK on success.
 */
static int handle_summary_macro(
             nagios_macros* mac,
             int macro_type,
             char const* arg1,
             char const* arg2,
             char** output,
             int* free_macro) {
  (void)arg1;
  (void)arg2;

  // Generate summary macros if needed.
  if (!mac->x[MACRO_TOTALHOSTSUP]) {
    // Get host totals.
    unsigned int host_problems(0);
    unsigned int host_problems_unhandled(0);
    unsigned int hosts_down(0);
    unsigned int hosts_down_unhandled(0);
    unsigned int hosts_unreachable(0);
    unsigned int hosts_unreachable_unhandled(0);
    unsigned int hosts_up(0);
    for (host* temp_host = host_list;
         temp_host != NULL;
         temp_host = temp_host->next) {
      // Filter totals based on contact if necessary.
      bool authorized(
             mac->contact_ptr
             ? is_contact_for_host(temp_host, mac->contact_ptr)
             : true);
      if (authorized) {
        bool problem(true);
        if ((temp_host->current_state == HOST_UP)
            && (temp_host->has_been_checked == true))
          hosts_up++;
        else if (temp_host->current_state == HOST_DOWN) {
          if (temp_host->scheduled_downtime_depth > 0)
            problem = false;
          if (temp_host->problem_has_been_acknowledged == true)
            problem = false;
          if (temp_host->checks_enabled == false)
            problem = false;
          if (problem)
            hosts_down_unhandled++;
          hosts_down++;
        }
        else if (temp_host->current_state == HOST_UNREACHABLE) {
          if (temp_host->scheduled_downtime_depth > 0)
            problem = false;
          if (temp_host->problem_has_been_acknowledged == true)
            problem = false;
          if (temp_host->checks_enabled == false)
            problem = false;
          if (problem)
            hosts_down_unhandled++;
          hosts_unreachable++;
        }
      }
    }
    host_problems = hosts_down + hosts_unreachable;
    host_problems_unhandled
      = hosts_down_unhandled + hosts_unreachable_unhandled;

    // Get service totals.
    unsigned int service_problems(0);
    unsigned int service_problems_unhandled(0);
    unsigned int services_critical(0);
    unsigned int services_critical_unhandled(0);
    unsigned int services_ok(0);
    unsigned int services_unknown(0);
    unsigned int services_unknown_unhandled(0);
    unsigned int services_warning(0);
    unsigned int services_warning_unhandled(0);
    for (service* temp_service = service_list;
         temp_service != NULL;
         temp_service = temp_service->next) {
      // Filter totals based on contact if necessary.
      bool authorized(
             mac->contact_ptr
             ? is_contact_for_service(
                 temp_service,
                 mac->contact_ptr)
             : true);
      if (authorized) {
        bool problem(true);
        if (temp_service->current_state == STATE_OK
            && temp_service->has_been_checked == true)
          services_ok++;
        else if (temp_service->current_state == STATE_WARNING) {
          host* temp_host(find_host(temp_service->host_name));
          if (temp_host != NULL
              && (temp_host->current_state == HOST_DOWN
                  || temp_host->current_state == HOST_UNREACHABLE))
            problem = false;
          if (temp_service->scheduled_downtime_depth > 0)
            problem = false;
          if (temp_service->problem_has_been_acknowledged == true)
            problem = false;
          if (temp_service->checks_enabled == false)
            problem = false;
          if (problem)
            services_warning_unhandled++;
          services_warning++;
        }
        else if (temp_service->current_state == STATE_UNKNOWN) {
          host* temp_host(find_host(temp_service->host_name));
          if (temp_host != NULL
              && (temp_host->current_state == HOST_DOWN
                  || temp_host->current_state == HOST_UNREACHABLE))
            problem = false;
          if (temp_service->scheduled_downtime_depth > 0)
            problem = false;
          if (temp_service->problem_has_been_acknowledged == true)
            problem = false;
          if (temp_service->checks_enabled == false)
            problem = false;
          if (problem)
            services_unknown_unhandled++;
          services_unknown++;
        }
        else if (temp_service->current_state == STATE_CRITICAL) {
          host* temp_host(find_host(temp_service->host_name));
          if (temp_host != NULL
              && (temp_host->current_state == HOST_DOWN
                  || temp_host->current_state == HOST_UNREACHABLE))
            problem = false;
          if (temp_service->scheduled_downtime_depth > 0)
            problem = false;
          if (temp_service->problem_has_been_acknowledged == true)
            problem = false;
          if (temp_service->checks_enabled == false)
            problem = false;
          if (problem)
            services_critical_unhandled++;
          services_critical++;
        }
      }
    }
    service_problems
      = services_warning + services_critical + services_unknown;
    service_problems_unhandled
      = services_warning_unhandled
      + services_critical_unhandled
      + services_unknown_unhandled;

    // These macros are time-intensive to compute, and will likely be
    // used together, so save them all for future use.
    string::setstr(mac->x[MACRO_TOTALHOSTSUP], hosts_up);
    string::setstr(mac->x[MACRO_TOTALHOSTSDOWN], hosts_down);
    string::setstr(mac->x[MACRO_TOTALHOSTSUNREACHABLE], hosts_unreachable);
    string::setstr(mac->x[MACRO_TOTALHOSTSDOWNUNHANDLED], hosts_down_unhandled);
    string::setstr(mac->x[MACRO_TOTALHOSTSUNREACHABLEUNHANDLED], hosts_unreachable_unhandled);
    string::setstr(mac->x[MACRO_TOTALHOSTPROBLEMS], host_problems);
    string::setstr(mac->x[MACRO_TOTALHOSTPROBLEMSUNHANDLED], host_problems_unhandled);
    string::setstr(mac->x[MACRO_TOTALSERVICESOK], services_ok);
    string::setstr(mac->x[MACRO_TOTALSERVICESWARNING], services_warning);
    string::setstr(mac->x[MACRO_TOTALSERVICESCRITICAL], services_critical);
    string::setstr(mac->x[MACRO_TOTALSERVICESUNKNOWN], services_unknown);
    string::setstr(mac->x[MACRO_TOTALSERVICESWARNINGUNHANDLED], services_warning_unhandled);
    string::setstr(mac->x[MACRO_TOTALSERVICESCRITICALUNHANDLED], services_critical_unhandled);
    string::setstr(mac->x[MACRO_TOTALSERVICESUNKNOWNUNHANDLED], services_unknown_unhandled);
    string::setstr(mac->x[MACRO_TOTALSERVICEPROBLEMS], service_problems);
    string::setstr(mac->x[MACRO_TOTALSERVICEPROBLEMSUNHANDLED], service_problems_unhandled);
  }

  // Return only the macro the user requested.
  *output = mac->x[macro_type];

  // Tell caller to NOT free memory when done.
  *free_macro = false;

  return (OK);
}

/**************************************
*                                     *
*         Redirection Object          *
*                                     *
**************************************/

// Redirection object.
struct grab_value_redirection {
  typedef std::unordered_map<unsigned int, int (*)(nagios_macros*, int, char const*, char const*, char**, int*)> entry;
  entry routines;
  grab_value_redirection() {
    // Host macros.
    static unsigned int const host_ids[] = {
      MACRO_HOSTNAME,
      MACRO_HOSTALIAS,
      MACRO_HOSTADDRESS,
      MACRO_LASTHOSTCHECK,
      MACRO_LASTHOSTSTATECHANGE,
      MACRO_HOSTOUTPUT,
      MACRO_HOSTPERFDATA,
      MACRO_HOSTSTATE,
      MACRO_HOSTSTATEID,
      MACRO_HOSTATTEMPT,
      MACRO_HOSTEXECUTIONTIME,
      MACRO_HOSTLATENCY,
      MACRO_HOSTDURATION,
      MACRO_HOSTDURATIONSEC,
      MACRO_HOSTDOWNTIME,
      MACRO_HOSTSTATETYPE,
      MACRO_HOSTPERCENTCHANGE,
      MACRO_HOSTACKAUTHOR,
      MACRO_HOSTACKCOMMENT,
      MACRO_LASTHOSTUP,
      MACRO_LASTHOSTDOWN,
      MACRO_LASTHOSTUNREACHABLE,
      MACRO_HOSTCHECKCOMMAND,
      MACRO_HOSTDISPLAYNAME,
      MACRO_HOSTACTIONURL,
      MACRO_HOSTNOTESURL,
      MACRO_HOSTNOTES,
      MACRO_HOSTCHECKTYPE,
      MACRO_LONGHOSTOUTPUT,
      MACRO_HOSTNOTIFICATIONNUMBER,
      MACRO_HOSTNOTIFICATIONID,
      MACRO_HOSTEVENTID,
      MACRO_LASTHOSTEVENTID,
      MACRO_HOSTGROUPNAMES,
      MACRO_HOSTACKAUTHORNAME,
      MACRO_HOSTACKAUTHORALIAS,
      MACRO_MAXHOSTATTEMPTS,
      MACRO_TOTALHOSTSERVICES,
      MACRO_TOTALHOSTSERVICESOK,
      MACRO_TOTALHOSTSERVICESWARNING,
      MACRO_TOTALHOSTSERVICESUNKNOWN,
      MACRO_TOTALHOSTSERVICESCRITICAL,
      MACRO_HOSTPROBLEMID,
      MACRO_LASTHOSTPROBLEMID,
      MACRO_LASTHOSTSTATE,
      MACRO_LASTHOSTSTATEID,
      MACRO_HOSTPARENTS,
      MACRO_HOSTCHILDREN,
      MACRO_HOSTID,
      MACRO_HOSTTIMEZONE
    };
    for (unsigned int i = 0;
         i < sizeof(host_ids) / sizeof(*host_ids);
         ++i)
      routines[host_ids[i]] = &handle_host_macro;

    // Hostgroup macros.
    static unsigned int const hostgroup_ids[] = {
      MACRO_HOSTGROUPNAME,
      MACRO_HOSTGROUPALIAS,
      MACRO_HOSTGROUPNOTES,
      MACRO_HOSTGROUPNOTESURL,
      MACRO_HOSTGROUPACTIONURL,
      MACRO_HOSTGROUPMEMBERS
    };
    for (unsigned int i = 0;
         i < sizeof(hostgroup_ids) / sizeof(*hostgroup_ids);
         ++i)
      routines[hostgroup_ids[i]] = &handle_hostgroup_macro;

    // Service macros.
    static unsigned int const service_ids[] = {
      MACRO_SERVICEDESC,
      MACRO_SERVICESTATE,
      MACRO_SERVICESTATEID,
      MACRO_SERVICEATTEMPT,
      MACRO_LASTSERVICECHECK,
      MACRO_LASTSERVICESTATECHANGE,
      MACRO_SERVICEOUTPUT,
      MACRO_SERVICEPERFDATA,
      MACRO_SERVICEEXECUTIONTIME,
      MACRO_SERVICELATENCY,
      MACRO_SERVICEDURATION,
      MACRO_SERVICEDURATIONSEC,
      MACRO_SERVICEDOWNTIME,
      MACRO_SERVICESTATETYPE,
      MACRO_SERVICEPERCENTCHANGE,
      MACRO_SERVICEACKAUTHOR,
      MACRO_SERVICEACKCOMMENT,
      MACRO_LASTSERVICEOK,
      MACRO_LASTSERVICEWARNING,
      MACRO_LASTSERVICEUNKNOWN,
      MACRO_LASTSERVICECRITICAL,
      MACRO_SERVICECHECKCOMMAND,
      MACRO_SERVICEDISPLAYNAME,
      MACRO_SERVICEACTIONURL,
      MACRO_SERVICENOTESURL,
      MACRO_SERVICENOTES,
      MACRO_SERVICECHECKTYPE,
      MACRO_LONGSERVICEOUTPUT,
      MACRO_SERVICENOTIFICATIONNUMBER,
      MACRO_SERVICENOTIFICATIONID,
      MACRO_SERVICEEVENTID,
      MACRO_LASTSERVICEEVENTID,
      MACRO_SERVICEGROUPNAMES,
      MACRO_SERVICEACKAUTHORNAME,
      MACRO_SERVICEACKAUTHORALIAS,
      MACRO_MAXSERVICEATTEMPTS,
      MACRO_SERVICEISVOLATILE,
      MACRO_SERVICEPROBLEMID,
      MACRO_LASTSERVICEPROBLEMID,
      MACRO_LASTSERVICESTATE,
      MACRO_LASTSERVICESTATEID,
      MACRO_SERVICEID,
      MACRO_SERVICETIMEZONE
    };
    for (unsigned int i = 0;
         i < sizeof(service_ids) / sizeof(*service_ids);
         ++i)
      routines[service_ids[i]] = &handle_service_macro;

    // Servicegroup macros.
    static unsigned int const servicegroup_ids[] = {
      MACRO_SERVICEGROUPNAME,
      MACRO_SERVICEGROUPALIAS,
      MACRO_SERVICEGROUPNOTES,
      MACRO_SERVICEGROUPNOTESURL,
      MACRO_SERVICEGROUPACTIONURL,
      MACRO_SERVICEGROUPMEMBERS
    };
    for (unsigned int i = 0;
         i < sizeof(servicegroup_ids) / sizeof(*servicegroup_ids);
         ++i)
      routines[servicegroup_ids[i]] = &handle_servicegroup_macro;

    // Contact macros.
    static unsigned int const contact_ids[] = {
      MACRO_CONTACTNAME,
      MACRO_CONTACTALIAS,
      MACRO_CONTACTEMAIL,
      MACRO_CONTACTPAGER,
      MACRO_CONTACTGROUPNAMES,
      MACRO_CONTACTTIMEZONE
    };
    for (unsigned int i = 0;
         i < sizeof(contact_ids) / sizeof(*contact_ids);
         ++i)
      routines[contact_ids[i]] = &handle_contact_macro;

    // Contactgroup macros.
    static unsigned int const contactgroup_ids[] = {
      MACRO_CONTACTGROUPNAME,
      MACRO_CONTACTGROUPALIAS,
      MACRO_CONTACTGROUPMEMBERS
    };
    for (unsigned int i = 0;
         i < sizeof(contactgroup_ids) / sizeof(*contactgroup_ids);
         ++i)
      routines[contactgroup_ids[i]] = &handle_contactgroup_macro;

    // Notification macros.
    static unsigned int const notification_ids[] = {
      MACRO_NOTIFICATIONTYPE,
      MACRO_NOTIFICATIONNUMBER,
      MACRO_NOTIFICATIONRECIPIENTS,
      MACRO_NOTIFICATIONISESCALATED,
      MACRO_NOTIFICATIONAUTHOR,
      MACRO_NOTIFICATIONAUTHORNAME,
      MACRO_NOTIFICATIONAUTHORALIAS,
      MACRO_NOTIFICATIONCOMMENT
    };
    for (unsigned int i = 0;
         i < sizeof(notification_ids) / sizeof(*notification_ids);
         ++i)
      routines[notification_ids[i]] = &handle_notification_macro;

    // Date/Time macros.
    static unsigned int const datetime_ids[] = {
      MACRO_LONGDATETIME,
      MACRO_SHORTDATETIME,
      MACRO_DATE,
      MACRO_TIME,
      MACRO_TIMET,
      MACRO_ISVALIDTIME,
      MACRO_NEXTVALIDTIME
    };
    for (unsigned int i = 0;
         i < sizeof(datetime_ids) / sizeof(*datetime_ids);
         ++i)
      routines[datetime_ids[i]] = &handle_datetime_macro;

    // Static macros.
    static unsigned int const static_ids[] = {
      MACRO_ADMINEMAIL,
      MACRO_ADMINPAGER,
      MACRO_MAINCONFIGFILE,
      MACRO_STATUSDATAFILE,
      MACRO_RETENTIONDATAFILE,
      MACRO_OBJECTCACHEFILE,
      MACRO_TEMPFILE,
      MACRO_LOGFILE,
      MACRO_RESOURCEFILE,
      MACRO_COMMANDFILE,
      MACRO_HOSTPERFDATAFILE,
      MACRO_SERVICEPERFDATAFILE,
      MACRO_PROCESSSTARTTIME,
      MACRO_TEMPPATH,
      MACRO_EVENTSTARTTIME
    };
    for (unsigned int i = 0;
         i < sizeof(static_ids) / sizeof(*static_ids);
         ++i)
      routines[static_ids[i]] = &handle_static_macro;

    // Summary macros.
    static unsigned int const summary_ids[] = {
      MACRO_TOTALHOSTSUP,
      MACRO_TOTALHOSTSDOWN,
      MACRO_TOTALHOSTSUNREACHABLE,
      MACRO_TOTALHOSTSDOWNUNHANDLED,
      MACRO_TOTALHOSTSUNREACHABLEUNHANDLED,
      MACRO_TOTALHOSTPROBLEMS,
      MACRO_TOTALHOSTPROBLEMSUNHANDLED,
      MACRO_TOTALSERVICESOK,
      MACRO_TOTALSERVICESWARNING,
      MACRO_TOTALSERVICESCRITICAL,
      MACRO_TOTALSERVICESUNKNOWN,
      MACRO_TOTALSERVICESWARNINGUNHANDLED,
      MACRO_TOTALSERVICESCRITICALUNHANDLED,
      MACRO_TOTALSERVICESUNKNOWNUNHANDLED,
      MACRO_TOTALSERVICEPROBLEMS,
      MACRO_TOTALSERVICEPROBLEMSUNHANDLED
    };
    for (unsigned int i = 0;
         i < sizeof(summary_ids) / sizeof(*summary_ids);
         ++i)
      routines[summary_ids[i]] = &handle_summary_macro;
  }
} static const redirector;

/**************************************
*                                     *
*          Global Functions           *
*                                     *
**************************************/

extern "C" {

/* this is the big one */
int grab_macro_value_r(
      nagios_macros* mac,
      char* macro_buffer,
      char** output,
      int* clean_options,
      int* free_macro) {
  char* buf = NULL;
  char* ptr = NULL;
  char* macro_name = NULL;
  char* arg[2] = { NULL, NULL };
  contact* temp_contact = NULL;
  contactgroup* temp_contactgroup = NULL;
  contactsmember* temp_contactsmember = NULL;
  char* temp_buffer = NULL;
  int delimiter_len = 0;
  unsigned int x;
  int result = OK;

  if (output == NULL)
    return (ERROR);

  /* clear the old macro value */
  delete[] *output;
  *output = NULL;

  if (macro_buffer == NULL || clean_options == NULL
      || free_macro == NULL)
    return (ERROR);

  /* work with a copy of the original buffer */
  buf = string::dup(macro_buffer);

  /* BY DEFAULT, TELL CALLER TO FREE MACRO BUFFER WHEN DONE */
  *free_macro = true;

  /* macro name is at start of buffer */
  macro_name = buf;

  /* see if there's an argument - if so, this is most likely an on-demand macro */
  if ((ptr = strchr(buf, ':'))) {
    ptr[0] = '\x0';
    ptr++;

    /* save the first argument - host name, hostgroup name, etc. */
    arg[0] = ptr;

    /* try and find a second argument */
    if ((ptr = strchr(ptr, ':'))) {
      ptr[0] = '\x0';
      ptr++;

      /* save second argument - service description or delimiter */
      arg[1] = ptr;
    }
  }

  /***** X MACROS *****/
  /* see if this is an x macro */
  for (x = 0; x < MACRO_X_COUNT; x++) {
    if (macro_x_names[x] == NULL)
      continue;

    if (!strcmp(macro_name, macro_x_names[x])) {
      logger(dbg_macros, most)
        << "  macros[" << x << "] (" << macro_x_names[x] << ") match.";

      /* get the macro value */
      result = grab_macrox_value_r(
                 mac,
                 x,
                 arg[0],
                 arg[1],
                 output,
                 free_macro);

      /* post-processing */
      /* host/service output/perfdata and author/comment macros should get cleaned */
      if ((x >= 16 && x <= 19) || (x >= 49 && x <= 52)
          || (x >= 99 && x <= 100) || (x >= 124 && x <= 127)) {
        *clean_options |= (STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);
        logger(dbg_macros, most)
          << "  New clean options: " << *clean_options;
      }
      /* url macros should get cleaned */
      if ((x >= 125 && x <= 126) || (x >= 128 && x <= 129)
          || (x >= 77 && x <= 78) || (x >= 74 && x <= 75)) {
        *clean_options |= URL_ENCODE_MACRO_CHARS;
        logger(dbg_macros, most)
          << "  New clean options: " << *clean_options;
      }

      break;
    }
  }

  /* we already found the macro... */
  if (x < MACRO_X_COUNT)
    ;
  /***** ARGV MACROS *****/
  else if (strstr(macro_name, "ARG") == macro_name) {
    /* which arg do we want? */
    x = atoi(macro_name + 3);

    if (!x || x > MAX_COMMAND_ARGUMENTS) {
      delete[] buf;
      return (ERROR);
    }

    /* use a pre-computed macro value */
    *output = mac->argv[x - 1];
    *free_macro = false;
  }
  /***** USER MACROS *****/
  else if (strstr(macro_name, "USER") == macro_name) {
    /* which macro do we want? */
    x = atoi(macro_name + 4);

    if (!x || x > MAX_USER_MACROS) {
      delete[] buf;
      return (ERROR);
    }

    /* use a pre-computed macro value */
    *output = macro_user[x - 1];
    *free_macro = false;
  }

  /***** CONTACT ADDRESS MACROS *****/
  /* NOTE: the code below should be broken out into a separate function */
  else if (strstr(macro_name, "CONTACTADDRESS") == macro_name) {
    /* which address do we want? */
    x = atoi(macro_name + 14) - 1;

    /* regular macro */
    if (arg[0] == NULL) {
      /* use the saved pointer */
      if ((temp_contact = mac->contact_ptr) == NULL) {
        delete[] buf;
        return (ERROR);
      }

      /* get the macro value */
      result = grab_contact_address_macro(x, temp_contact, output);
    }
    /* on-demand macro */
    else {
      /* on-demand contact macro with a contactgroup and a delimiter */
      if (arg[1] != NULL) {
        if ((temp_contactgroup = configuration::applier::state::instance().find_contactgroup(arg[0])) == NULL)
          return (ERROR);

        delimiter_len = strlen(arg[1]);

        /* concatenate macro values for all contactgroup members */
      for(std::unordered_map<std::string, contact *>::const_iterator
            it(temp_contactgroup->get_members().begin()),
            end(temp_contactgroup->get_members().end());
            it != end;
            ++it) {
          if ((temp_contact = it->second) == NULL)
            continue;
          if ((temp_contact = configuration::applier::state::instance().find_contact(temp_contactsmember->contact_name)) == NULL)
            continue;

          /* get the macro value for this contact */
          grab_contact_address_macro(x, temp_contact, &temp_buffer);

          if (temp_buffer == NULL)
            continue;

          /* add macro value to already running macro */
          if (*output == NULL)
            *output = string::dup(temp_buffer);
          else {
            *output = resize_string(
                        *output,
                        strlen(*output)
                        + strlen(temp_buffer)
                        + delimiter_len
                        + 1);
            strcat(*output, arg[1]);
            strcat(*output, temp_buffer);
          }
          delete[] temp_buffer;
          temp_buffer = NULL;
        }
      }
      /* else on-demand contact macro */
      else {
        /* find the contact */
        if ((temp_contact = configuration::applier::state::instance().find_contact(arg[0])) == NULL) {
          delete[] buf;
          return (ERROR);
        }

        /* get the macro value */
        result = grab_contact_address_macro(x, temp_contact, output);
      }
    }
  }
  /***** CUSTOM VARIABLE MACROS *****/
  else if (macro_name[0] == '_') {
    /* get the macro value */
    result = grab_custom_macro_value_r(
               mac,
               macro_name,
               arg[0],
               arg[1],
               output);
  }
  else if (
    configuration::applier::state::instance().user_macros().find(macro_name)
      != configuration::applier::state::instance().user_macros().end()) {
    /*** New style user macros ***/
    *output = string::dup(
      configuration::applier::state::instance().user_macros_find(macro_name)->second.c_str());
    result = true;
  }
  /* no macro matched... */
  else {
    logger(dbg_macros, basic)
      << " WARNING: Could not find a macro matching '"
      << macro_name << "'!";
    result = ERROR;
  }

  /* free memory */
  delete[] buf;
  return (result);
}

int grab_macro_value(
      char* macro_buffer,
      char** output,
      int* clean_options,
      int* free_macro) {
  return (grab_macro_value_r(
            get_global_macros(),
            macro_buffer,
            output,
            clean_options,
            free_macro));
}

/**
 *  Grab a macro value.
 *
 *  @param[out] mac        Macro object.
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer must be freed by
 *                         caller.
 *
 *  @return OK on success.
 */
int grab_macrox_value_r(
      nagios_macros* mac,
      int macro_type,
      char const* arg1,
      char const* arg2,
      char** output,
      int* free_macro) {
  int retval;
  if (!mac || !output || !free_macro)
    retval = ERROR;
  else {
    grab_value_redirection::entry::const_iterator it(
      redirector.routines.find(macro_type));
    if (redirector.routines.end() == it) {
      retval = ERROR;
      logger(dbg_macros, basic)
        << "UNHANDLED MACRO #" << macro_type << "! THIS IS A BUG!";
    }
    else
      retval = (*it->second)(
                 mac,
                 macro_type,
                 arg1,
                 arg2,
                 output,
                 free_macro);
  }
  return (retval);
}

/**
 *  Grab a global macro value.
 *
 *  @param[in]  macro_type Macro to get.
 *  @param[in]  arg1       Argument 1.
 *  @param[in]  arg2       Argument 2.
 *  @param[out] output     Output buffer.
 *  @param[out] free_macro Set to true if output buffer must be freed by
 *                         caller.
 *
 *  @return OK on success.
 *
 *  @see grab_macrox_value_r
 */
int grab_macrox_value(
      int macro_type,
      char const* arg1,
      char const* arg2,
      char** output,
      int* free_macro) {
  return (grab_macrox_value_r(
            get_global_macros(),
            macro_type,
            arg1,
            arg2,
            output,
            free_macro));
}

}
