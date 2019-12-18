/*
** Copyright 2002-2006      Ethan Galstad
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

#ifndef CCE_EVENTS_HH
#define CCE_EVENTS_HH

// Event Types
#define EVENT_SERVICE_CHECK 0     // active service check
#define EVENT_COMMAND_CHECK 1     // external command check
#define EVENT_LOG_ROTATION 2      // log file rotation
#define EVENT_PROGRAM_SHUTDOWN 3  // program shutdown
#define EVENT_PROGRAM_RESTART 4   // program restart
#define EVENT_CHECK_REAPER 5      // reaps results from host and service checks
#define EVENT_ORPHAN_CHECK 6      // checks for orphaned hosts and services
#define EVENT_RETENTION_SAVE 7    // save (dump) retention data
#define EVENT_STATUS_SAVE 8       // save (dump) status data
#define EVENT_SCHEDULED_DOWNTIME 9  // scheduled host or service downtime
#define EVENT_SFRESHNESS_CHECK 10   // checks service result "freshness"
#define EVENT_EXPIRE_DOWNTIME \
  11  // checks for (and removes) expired scheduled downtime
#define EVENT_HOST_CHECK 12        // active host check
#define EVENT_HFRESHNESS_CHECK 13  // checks host result "freshness"
#define EVENT_RESCHEDULE_CHECKS \
  14                             // adjust scheduling of host and service checks
#define EVENT_EXPIRE_COMMENT 15  // removes expired comments
#define EVENT_EXPIRE_HOST_ACK 16     // remove expired host acknowledgement
#define EVENT_EXPIRE_SERVICE_ACK 17  // remove expired service acknowledgement
#define EVENT_ENGINERPC_CHECK 18 // EngineRPC command check
#define EVENT_SLEEP \
  98  // asynchronous sleep event that occurs when event queues are empty
#define EVENT_USER_FUNCTION 99  // USER-defined function (modules)

#endif  // !CCE_EVENTS_HH
