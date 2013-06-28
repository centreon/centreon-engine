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

#ifndef CCE_OBJECTS_HOSTDEPENDENCY_HH
#  define CCE_OBJECTS_HOSTDEPENDENCY_HH

/* Forward declaration. */
struct host_struct;
struct timeperiod_struct;

typedef struct                  hostdependency_struct {
  int                           dependency_type;
  char*                         dependent_host_name;
  char*                         host_name;
  char*                         dependency_period;
  int                           inherits_parent;
  int                           fail_on_up;
  int                           fail_on_down;
  int                           fail_on_unreachable;
  int                           fail_on_pending;
  int                           circular_path_checked;
  int                           contains_circular_path;

  host_struct*                  master_host_ptr;
  host_struct*                  dependent_host_ptr;
  timeperiod_struct*            dependency_period_ptr;
  struct hostdependency_struct* next;
  struct hostdependency_struct* nexthash;
}                               hostdependency;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

hostdependency* add_host_dependency(
                  char const* dependent_host_name,
                  char const* host_name,
                  int dependency_type,
                  int inherits_parent,
                  int fail_on_up,
                  int fail_on_down,
                  int fail_on_unreachable,
                  int fail_on_pending,
                  char const* dependency_period);
int             check_for_circular_hostdependency_path(
                  hostdependency* root_dep,
                  hostdependency* dep,
                  int dependency_type);

#  ifdef __cplusplus
}

#    include <ostream>

bool          operator==(
                hostdependency const& obj1,
                hostdependency const& obj2) throw ();
bool          operator!=(
                hostdependency const& obj1,
                hostdependency const& obj2) throw ();
bool          operator<(
                hostdependency const& obj1,
                hostdependency const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, hostdependency const& obj);

#  endif /* C++ */

#endif // !CCE_OBJECTS_HOSTDEPENDENCY_HH


