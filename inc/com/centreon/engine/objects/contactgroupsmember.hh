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

#ifndef CCE_OBJECTS_CONTACTGROUPSMEMBER_HH
#  define CCE_OBJECTS_CONTACTGROUPSMEMBER_HH

/* Forward declaration. */
struct contactgroup_struct;
struct host_struct;
struct hostescalation_struct;
struct service_struct;
struct serviceescalation_struct;

typedef struct                       contactgroupsmember_struct {
  char*                              group_name;
  contactgroup_struct*               group_ptr;
  struct contactgroupsmember_struct* next;
}                                    contactgroupsmember;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

contactgroupsmember* add_contactgroup_to_host(
                       host_struct* hst,
                       char const* group_name);
contactgroupsmember* add_contactgroup_to_host_escalation(
                       hostescalation_struct* he,
                       char const* group_name);
contactgroupsmember* add_contactgroup_to_service(
                       service_struct* svc,
                       char const* group_name);
contactgroupsmember* add_contactgroup_to_serviceescalation(
                       serviceescalation_struct* se,
                       char const* group_name);

#  ifdef __cplusplus
}

#    include <ostream>

bool          operator==(
                contactgroupsmember const& obj1,
                contactgroupsmember const& obj2) throw ();
bool          operator!=(
                contactgroupsmember const& obj1,
                contactgroupsmember const& obj2) throw ();
bool          operator<(
                contactgroupsmember const& obj1,
                contactgroupsmember const& obj2) throw ();
std::ostream& operator<<(
                std::ostream& os,
                contactgroupsmember const& obj);

#  endif /* C++ */

#endif // !CCE_OBJECTS_CONTACTGROUPSMEMBER_HH


