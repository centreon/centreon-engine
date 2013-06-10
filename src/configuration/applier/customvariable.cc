/*
** Copyright 2013 Merethis
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

#include "com/centreon/engine/configuration/applier/customvariable.hh"

/**
 *  Compare a custom variables list with some properties.
 *
 *  @param[in] left  Custom variables list.
 *  @param[in] right Properties.
 *
 *  @return True if both match.
 */
bool operator==(
       customvariablesmember const* left,
       properties const& right) {
  properties::const_iterator
    it(right.begin()),
    end(right.end());
  while (left && (it != end)) {
    if (strcmp(left->variable_name, it->first.c_str())
        || strcmp(left->variable_value, it->second.c_str()))
      return (false);
    left = left->next;
    ++it;
  }
  return (!left && (it == end));
}

/**
 *  Compare some properties with a custom variables list.
 *
 *  @param[in] left  Properties.
 *  @param[in] right Custom variables list.
 *
 *  @return True if both match.
 */
bool operator==(
       properties const& left,
       customvariablesmember const* right) {
  return (operator==(right, left));
}

/**
 *  Compare a custom variables list with some properties.
 *
 *  @param[in] left  Custom variables list.
 *  @param[in] right Properties.
 *
 *  @return False if both match.
 */
bool operator!=(
       customvariablesmember const* left,
       properties const& right) {
  return (!operator==(left, right));
}

/**
 *  Compare some properties with a custom variables list.
 *
 *  @param[in] left  Properties.
 *  @param[in] right Custom variables list.
 *
 *  @return False if both match.
 */
bool operator!=(
       properties const& left,
       customvariablesmember const* right) {
  return (!operator==(right, left));
}
