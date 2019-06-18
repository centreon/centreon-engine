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

#ifndef CCE_OBJECTS_TIMEPERIOD_HH
#  define CCE_OBJECTS_TIMEPERIOD_HH

#  include <ostream>
#  include <string>
#  include <unordered_map>
#  include "com/centreon/engine/common.hh"
#  include "com/centreon/engine/daterange.hh"
#  include "com/centreon/engine/timerange.hh"
#  include "com/centreon/engine/namespace.hh"



/* Forward declaration. */
CCE_BEGIN()
class timeperiod;
CCE_END()

typedef std::unordered_map<std::string,
  std::shared_ptr<com::centreon::engine::timeperiod>> timeperiod_map;
typedef std::unordered_multimap<std::string,
  std::shared_ptr<com::centreon::engine::timeperiod>> timeperiodexclusion;

CCE_BEGIN()

class                timeperiod {
 public:
                      timeperiod(std::string const& name, std::string const& alias);

  std::string const&  get_name() const;
  void                set_name(std::string const& name);
  std::string const   get_alias() const;
  void                set_alias(std::string const& alias);

  bool                operator==(timeperiod const& obj) throw ();
  bool                operator!=(timeperiod const& obj) throw ();

  timerange_list      days[7];
  daterange_list      exceptions[DATERANGE_TYPES];
  timeperiodexclusion exclusions;

  static timeperiod_map
                      timeperiods;

 private:
  std::string         _name;
  std::string         _alias;

};

CCE_END()

bool check_time_against_period(
  time_t test_time,
  com::centreon::engine::timeperiod* tperiod);
void get_next_valid_time(
  time_t pref_time,
  time_t* valid_time,
  com::centreon::engine::timeperiod *tperiod);

std::ostream& operator<<(std::ostream& os, com::centreon::engine::timeperiod const& obj);
std::ostream& operator<<(std::ostream& os, timeperiodexclusion const& obj);

#endif // !CCE_OBJECTS_TIMEPERIOD_HH


