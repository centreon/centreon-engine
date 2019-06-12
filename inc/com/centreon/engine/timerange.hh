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

#ifndef CCE_OBJECTS_TIMERANGE_HH
#  define CCE_OBJECTS_TIMERANGE_HH

#include <list>
#include <memory>
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()
class timerange;
CCE_END()

typedef std::list<std::shared_ptr<com::centreon::engine::timerange> >
  timerange_list;

CCE_BEGIN()
class                   timerange {
 public:
                        timerange(uint64_t start, uint64_t end);
  uint64_t              get_range_start() const;
  uint64_t              get_range_end() const;
  //static timerange_list timeranges;

  bool                  operator==(timerange const& obj) throw ();
  bool                  operator!=(timerange const& obj) throw ();

 private:
  uint64_t              _range_start;
  uint64_t              _range_end;
};
CCE_END()

std::ostream&         operator<<(std::ostream& os,
                                 com::centreon::engine::timerange const& obj);
std::ostream&         operator<<(std::ostream& os, timerange_list const& obj);

#endif // !CCE_OBJECTS_TIMERANGE_HH


