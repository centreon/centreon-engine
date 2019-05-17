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

#ifndef CCE_OBJECTS_HOSTGROUP_HH
#  define CCE_OBJECTS_HOSTGROUP_HH

/* Forward declaration. */
CCE_BEGIN()
  class host;
CCE_END()
struct hostsmember_struct;

typedef struct             hostgroup_struct {
  char*                    group_name;
  char*                    alias;
  hostsmember_struct*      members;
  char*                    notes;
  char*                    notes_url;
  char*                    action_url;
  struct hostgroup_struct* next;
  struct hostgroup_struct* nexthash;
}                          hostgroup;

/* Other HOSTGROUP structure. */
struct                      hostgroup_other_properties {
  unsigned int              hostgroup_id;
};

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

hostgroup* add_hostgroup(
             char const* name,
             char const* alias,
             char const* notes,
             char const* notes_url,
             char const* action_url);
int        is_host_member_of_hostgroup(
             hostgroup_struct* group,
             com::centreon::engine::host* hst);

#  ifdef __cplusplus
}

#    include <ostream>
#    include <string>
#    include "com/centreon/engine/namespace.hh"

bool          operator==(
                hostgroup const& obj1,
                hostgroup const& obj2) throw ();
bool          operator!=(
                hostgroup const& obj1,
                hostgroup const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, hostgroup const& obj);

CCE_BEGIN()

hostgroup&    find_hostgroup(std::string const& name);
bool          is_hostgroup_exist(std::string const& name) throw ();
unsigned int  get_hostgroup_id(char const* name);

CCE_END()

#  endif /* C++ */

#endif // !CCE_OBJECTS_HOSTGROUP_HH


