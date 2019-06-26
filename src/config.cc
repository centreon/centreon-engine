/*
** Copyright 1999-2008 Ethan Galstad
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

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <unordered_map>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;

/****************************************************************/
/**************** CONFIG VERIFICATION FUNCTIONS *****************/
/****************************************************************/

/* dfs status values */
#define DFS_UNCHECKED                    0      /* default value */
#define DFS_TEMP_CHECKED                 1      /* check just one time */
#define DFS_OK                           2      /* no problem */
#define DFS_NEAR_LOOP                    3      /* has trouble sons */
#define DFS_LOOPY                        4      /* is a part of a loop */

#define dfs_get_status(h) h->get_circular_path_checked()
#define dfs_unset_status(h) h->set_circular_path_checked(0)
#define dfs_set_status(h, flag) h->set_circular_path_checked(flag)
#define dfs_host_status(h) (h ? dfs_get_status(h) : DFS_OK)

/**
 * Modified version of Depth-first Search
 * http://en.wikipedia.org/wiki/Depth-first_search
 */
static int dfs_host_path(host* root) {
  if (!root)
    return DFS_NEAR_LOOP;

  if (dfs_get_status(root) != DFS_UNCHECKED)
    return dfs_get_status(root);

  /* Mark the root temporary checked */
  dfs_set_status(root, DFS_TEMP_CHECKED);

  /* We are scanning the children */
  for (host_map_unsafe::iterator
         it(root->child_hosts.begin()),
         end(root->child_hosts.end());
       it != end;
       it++) {
    int child_status = dfs_get_status(it->second);

    /* If a child is not checked, check it */
    if (child_status == DFS_UNCHECKED)
      child_status = dfs_host_path(it->second);

    /* If a child already temporary checked, its a problem,
     * loop inside, and its a acked status */
    if (child_status == DFS_TEMP_CHECKED) {
      dfs_set_status(it->second, DFS_LOOPY);
      dfs_set_status(root, DFS_LOOPY);
    }

    /* If a child already temporary checked, its a problem, loop inside */
    if (child_status == DFS_NEAR_LOOP || child_status == DFS_LOOPY) {
      /* if a node is know to be part of a loop, do not let it be less */
      if (dfs_get_status(root) != DFS_LOOPY)
        dfs_set_status(root, DFS_NEAR_LOOP);

      /* we already saw this child, it's a problem */
      dfs_set_status(it->second, DFS_LOOPY);
    }
  }

  /*
   * If root have been modified, do not set it OK
   * A node is OK if and only if all of his children are OK
   * If it does not have child, goes ok
   */
  if (dfs_get_status(root) == DFS_TEMP_CHECKED)
    dfs_set_status(root, DFS_OK);
  return dfs_get_status(root);
}

