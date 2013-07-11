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

#ifndef CCE_OBJECTS_COMMAND_HH
#  define CCE_OBJECTS_COMMAND_HH

typedef struct           command_struct {
  char*                  name;
  char*                  command_line;
  struct command_struct* next;
  struct command_struct* nexthash;
}                        command;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

command* add_command(char const* name, char const* value);

#  ifdef __cplusplus
}

#    include <ostream>
#    include <string>
#    include "com/centreon/engine/namespace.hh"

bool          operator==(
                command const& obj1,
                command const& obj2) throw ();
bool          operator!=(
                command const& obj1,
                command const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, command const& obj);

CCE_BEGIN()

command&      find_command(std::string const& name);
bool          is_command_exist(std::string const& name) throw ();

CCE_END()

#  endif /* C++ */

#endif // !CCE_OBJECTS_COMMAND_HH


