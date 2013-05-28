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

#  include "com/centreon/engine/configuration/applier/base.hh"
#  include "com/centreon/engine/configuration/timeperiod.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                configuration {
  namespace              applier {
    class                timeperiod
      : public base {
    public:
      void               apply(state const& config);
      static timeperiod& instance();
      static void        load();
      static void        unload();

    private:
                         timeperiod();
                         timeperiod(timeperiod const&);
                         ~timeperiod() throw ();
      timeperiod&        operator=(timeperiod const&);
      void               _add_timeperiods(map_timeperiod const& data);
      void               _modify_timeperiods(map_timeperiod const& data);
      void               _remove_timeperiods(map_timeperiod const& data);
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_TIMEPERIOD_HH
