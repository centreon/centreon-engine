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

#ifndef CCE_CONFIGURATION_POINT_2D_HH
#define CCE_CONFIGURATION_POINT_2D_HH

#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace configuration {
class point_2d {
 public:
  point_2d(int x = -1, int y = -1);
  point_2d(point_2d const& right);
  ~point_2d() throw();
  point_2d& operator=(point_2d const& right);
  bool operator==(point_2d const& right) const throw();
  bool operator!=(point_2d const& right) const throw();
  bool operator<(point_2d const& right) const throw();
  int x() const throw();
  int y() const throw();

 private:
  int _x;
  int _y;
};
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_POINT_2D_HH
