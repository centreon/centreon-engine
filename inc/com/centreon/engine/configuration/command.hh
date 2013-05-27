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

#ifndef CCE_CONFIGURATION_COMMAND_HH
#  define CCE_CONFIGURATION_COMMAND_HH

#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    command
    : public object {
  public:
                           command();
                           command(command const& right);
                           ~command() throw ();
    command&               operator=(command const& right);
    bool                   operator==(
                             command const& right) const throw ();
    bool                   operator!=(
                             command const& right) const throw ();
    /*
    std::string const&     command_line() const throw ();
    std::string const&     command_name() const throw ();
    std::string const&     connector() const throw ();
    */

    std::size_t            id() const throw ();
    void                   merge(object const& obj);
    bool                   parse(
                             std::string const& key,
                             std::string const& value);

  private:
    bool                   _set_command_line(std::string const& value);
    bool                   _set_command_name(std::string const& value);
    bool                   _set_connector(std::string const& value);

    std::string            _command_line;
    std::string            _command_name;
    std::string            _connector;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_COMMAND_HH


