/*
** Copyright 2011-2013,2015 Merethis
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

#ifndef CCE_OBJECTS_HOSTSMEMBER_HH
#  define CCE_OBJECTS_HOSTSMEMBER_HH

/* Forward declaration. */
struct host_struct;

typedef struct               hostsmember_struct {
  char*                      host_name;
  host_struct*               host_ptr;
  struct hostsmember_struct* next;
}                            hostsmember;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

hostsmember* add_child_link_to_host(
               host_struct* hst,
               host_struct* child_ptr);
hostsmember* add_parent_host_to_host(
               host_struct* hst,
               char const* host_name);

#  ifdef __cplusplus
}

#    include <ostream>

bool          operator==(
                hostsmember const& obj1,
                hostsmember const& obj2) throw ();
bool          operator!=(
                hostsmember const& obj1,
                hostsmember const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, hostsmember const& obj);

#  endif /* C++ */

#endif // !CCE_OBJECTS_HOSTSMEMBER_HH
