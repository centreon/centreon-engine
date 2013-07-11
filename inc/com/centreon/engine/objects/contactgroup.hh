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

#ifndef CCE_OBJECTS_CONTACTGROUP_HH
#  define CCE_OBJECTS_CONTACTGROUP_HH

/* Forward declaration. */
struct contact_struct;
struct contactsmember_struct;

typedef struct                contactgroup_struct {
  char*                       group_name;
  char*                       alias;
  contactsmember_struct*      members;
  struct contactgroup_struct* next;
  struct contactgroup_struct* nexthash;
}                             contactgroup;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

contactgroup* add_contactgroup(char const* name, char const* alias);
int           is_contact_member_of_contactgroup(
                contactgroup_struct* group,
                contact_struct* cntct);

#  ifdef __cplusplus
}

#    include <ostream>
#    include <string>
#    include "com/centreon/engine/namespace.hh"

bool          operator==(
                contactgroup const& obj1,
                contactgroup const& obj2) throw ();
bool          operator!=(
                contactgroup const& obj1,
                contactgroup const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, contactgroup const& obj);

CCE_BEGIN()

contactgroup& find_contactgroup(std::string const& name);
bool          is_contactgroup_exist(std::string const& name) throw ();

CCE_END()

#  endif /* C++ */

#endif // !CCE_OBJECTS_CONTACTGROUP_HH


