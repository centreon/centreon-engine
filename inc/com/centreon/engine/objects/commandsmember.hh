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

#ifndef CCE_OBJECTS_COMMANDSMEMBER_HH
#  define CCE_OBJECTS_COMMANDSMEMBER_HH

#  include <ostream>

/* Forward declaration. */
struct command_struct;

typedef struct                  commandsmember_struct {
  char*                         cmd;
  command_struct*               command_ptr;
  struct commandsmember_struct* next;
}                               commandsmember;

bool          operator==(
                commandsmember const& obj1,
                commandsmember const& obj2) throw ();
bool          operator!=(
                commandsmember const& obj1,
                commandsmember const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, commandsmember const& obj);

#endif // !CCE_OBJECTS_COMMANDSMEMBER_HH
