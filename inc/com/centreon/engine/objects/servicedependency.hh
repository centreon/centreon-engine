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

#ifndef CCE_OBJECTS_SERVICEDEPENDENCY_HH
#  define CCE_OBJECTS_SERVICEDEPENDENCY_HH
#include "com/centreon/engine/namespace.hh"

/* Forward declaration. */
CCE_BEGIN()
class service;
class timeperiod;
CCE_END()

typedef struct                     servicedependency_struct {
  int                              dependency_type;
  char*                            dependent_host_name;
  char*                            dependent_service_description;
  char*                            host_name;
  char*                            service_description;
  char*                            dependency_period;
  int                              inherits_parent;
  int                              fail_on_ok;
  int                              fail_on_warning;
  int                              fail_on_unknown;
  int                              fail_on_critical;
  int                              fail_on_pending;
  int                              circular_path_checked;
  int                              contains_circular_path;

  com::centreon::engine::service*  master_service_ptr;
  com::centreon::engine::service*  dependent_service_ptr;
  com::centreon::engine::timeperiod*
                                   dependency_period_ptr;
  struct servicedependency_struct* next;
  struct servicedependency_struct* nexthash;
}                                  servicedependency;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

servicedependency* add_service_dependency(
                     char const* dependent_host_name,
                     char const* dependent_service_description,
                     char const* host_name,
                     char const* service_description,
                     int dependency_type,
                     int inherits_parent,
                     int fail_on_ok,
                     int fail_on_warning,
                     int fail_on_unknown,
                     int fail_on_critical,
                     int fail_on_pending,
                     char const* dependency_period);
int                check_for_circular_servicedependency_path(
                     servicedependency* root_dep,
                     servicedependency* dep,
                     int dependency_type);

#  ifdef __cplusplus
}

#    include <ostream>

bool          operator==(
                servicedependency const& obj1,
                servicedependency const& obj2) throw ();
bool          operator!=(
                servicedependency const& obj1,
                servicedependency const& obj2) throw ();
bool          operator<(
                servicedependency const& obj1,
                servicedependency const& obj2);
std::ostream& operator<<(
                std::ostream& os,
                servicedependency const& obj);
#  endif /* C++ */

#endif // !CCE_OBJECTS_SERVICEDEPENDENCY_HH


