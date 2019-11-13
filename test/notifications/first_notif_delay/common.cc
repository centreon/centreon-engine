/*
** Copyright 2011-2013 Merethis
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

#include "test/notifications/first_notif_delay/common.hh"
#include <unistd.h>
#include <cstdio>
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/io/file_stream.hh"

using namespace com::centreon;

/**
 *  Create a default setup for use with first_notification_delay unit
 *  tests.
 *
 *  @return 0 on success.
 */
int engine::first_notif_delay_default_setup(std::string const& path) {
  // Interval length is 1 second.
  config->interval_length(1);

  // Remove flag file.
  io::file_stream::remove(path);

  // Return value.
  int retval(0);

  // Add host.
  host* hst(add_host(const_cast<char*>("myhost"),     // Name
                     const_cast<char*>("MyHost"),     // Display name
                     const_cast<char*>("MyAlias"),    // Alias
                     const_cast<char*>("127.0.0.1"),  // Address
                     NULL,                            // Check period
                     0,                               // Initial state
                     1.0,                             // Check interval
                     1.0,                             // Retry interval
                     1,                               // Max attempts
                     0,                               // Notify up
                     1,                               // Notify down
                     1,                               // Notify unreachable
                     0,                               // Notify flapping
                     0,                               // Notify downtime
                     1.0,                             // Notification interval
                     FIRST_NOTIF_DELAY,  // First notification delay
                     NULL,               // Notification period
                     1,                  // Notifications enabled
                     const_cast<char*>("mycommand"),  // Check command
                     1,                               // Checks enabled
                     1,                               // Accept passive checks
                     NULL,                            // Event handler
                     0,                               // Event handler enabled
                     0,                               // Flap detection enabled
                     0.0,                             // Low flap threshold
                     0.0,                             // High flap threshold
                     0,                               // Flap detection on up
                     0,                               // Flap detection on down
                     0,     // Flap detection on unreachable
                     0,     // Stalk on ok
                     0,     // Stalk on down
                     0,     // Stalk on unreachable
                     0,     // Process perfdata
                     0,     // Failure prediction enabled
                     NULL,  // Failure prediction options
                     0,     // Check freshness
                     0,     // Freshness threshold
                     NULL,  // Notes
                     NULL,  // Notes URL
                     NULL,  // Action URL
                     NULL,  // Icon image
                     NULL,  // Icon image alt
                     NULL,  // VRML image
                     NULL,  // Statusmap image
                     0,     // X2D
                     0,     // Y2D
                     0,     // Have 2D coords
                     0.0,   // X3D
                     0.0,   // Y3D
                     0.0,   // Z3D
                     0,     // Have 3D coords
                     0,     // Should be drawn
                     0,     // Retain status information
                     0,     // Retain non-status information
                     0));   // Obsess over
  retval |= (NULL == hst);
  host_list->has_been_checked = 1;
  host_list->last_check = time(NULL);
  config->cached_host_check_horizon(24 * 60 * 60);

  // Add service.
  service* svc(
      add_service(const_cast<char*>("myhost"),     // Host name
                  const_cast<char*>("myservice"),  // Service description
                  const_cast<char*>("MyService"),  // Display name
                  NULL,                            // Check period
                  0,                               // Initial state
                  1,                               // Max attempts
                  1,                               // Parallelize
                  1,                               // Accept passive checks
                  1.0,                             // Check interval
                  1.0,                             // Retry interval
                  1.0,                             // Notification interval
                  FIRST_NOTIF_DELAY,               // First notification delay
                  NULL,                            // Notification period
                  0,                               // Notify recovery
                  1,                               // Notify unknown
                  1,                               // Notify warning
                  1,                               // Notify critical
                  0,                               // Notify flapping
                  0,                               // Notify downtime
                  1,                               // Notifications enabled
                  0,                               // Volatile
                  NULL,                            // Event handler
                  0,                               // Event handler enabled
                  const_cast<char*>("mycommand"),  // Check command
                  1,                               // Checks enabled
                  0,                               // Flap detection enabled
                  0.0,                             // Low flap threshold
                  0.0,                             // High flap threshold
                  0,                               // Flap detection on ok
                  0,                               // Flap detection on warning
                  0,                               // Flap detection on unknown
                  0,                               // Flap detection on critical
                  0,                               // Stalk on ok
                  0,                               // Stalk on warning
                  0,                               // Stalk on unknown
                  0,                               // Stalk on critical
                  0,                               // Process perfdata
                  0,                               // Failure prediction enabled
                  NULL,                            // Failure prediction options
                  0,                               // Check freshness
                  0,                               // Freshness threshold
                  NULL,                            // Notes
                  NULL,                            // Notes URL
                  NULL,                            // Action URL
                  NULL,                            // Icon image
                  NULL,                            // Icon image alt
                  0,                               // Retain status information
                  0,    // Retain non-status information
                  0));  // Obsess over service
  retval |= (NULL == svc);
  service_list->host_ptr = host_list;

  // Add contact.
  contact* cntct(add_contact(const_cast<char*>("mycontact"),        // Name
                             const_cast<char*>("MyContact"),        // Alias
                             const_cast<char*>("my@contact.info"),  // Email
                             NULL,                                  // Pager
                             NULL,                                  // Addresses
                             NULL,  // Service notification period
                             NULL,  // Host notification period
                             0,     // Notify service ok
                             1,     // Notify service critical
                             1,     // Notify service warning
                             1,     // Notify service unknown
                             0,     // Notify service flapping
                             0,     // Notify service downtime
                             0,     // Notify host up
                             1,     // Notify host down
                             1,     // Notify host unreachable
                             0,     // Notify host flapping
                             0,     // Notify host downtime
                             1,     // Host notifications enabled
                             1,     // Service notifications enabled
                             1,     // Can submit commands
                             0,     // Retain status information
                             0));   // Retain non-status information
  retval |= (NULL == cntct);
  retval |= (NULL == add_contact_to_host(hst, const_cast<char*>("mycontact")));
  hst->contacts->contact_ptr = cntct;
  retval |=
      (NULL == add_contact_to_service(svc, const_cast<char*>("mycontact")));
  svc->contacts->contact_ptr = cntct;

  // Add command.
  std::string cmd_str("/usr/bin/env touch " + path);
  command* cmd(add_command("mycommand", cmd_str.c_str()));
  retval |=
      (NULL == add_host_notification_command_to_contact(cntct, "mycommand"));
  retval |= (NULL == cmd);
  cntct->host_notification_commands->command_ptr = cmd;
  retval |=
      (NULL == add_service_notification_command_to_contact(cntct, "mycommand"));
  cntct->service_notification_commands->command_ptr = cmd;

  return (retval);
}
