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

#ifndef CCE_OBJECTS_SERVICESMEMBER_HH
#  define CCE_OBJECTS_SERVICESMEMBER_HH

/* Forward declaration. */
struct host_struct;
struct service_struct;
struct servicegroup_struct;

typedef struct                  servicesmember_struct {
  char*                         host_name;
  char*                         service_description;
  service_struct*               service_ptr;
  struct servicesmember_struct* next;
}                               servicesmember;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

servicesmember* add_service_link_to_host(
                  host_struct* hst,
                  service_struct* service_ptr);
servicesmember* add_service_to_servicegroup(
                  servicegroup_struct* temp_servicegroup,
                  char const* host_name,
                  char const* svc_description);

#  ifdef __cplusplus
}

#    include <ostream>

bool          operator==(
                servicesmember const& obj1,
                servicesmember const& obj2) throw ();
bool          operator!=(
                servicesmember const& obj1,
                servicesmember const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, servicesmember const& obj);

#  endif /* C++ */

#endif // !CCE_OBJECTS_SERVICESMEMBER_HH


