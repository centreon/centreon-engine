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

#include "com/centreon/engine/configuration/group.hh"
#include <algorithm>
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Constructor.
 *
 *  @param[in] is_inherit  True if add on merge list.
 */
template <typename T>
group<T>::group(bool is_inherit)
    : _is_inherit(is_inherit), _is_null(false), _is_set(false) {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  The object to copy.
 */
template <typename T>
group<T>::group(group const& other) {
  operator=(other);
}

/**
 *  Destructor.
 */
template <typename T>
group<T>::~group() throw() {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  The object to copy.
 *
 *  @return This object.
 */
template <typename T>
group<T>& group<T>::operator=(group const& other) {
  if (this != &other) {
    _data = other._data;
    _is_inherit = other._is_inherit;
    _is_null = other._is_null;
    _is_set = other._is_set;
  }
  return *this;
}

/**
 *  Assignment operator.
 *
 *  @param[in] other  The object to copy.
 *
 *  @return This object.
 */
template <typename T>
group<T>& group<T>::operator=(std::string const& other) {
  _data.clear();
  if (!other.empty()) {
    if (other[0] == '+') {
      _is_inherit = true;
      string::split(other.substr(1), _data, ',');
    } else if (other == "null")
      _is_null = true;
    else {
      _is_inherit = false;
      string::split(other, _data, ',');
    }
  }
  _is_set = true;
  return *this;
}

/**
 *  Add data.
 *
 *  @param[in] other  The object to add.
 *
 *  @return This object.
 */
template <typename T>
group<T>& group<T>::operator+=(group<T> const& other) {
  if (this != &other) {
    std::copy(other._data.begin(), other._data.end(),
              std::inserter(_data, _data.end()));
    _is_set = true;
  }
  return *this;
}

/**
 *  Equal operator.
 *
 *  @param[in] other  The object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
template <typename T>
bool group<T>::operator==(group const& other) const throw() {
  return _data == other._data;
}

/**
 *  Not equal operator.
 *
 *  @param[in] other  The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
template <typename T>
bool group<T>::operator!=(group const& other) const throw() {
  return !operator==(other);
}

/**
 *  Less-than operator.
 *
 *  @param[in] other  Object to compare to.
 *
 *  @return True if this object is less than other.
 */
template <typename T>
bool group<T>::operator<(group const& other) const throw() {
  return _data < other._data;
}

/**
 *  Clear group.
 */
template <typename T>
void group<T>::reset() {
  _data.clear();
  _is_inherit = false;
  _is_null = false;
  _is_set = false;
}

// Explicit instantiations.
template class com::centreon::engine::configuration::group<list_string>;
template class com::centreon::engine::configuration::group<set_string>;
template class com::centreon::engine::configuration::group<set_pair_string>;
