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

#include "com/centreon/engine/configuration/point_2d.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Constructor.
 *
 *  @param[in] x The x coordinates.
 *  @param[in] y The y coordinates.
 */
point_2d::point_2d(int x, int y) : _x(x), _y(y) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
point_2d::point_2d(point_2d const& right) {
  operator=(right);
}

/**
 *  Destructor.
 */
point_2d::~point_2d() throw() {}

/**
 *  Copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
point_2d& point_2d::operator=(point_2d const& right) {
  if (this != &right) {
    _x = right._x;
    _y = right._y;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The opbject to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool point_2d::operator==(point_2d const& right) const throw() {
  return (_x == right._x && _y == right._y);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The opbject to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool point_2d::operator!=(point_2d const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Less-than operator.
 *
 *  @param[in] right Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool point_2d::operator<(point_2d const& right) const throw() {
  if (_x != right._x)
    return (_x < right._x);
  return (_y < right._y);
}

/**
 *  Get the x coordinates.
 *
 *  @return The x coordinates.
 */
int point_2d::x() const throw() {
  return (_x);
}

/**
 *  Get the y coordinates.
 *
 *  @return The y coordinates.
 */
int point_2d::y() const throw() {
  return (_y);
}
