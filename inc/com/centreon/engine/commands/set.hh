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

#ifndef CCE_COMMANDS_SET_HH
#  define CCE_COMMANDS_SET_HH

#  include <string>
#  include "com/centreon/engine/commands/command.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"
#  include "com/centreon/unordered_hash.hh"

CCE_BEGIN()

namespace       commands {
  /**
   *  @class set set.hh
   *  @brief Store all command.
   *
   *  Set is a singleton to store all command class and have a simple
   *  access to used it.
   */
  class         set {
  public:
    void        add_command(command const& cmd);
    void        add_command(
                            shared_ptr<command> cmd);
    shared_ptr<command>
                get_command(std::string const& cmd_name);
    static set& instance();
    static void load();
    void        remove_command(std::string const& cmd_name);
    static void unload();

  private:
                set();
                set(set const& right);
                ~set() throw ();
    set&        operator=(set const& right);

    umap<std::string, shared_ptr<command> >
                _list;
  };
}

CCE_END()

#endif // !CCE_COMMANDS_SET_HH
