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

#ifndef CCE_CONFIGURATION_APPLIER_TIMEPERIOD_HH
#  define CCE_CONFIGURATION_APPLIER_TIMEPERIOD_HH

#  include <list>
#  include <string>
#  include <vector>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

// Forward declaration.
struct            timeperiod_struct;

CCE_BEGIN()

namespace         configuration {
  // Forward declarations.
  class           daterange;
  class           state;
  class           timeperiod;
  class           timerange;

  namespace       applier {
    class         timeperiod {
    public:
                  timeperiod();
                  timeperiod(timeperiod const& right);
                  ~timeperiod() throw ();
      timeperiod& operator=(timeperiod const& right);
      void        add_object(
                    shared_ptr<configuration::timeperiod> obj);
      void        expand_object(
                    shared_ptr<configuration::timeperiod> obj,
                    configuration::state& s);
      void        modify_object(
                    shared_ptr<configuration::timeperiod> obj);
      void        remove_object(
                    shared_ptr<configuration::timeperiod> obj);
      void        resolve_object(
                    shared_ptr<configuration::timeperiod> obj);

    private:
      void        _add_exclusions(
                    std::list<std::string> const& exclusions,
                    timeperiod_struct* tp);
      void        _add_exceptions(
                    std::vector<std::list<daterange> > const& exceptions,
                    timeperiod_struct* tp);
      void        _add_time_ranges(
                    std::vector<std::list<timerange> > const& ranges,
                    timeperiod_struct* tp);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_TIMEPERIOD_HH
