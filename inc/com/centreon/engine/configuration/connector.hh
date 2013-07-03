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

#ifndef CCE_CONFIGURATION_CONNECTOR_HH
#  define CCE_CONFIGURATION_CONNECTOR_HH

#  include <set>
#  include "com/centreon/engine/commands/connector.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    connector
    : public object {
  public:
                           connector();
                           connector(connector const& right);
                           ~connector() throw ();
    connector&             operator=(connector const& right);
    bool                   operator==(
                             connector const& right) const throw ();
    bool                   operator!=(
                             connector const& right) const throw ();
    bool                   operator<(
                             connector const& right) const throw ();
    void                   check_validity() const;
    void                   merge(object const& obj);
    bool                   parse(
                             std::string const& key,
                             std::string const& value);

    std::string const&     connector_line() const throw ();
    std::string const&     connector_name() const throw ();

  private:
    bool                   _set_connector_line(std::string const& value);
    bool                   _set_connector_name(std::string const& value);

    std::string            _connector_line;
    std::string            _connector_name;
  };

  typedef shared_ptr<connector>   connector_ptr;
  typedef std::set<connector_ptr> set_connector;
}

CCE_END()

#endif // !CCE_CONFIGURATION_CONNECTOR_HH

