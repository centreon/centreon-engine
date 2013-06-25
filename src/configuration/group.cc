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

#include "com/centreon/engine/configuration/group.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace  com::centreon::engine::configuration;

/**
 *  Constructor.
 *
 *  @param[in] is_add_inherit True if add on merge list.
 */
group::group(bool is_add_inherit)
  : _is_add_inherit(is_add_inherit) {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
group::group(group const& right) {
  operator=(right);
}

/**
 *  Destructor.
 */
group::~group() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
group& group::operator=(group const& right) {
  if (this != &right) {
    _group = right._group;
    _is_add_inherit = right._is_add_inherit;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool group::operator==(group const& right) const throw () {
  return (_group == right._group
          && _is_add_inherit == right._is_add_inherit);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool group::operator!=(group const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Less-than operator.
 *
 *  @param[in] right Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool group::operator<(group const& right) const throw () {
  if (_is_add_inherit != right._is_add_inherit)
    return (_is_add_inherit != right._is_add_inherit);
  return (_group < right._group);
}

/**
 *  Clear group.
 */
void group::clear() {
  _group.clear();
}

/**
 *  Get if the group is empty.
 *
 *  @return True is the group is empty, otherwise false.
 */
bool group::empty() const throw () {
  return (_group.empty());
}

/**
 *  Get the group list.
 *
 *  @return The group list.
 */
std::list<std::string> const& group::get() const throw () {
  return (_group);
}

/**
 *  Get the group list.
 *
 *  @return The group list.
 */
std::list<std::string>& group::get() throw () {
  return (_group);
}

/**
 *  Get if the group add on inherit.
 *
 *  @return True if add inherit is true.
 */
bool group::is_add_inherit() const throw () {
  return (_is_add_inherit);
}

/**
 *  Set the add inherit property.
 *
 *  @param[in] enable True to enable add inherit.
 */
void group::is_add_inherit(bool enable) throw () {
  _is_add_inherit = enable;
}

/**
 *  Add or set group contents with inherit rules.
 *
 *  @param[in] grp The group to get data.
 */
void group::set(group const& grp) {
  if (_is_add_inherit)
    _group.insert(_group.end(), grp._group.begin(), grp._group.end());
  else
    _group = grp._group;
}

/**
 *  Set group contents with string argument.
 *
 *  @param[in] value The string arguments.
 */
void group::set(std::string const& value) {
  _group.clear();
  if (!value.empty()) {
    if (value[0] == '+') {
      _is_add_inherit = true;
      misc::split(value.substr(1), _group, ',');
    }
    else {
      _is_add_inherit = false;
      misc::split(value, _group, ',');
    }
  }
}
