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

#include "com/centreon/engine/macros/clear_host.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/macros/misc.hh"

extern "C" {

/**
 *  Clear host macros.
 *
 *  @param[in,out] mac Macro object.
 *
 *  @return OK on success.
 */
int clear_host_macros_r(nagios_macros* mac) {
  static unsigned int const to_free[] = {
    MACRO_HOSTNAME,
    MACRO_HOSTDISPLAYNAME,
    MACRO_HOSTALIAS,
    MACRO_HOSTADDRESS,
    MACRO_HOSTSTATE,
    MACRO_HOSTSTATEID,
    MACRO_HOSTCHECKTYPE,
    MACRO_HOSTSTATETYPE,
    MACRO_HOSTOUTPUT,
    MACRO_LONGHOSTOUTPUT,
    MACRO_HOSTPERFDATA,
    MACRO_HOSTCHECKCOMMAND,
    MACRO_HOSTATTEMPT,
    MACRO_MAXHOSTATTEMPTS,
    MACRO_HOSTPERCENTCHANGE,
    MACRO_HOSTDURATIONSEC,
    MACRO_HOSTDURATION,
    MACRO_HOSTEXECUTIONTIME,
    MACRO_HOSTLATENCY,
    MACRO_LASTHOSTCHECK,
    MACRO_LASTHOSTSTATECHANGE,
    MACRO_LASTHOSTUP,
    MACRO_LASTHOSTDOWN,
    MACRO_LASTHOSTUNREACHABLE,
    MACRO_HOSTEVENTID,
    MACRO_LASTHOSTEVENTID,
    MACRO_HOSTACTIONURL,
    MACRO_HOSTNOTESURL,
    MACRO_HOSTNOTES,
    MACRO_HOSTGROUPNAMES,
    MACRO_TOTALHOSTSERVICES,
    MACRO_TOTALHOSTSERVICESOK,
    MACRO_TOTALHOSTSERVICESWARNING,
    MACRO_TOTALHOSTSERVICESUNKNOWN,
    MACRO_TOTALHOSTSERVICESCRITICAL,
    MACRO_HOSTPROBLEMID,
    MACRO_LASTHOSTPROBLEMID
  };
  for (unsigned int i = 0;
       i < sizeof(to_free) / sizeof(*to_free);
       ++i) {
    delete[] mac->x[i];
    mac->x[i] = NULL;
  }

  // Clear custom host variables.
  customvariablesmember* next(NULL);
  for (customvariablesmember* it = mac->custom_host_vars;
       it != NULL;
       it = next) {
    next = it->next;
    delete[] it->variable_name;
    delete[] it->variable_value;
    delete it;
  }

  // Clear pointers.
  mac->custom_host_vars = NULL;
  mac->host_ptr = NULL;

  return (OK);
}

/**
 *  Clear global host macros.
 *
 *  @return OK on success.
 *
 *  @see clear_host_macros_r
 */
int clear_host_macros() {
  return (clear_host_macros_r(get_global_macros()));
}

}
