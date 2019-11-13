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

#include "com/centreon/engine/configuration/point_3d.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Constructor.
 *
 *  @param[in] x The x coordinates.
 *  @param[in] y The y coordinates.
 *  @param[in] z The z coordinates.
 */
point_3d::point_3d(double x, double y, double z) : _x(x), _y(y), _z(z) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
point_3d::point_3d(point_3d const& right) {
  operator=(right);
}

/**
 *  Destructor.
 */
point_3d::~point_3d() throw() {}

/**
 *  Copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
point_3d& point_3d::operator=(point_3d const& right) {
  if (this != &right) {
    _x = right._x;
    _y = right._y;
    _z = right._z;
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
bool point_3d::operator==(point_3d const& right) const throw() {
  return (_x == right._x && _y == right._y && _z == right._z);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The opbject to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool point_3d::operator!=(point_3d const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Less-than operator.
 *
 *  @param[in] right Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool point_3d::operator<(point_3d const& right) const throw() {
  if (_x != right._x)
    return (_x < right._x);
  else if (_y != right._y)
    return (_y < right._y);
  return (_z < right._z);
}

/**
 *  Get the x coordinates.
 *
 *  @return The x coordinates.
 */
double point_3d::x() const throw() {
  return (_x);
}

/**
 *  Get the y coordinates.
 *
 *  @return The y coordinates.
 */
double point_3d::y() const throw() {
  return (_y);
}

/**
 *  Get the z coordinates.
 *
 *  @return The z coordinates.
 */
double point_3d::z() const throw() {
  return (_z);
}
