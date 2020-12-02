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

#include "com/centreon/engine/macros/clear_service.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/macros/misc.hh"

/**
 *  Clear service macros.
 *
 *  @param[in,out] mac Macro object.
 *
 *  @return OK on success.
 */
int clear_service_macros_r(nagios_macros* mac) {
  static unsigned int const to_free[] = {MACRO_SERVICEDESC,
                                         MACRO_SERVICEDESC,
                                         MACRO_SERVICEDISPLAYNAME,
                                         MACRO_SERVICEOUTPUT,
                                         MACRO_LONGSERVICEOUTPUT,
                                         MACRO_SERVICEPERFDATA,
                                         MACRO_SERVICECHECKCOMMAND,
                                         MACRO_SERVICECHECKTYPE,
                                         MACRO_SERVICESTATETYPE,
                                         MACRO_SERVICESTATE,
                                         MACRO_SERVICEISVOLATILE,
                                         MACRO_SERVICESTATEID,
                                         MACRO_SERVICEATTEMPT,
                                         MACRO_MAXSERVICEATTEMPTS,
                                         MACRO_SERVICEEXECUTIONTIME,
                                         MACRO_SERVICELATENCY,
                                         MACRO_LASTSERVICECHECK,
                                         MACRO_LASTSERVICESTATECHANGE,
                                         MACRO_LASTSERVICEOK,
                                         MACRO_LASTSERVICEWARNING,
                                         MACRO_LASTSERVICEUNKNOWN,
                                         MACRO_LASTSERVICECRITICAL,
                                         MACRO_SERVICEDOWNTIME,
                                         MACRO_SERVICEPERCENTCHANGE,
                                         MACRO_SERVICEDURATIONSEC,
                                         MACRO_SERVICEDURATION,
                                         MACRO_SERVICENOTIFICATIONNUMBER,
                                         MACRO_SERVICENOTIFICATIONID,
                                         MACRO_SERVICEEVENTID,
                                         MACRO_LASTSERVICEEVENTID,
                                         MACRO_SERVICEACTIONURL,
                                         MACRO_SERVICENOTESURL,
                                         MACRO_SERVICENOTES,
                                         MACRO_SERVICEGROUPNAMES,
                                         MACRO_SERVICEPROBLEMID,
                                         MACRO_LASTSERVICEPROBLEMID,
                                         MACRO_SERVICEID,
                                         MACRO_SERVICETIMEZONE};
  for (unsigned int i = 0; i < sizeof(to_free) / sizeof(*to_free); ++i) {
    mac->x[i] = "";
  }

  // Clear custom service variables.
  mac->custom_service_vars.clear();

  // Clear pointers.
  mac->service_ptr = nullptr;

  return OK;
}
