/*
** Copyright 2000-2006 Ethan Galstad
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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/xsddefault.hh"

/******************************************************************/
/****************** TOP-LEVEL OUTPUT FUNCTIONS ********************/
/******************************************************************/

/**
 *  Updates program status info.
 *
 *  @return OK.
 */
int update_program_status() {
  broker_program_status(
    NEBTYPE_PROGRAMSTATUS_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    NULL);
  return (OK);
}

/**
 *  Updates host status info.
 *
 *  @param[in] hst  Host.
 *
 *  @return OK.
 */
int update_host_status(host* hst) {
  broker_host_status(
    NEBTYPE_HOSTSTATUS_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    NULL);
  return (OK);
}

/**
 *  Updates service status info.
 *
 *  @param[in] svc  Service.
 *
 *  @return OK.
 */
int update_service_status(service* svc) {
  broker_service_status(
    NEBTYPE_SERVICESTATUS_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    NULL);
  return (OK);
}
