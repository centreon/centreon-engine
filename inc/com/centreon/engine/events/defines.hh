/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCE_EVENTS_HH
#  define CCE_EVENTS_HH

// Event Types
#  define EVENT_SERVICE_CHECK      0   // active service check
#  define EVENT_COMMAND_CHECK      1   // external command check
#  define EVENT_PROGRAM_SHUTDOWN   2   // program shutdown
#  define EVENT_PROGRAM_RESTART    3   // program restart
#  define EVENT_CHECK_REAPER       4   // reaps results from host and service checks
#  define EVENT_RETENTION_SAVE     5   // save (dump) retention data
#  define EVENT_SFRESHNESS_CHECK   6   // checks service result "freshness"
#  define EVENT_HOST_CHECK         7   // active host check
#  define EVENT_HFRESHNESS_CHECK   8   // checks host result "freshness"
#  define EVENT_RESCHEDULE_CHECKS  9   // adjust scheduling of host and service checks
#  define EVENT_PROGRAM_RELOAD     10  // reload configuration
#  define EVENT_SLEEP              98  // asynchronous sleep event that occurs when event queues are empty
#  define EVENT_USER_FUNCTION      99  // USER-defined function (modules)

#endif // !CCE_EVENTS_HH