/* check for circular paths and dependencies */
int pre_flight_circular_check(int* w, int* e) {
  int found(false);
  int warnings(0);
  int errors(0);

  /* bail out if we aren't supposed to verify circular paths */
  if (!verify_circular_paths)
    return OK;

  /********************************************/
  /* check for circular paths between hosts   */
  /********************************************/

/* We clean the dsf status from previous check */
  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it)
    dfs_set_status(it->second.get(), DFS_UNCHECKED);

  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it)
    if (dfs_host_path(it->second.get()) == DFS_LOOPY)
      errors = 1;

  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it) {
    if (dfs_get_status(it->second.get()) == DFS_LOOPY)
      logger(log_verification_error, basic)
        << "Error: The host '" << it->first
        << "' is part of a circular parent/child chain!";
    /* clean DFS status */
    dfs_set_status(it->second.get(), DFS_UNCHECKED);
  }

  /********************************************/
  /* check for circular dependencies         */
  /********************************************/

  /* check execution dependencies between all services */
  for (servicedependency_mmap::iterator
         it(servicedependency::servicedependencies.begin()),
         end(servicedependency::servicedependencies.end());
       it != end;
       ++it) {

    /* clear checked flag for all dependencies */
    for (servicedependency_mmap::iterator
           it2(servicedependency::servicedependencies.begin()),
           end2(servicedependency::servicedependencies.end());
         it2 != end2;
         ++it2)
      it2->second->set_circular_path_checked(false);

    found = it->second->check_for_circular_servicedependency_path(
              it->second.get(),
              hostdependency::execution);
    if (found) {
      logger(log_verification_error, basic)
        << "Error: A circular execution dependency (which could result "
        "in a deadlock) exists for service '"
        << it->second->get_service_description() << "' on host '"
        << it->second->get_hostname() << "'!";
      errors++;
    }
  }

  /* check notification dependencies between all services */
  for (servicedependency_mmap::iterator
         it(servicedependency::servicedependencies.begin()),
         end(servicedependency::servicedependencies.end());
       it != end;
       ++it) {

    /* clear checked flag for all dependencies */
    for (servicedependency_mmap::iterator
           it2(servicedependency::servicedependencies.begin()),
           end2(servicedependency::servicedependencies.end());
         it2 != end2;
         ++it2)
      it2->second->set_circular_path_checked(false);

    found = it->second->check_for_circular_servicedependency_path(
              it->second.get(),
              hostdependency::notification);
    if (found) {
      logger(log_verification_error, basic)
        << "Error: A circular notification dependency (which could "
        "result in a deadlock) exists for service '"
        << it->second->get_service_description() << "' on host '"
        << it->second->get_hostname() << "'!";
      errors++;
    }
  }

  /* clear checked flag for all dependencies */
  for (servicedependency_mmap::iterator
         it(servicedependency::servicedependencies.begin()),
         end(servicedependency::servicedependencies.end());
       it != end;
       ++it)
    it->second->set_circular_path_checked(false);

  /* check execution dependencies between all hosts */
  for (hostdependency_mmap::iterator
         it(hostdependency::hostdependencies.begin()),
         end(hostdependency::hostdependencies.end());
       it != end;
       ++it) {

    /* clear checked flag for all dependencies */
    for (hostdependency_mmap::iterator
           it2(hostdependency::hostdependencies.begin()),
           end2(hostdependency::hostdependencies.end());
         it2 != end2;
         ++it2)
      it2->second->set_circular_path_checked(false);

    found = it->second->check_for_circular_hostdependency_path(
      it->second.get(), hostdependency::execution);
    if (found) {
      logger(log_verification_error, basic)
        << "Error: A circular execution dependency (which could "
        "result in a deadlock) exists for host '"
        << it->second->get_hostname() << "'!";
      errors++;
    }
  }

  /* check notification dependencies between all hosts */
  for (hostdependency_mmap::iterator
         it(hostdependency::hostdependencies.begin()),
         end(hostdependency::hostdependencies.end());
       it != end;
       ++it) {

    /* clear checked flag for all dependencies */
    for (hostdependency_mmap::iterator
           it2(hostdependency::hostdependencies.begin()),
           end2(hostdependency::hostdependencies.end());
         it2 != end2;
         ++it2)
      it2->second->set_circular_path_checked(false);

    found = it->second->check_for_circular_hostdependency_path(
      it->second.get(), hostdependency::notification);
    if (found) {
      logger(log_verification_error, basic)
        << "Error: A circular notification dependency (which could "
        "result in a deadlock) exists for host '"
        << it->first << "'!";
      errors++;
    }
  }

  /* clear checked flag for all dependencies */
  for (hostdependency_mmap::iterator
         it(hostdependency::hostdependencies.begin()),
         end(hostdependency::hostdependencies.end());
       it != end;
       ++it)
      it->second->set_circular_path_checked(false);

  /* update warning and error count */
  if (w != nullptr)
    *w += warnings;
  if (e != nullptr)
    *e += errors;

  return (errors > 0) ? ERROR : OK;
}
