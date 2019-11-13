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

#ifndef CCE_CONFIGURATION_POINT_3D_HH
#define CCE_CONFIGURATION_POINT_3D_HH

#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace configuration {
class point_3d {
 public:
  point_3d(double x = 0.0, double y = 0.0, double z = 0.0);
  point_3d(point_3d const& right);
  ~point_3d() throw();
  point_3d& operator=(point_3d const& right);
  bool operator==(point_3d const& right) const throw();
  bool operator!=(point_3d const& right) const throw();
  bool operator<(point_3d const& right) const throw();
  double x() const throw();
  double y() const throw();
  double z() const throw();

 private:
  double _x;
  double _y;
  double _z;
};
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_POINT_3D_HH
