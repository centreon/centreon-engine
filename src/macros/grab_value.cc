/*
** Copyright 1999-2010 Ethan Galstad
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

#include <cstdlib>
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros/grab_value.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/unordered_hash.hh"

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
 *  @param[out] mac         Macro object.
 *  @param[in]  macro_type  Macro to get.
 *  @param[in]  host_name   Host name.
 *  @param[in]  unused      Unused.
 *  @param[out] output      Output buffer.
 *  @param[out] free_macro  Set to true if output buffer must be freed
 *                          by caller.
 *
 *  @return OK on success.
 */
static int handle_host_macro(
             nagios_macros* mac,
             int macro_type,
             char const* host_name,
             char const* unused,
             char** output,
             int* free_macro) {
  (void)unused;

  // Find the host for on-demand macros or use saved host pointer.
  int retval;
  host* hst(host_name ? find_host(host_name) : mac->host_ptr);
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
  return (retval);
}

/**
 *  Get service macro.
 *
 *  @param[out] mac                  Macro object.
 *  @param[in]  macro_type           Macro to get.
 *  @param[in]  host_name            Host name.
 *  @param[in]  service_description  Service description.
 *  @param[out] output               Output buffer.
 *  @param[out] free_macro           Set to true if macro must be freed
 *                                   by caller.
 *
 *  @return OK on success.
 */
static int handle_service_macro(
             nagios_macros* mac,
             int macro_type,
             char const* host_name,
             char const* service_description,
             char** output,
             int* free_macro) {
  // Return value.
  int retval;

  // Use saved service pointer.
  if (!host_name && !service_description) {
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
  // Else ondemand macro...
  else {
    // If host name is blank, it means use the current host name.
    if (!host_name || (host_name[0] == '\0')) {
      if (!mac->host_ptr || !mac->host_ptr->name)
        retval = ERROR;
      else if (service_description) {
        service* svc(find_service(
                       mac->host_ptr->name,
                       service_description));
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
    else if (host_name && service_description) {
      // On-demand macro with both host and service name.
      service* svc(find_service(host_name, service_description));
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
  return (retval);
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
      bool problem(true);
      if ((temp_host->current_state == HOST_UP)
          && (temp_host->has_been_checked == true))
        hosts_up++;
      else if (temp_host->current_state == HOST_DOWN) {
        if (temp_host->checks_enabled == false)
          problem = false;
        if (problem)
          hosts_down_unhandled++;
        hosts_down++;
      }
      else if (temp_host->current_state == HOST_UNREACHABLE) {
        if (temp_host->checks_enabled == false)
          problem = false;
        if (problem)
          hosts_down_unhandled++;
        hosts_unreachable++;
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
        if (temp_service->checks_enabled == false)
          problem = false;
        if (problem)
          services_critical_unhandled++;
        services_critical++;
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
  typedef umap<unsigned int, int (*)(nagios_macros*, int, char const*, char const*, char**, int*)> entry;
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
      MACRO_HOSTSTATETYPE,
      MACRO_HOSTPERCENTCHANGE,
      MACRO_LASTHOSTUP,
      MACRO_LASTHOSTDOWN,
      MACRO_LASTHOSTUNREACHABLE,
      MACRO_HOSTCHECKCOMMAND,
      MACRO_HOSTACTIONURL,
      MACRO_HOSTNOTESURL,
      MACRO_HOSTNOTES,
      MACRO_HOSTCHECKTYPE,
      MACRO_LONGHOSTOUTPUT,
      MACRO_HOSTEVENTID,
      MACRO_LASTHOSTEVENTID,
      MACRO_MAXHOSTATTEMPTS,
      MACRO_TOTALHOSTSERVICES,
      MACRO_TOTALHOSTSERVICESOK,
      MACRO_TOTALHOSTSERVICESWARNING,
      MACRO_TOTALHOSTSERVICESUNKNOWN,
      MACRO_TOTALHOSTSERVICESCRITICAL,
      MACRO_HOSTPROBLEMID,
      MACRO_LASTHOSTPROBLEMID,
      MACRO_LASTHOSTSTATE,
      MACRO_LASTHOSTSTATEID
    };
    for (unsigned int i = 0;
         i < sizeof(host_ids) / sizeof(*host_ids);
         ++i)
      routines[host_ids[i]] = &handle_host_macro;

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
      MACRO_SERVICESTATETYPE,
      MACRO_SERVICEPERCENTCHANGE,
      MACRO_LASTSERVICEOK,
      MACRO_LASTSERVICEWARNING,
      MACRO_LASTSERVICEUNKNOWN,
      MACRO_LASTSERVICECRITICAL,
      MACRO_SERVICECHECKCOMMAND,
      MACRO_SERVICEACTIONURL,
      MACRO_SERVICENOTESURL,
      MACRO_SERVICENOTES,
      MACRO_SERVICECHECKTYPE,
      MACRO_LONGSERVICEOUTPUT,
      MACRO_SERVICEEVENTID,
      MACRO_LASTSERVICEEVENTID,
      MACRO_MAXSERVICEATTEMPTS,
      MACRO_SERVICEISVOLATILE,
      MACRO_SERVICEPROBLEMID,
      MACRO_LASTSERVICEPROBLEMID,
      MACRO_LASTSERVICESTATE,
      MACRO_LASTSERVICESTATEID
    };
    for (unsigned int i = 0;
         i < sizeof(service_ids) / sizeof(*service_ids);
         ++i)
      routines[service_ids[i]] = &handle_service_macro;

    // Date/Time macros.
    static unsigned int const datetime_ids[] = {
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
      MACRO_MAINCONFIGFILE,
      MACRO_STATUSDATAFILE,
      MACRO_RETENTIONDATAFILE,
      MACRO_LOGFILE,
      MACRO_RESOURCEFILE,
      MACRO_COMMANDFILE,
      MACRO_PROCESSSTARTTIME,
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

    /* save the first argument - host name, etc. */
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
      /* host/service output/perfdata and author macros should get cleaned */
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
