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

#ifndef CCE_CONFIGURATION_APPLIER_COMMAND_HH
#  define CCE_CONFIGURATION_APPLIER_COMMAND_HH

#  include <list>
#  include <string>
#  include "com/centreon/engine/configuration/command.hh"
#  include "com/centreon/engine/namespace.hh"

struct commandsmember_struct;

CCE_BEGIN()

namespace             configuration {
  namespace           applier {
    class             command {
    public:
                      command();
                      command(command const& right);
                      ~command() throw ();
      command&        operator=(command const& right);
      void            add_object(command_ptr obj);
      void            modify_object(command_ptr obj);
      void            remove_object(command_ptr obj);
      void            resolve_object(command_ptr obj);
    };
  }
}

CCE_END()

bool                  operator==(
                        commandsmember_struct const* left,
                        std::list<std::string> const& right);
bool                  operator==(
                        std::list<std::string> const& left,
                        commandsmember_struct const* right);
bool                  operator!=(
                        commandsmember_struct const* left,
                        std::list<std::string> const& right);
bool                  operator!=(
                        std::list<std::string> const& left,
                        commandsmember_struct const* right);

#endif // !CCE_CONFIGURATION_APPLIER_COMMAND_HH
