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
#include "com/centreon/engine/string.hh"

using namespace  com::centreon::engine::configuration;

/**
 *  Constructor.
 *
 *  @param[in] is_add_inherit True if add on merge list.
 */
group::group(bool is_inherit)
  : _is_inherit(is_inherit),
    _is_null(false),
    _is_set(false) {

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
    _data = right._data;
    _is_inherit = right._is_inherit;
    _is_null = right._is_null;
    _is_set = right._is_set;
  }
  return (*this);
}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
group& group::operator=(std::string const& right) {
  _data.clear();
  if (!right.empty()) {
    if (right[0] == '+') {
      _is_inherit = true;
      string::split(right.substr(1), _data, ',');
    }
    else if (right == "null")
      _is_null = true;
    else {
      _is_inherit = false;
      string::split(right, _data, ',');
    }
  }
  _is_set = true;
  return (*this);
}

/**
 *  Add data.
 *
 *  @param[in] right The object to add.
 *
 *  @return This object.
 */
group& group::operator+=(group const& right) {
  if (this != &right) {
    _data.insert(_data.end(), right._data.begin(), right._data.end());
    _is_set = true;
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
  return (_is_inherit == right._is_inherit
          && _is_set == right._is_set
          && _is_null == right._is_null
          && _data == right._data);
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
  if (_is_inherit != right._is_inherit)
    return (_is_inherit != right._is_inherit);
  if (_is_null != right._is_null)
    return (_is_null != right._is_null);
  if (_is_set != right._is_set)
    return (_is_set != right._is_set);
  return (_data < right._data);
}

/**
 *  Clear group.
 */
void group::reset() {
  _data.clear();
  _is_inherit = false;
  _is_null = false;
  _is_set = false;
}
