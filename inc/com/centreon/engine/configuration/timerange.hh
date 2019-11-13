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

#ifndef CCE_CONFIGURATION_TIMERANGE_HH
#define CCE_CONFIGURATION_TIMERANGE_HH

#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace configuration {
class timerange {
 public:
  timerange(unsigned long start = 0, unsigned long end = 0);
  timerange(timerange const& right);
  ~timerange() throw();
  timerange& operator=(timerange const& right);
  bool operator==(timerange const& right) const throw();
  bool operator!=(timerange const& right) const throw();
  bool operator<(timerange const& right) const throw();
  unsigned long end() const throw();
  void end(unsigned long value);
  unsigned long start() const throw();
  void start(unsigned long value);

 private:
  unsigned long _end;
  unsigned long _start;
};
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_TIMERANGE_HH
