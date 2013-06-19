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

#ifndef CCE_OBJECTS_CONTACTSMEMBER_HH
#  define CCE_OBJECTS_CONTACTSMEMBER_HH

/* Forward declaration. */
struct contact_struct;

typedef struct                  contactsmember_struct {
  char*                         contact_name;
  contact_struct*               contact_ptr;
  struct contactsmember_struct* next;
}                               contactsmember;

#  ifdef __cplusplus
#    include <ostream>

bool          operator==(
                contactsmember const& obj1,
                contactsmember const& obj2) throw ();
bool          operator!=(
                contactsmember const& obj1,
                contactsmember const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, contactsmember const& obj);

#  endif // C++

#endif // !CCE_OBJECTS_CONTACTSMEMBER_HH


