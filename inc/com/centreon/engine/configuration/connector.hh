/*
** Copyright 2011-2013,2017 Centreon
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

#  include <memory>
#  include <set>
#  include <string>
#  include "com/centreon/engine/commands/connector.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    connector : public object {
   public:
    typedef std::string    key_type;

                           connector(key_type const& key = "");
                           connector(connector const& right);
                           ~connector() throw () override;
    connector&             operator=(connector const& right);
    bool                   operator==(
                             connector const& right) const throw ();
    bool                   operator!=(
                             connector const& right) const throw ();
    bool                   operator<(
                             connector const& right) const throw ();
    void                   check_validity() const override;
    key_type const&        key() const throw ();
    void                   merge(object const& obj) override;
    bool                   parse(char const* key, char const* value) override;

    std::string const&     connector_line() const throw ();
    std::string const&     connector_name() const throw ();

   private:
    typedef bool (*setter_func)(connector&, char const*);

    bool                   _set_connector_line(std::string const& value);
    bool                   _set_connector_name(std::string const& value);

    std::string            _connector_line;
    std::string            _connector_name;
    static std::unordered_map<std::string, setter_func> const _setters;
  };

  typedef std::shared_ptr<connector> connector_ptr;
  typedef std::set<connector>        set_connector;
}

CCE_END()

#endif // !CCE_CONFIGURATION_CONNECTOR_HH
