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

#ifndef CCE_OBJECTS_SERVICEGROUP_HH
#  define CCE_OBJECTS_SERVICEGROUP_HH

/* Forward declaration. */
struct servicesmember_struct;

typedef struct                servicegroup_struct {
  char*                       group_name;
  char*                       alias;
  servicesmember_struct*      members;
  char*                       notes;
  char*                       notes_url;
  char*                       action_url;
  struct servicegroup_struct* next;
  struct servicegroup_struct* nexthash;
}                             servicegroup;

#  ifdef __cplusplus
#    include <ostream>

bool          operator==(
                servicegroup const& obj1,
                servicegroup const& obj2) throw ();
bool          operator!=(
                servicegroup const& obj1,
                servicegroup const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, servicegroup const& obj);

#  endif // C++

#endif // !CCE_OBJECTS_SERVICEGROUP_HH


