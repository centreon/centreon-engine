/*
** Copyright 2011-2019 Centreon
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

#ifndef CCE_OBJECTS_CUSTOMVARIABLE_HH
#  define CCE_OBJECTS_CUSTOMVARIABLE_HH

#  include <map>
#  include <string>
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

/**
 * @class customvariable customvariable.hh "com/centreon/engine/objects/customvariable.hh"
 * @brief This class represent a customvariable
 *
 * This class represents a customvariable, it contains its name, its value and
 * others properties that can be useful.
 */
class                customvariable {
 public:
                     customvariable(
                       std::string const& key,
                       std::string const& value);
                     customvariable(customvariable const& other);
                     ~customvariable();
  customvariable&    operator=(customvariable const& other);
  bool               operator<(customvariable const& other) const;
  bool               operator==(customvariable const& other) const;
  bool               operator==(customvariable const& other);
  bool               operator!=(customvariable const& other);
  void               set_sent(bool sent);
  bool               is_sent() const;
  void               set_value(std::string const& value);
  std::string const& get_value() const;

 private:
  std::string        _key;
  std::string        _value;
  bool               _is_sent;
};

typedef std::map<std::string, customvariable> map_customvar;

CCE_END()

#endif // !CCE_OBJECTS_CUSTOMVARIABLE_HH
