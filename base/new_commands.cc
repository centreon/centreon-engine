/*
** Copyright 2011 Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <string>
#include "centreonscheduler.nsmap" // gSOAP namespaces.
#include "soapH.h"

/**
 *  Enable or disable hosts passive checks globally.
 *
 *  @param[in]  s      Unused.
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res    Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostsPassiveChecksEnabled(soap* s,
                                                    bool enable,
                                                    centreonscheduler__setHostsPassiveChecksEnabledResponse& res) {
}

/**
 *  Enable or disable services passive checks globally.
 *
 *  @param[in]  s      Unused.
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res    Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setServicesPassiveChecksEnabled(soap* s,
                                                       bool enable,
                                                       centreonscheduler__setServicesPassiveChecksEnabledResponse& res) {
}
