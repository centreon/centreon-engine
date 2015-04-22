/*
** Copyright 2015 Merethis
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

#ifndef CCE_CONFIGURATION_DURATION_HH
#  define CCE_CONFIGURATION_DURATION_HH

#  include <string>
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace     configuration {

/**
 *  @class duration duration.hh "com/centreon/engine/configuration/duration.hh"
 *  @brief Parse a duration.
 *
 *  Parse a duration from a configuration file. Durations are expressed
 *  as a number eventually followed by a suffix. Supported suffixes are
 *  "s" for seconds, "m" for minutes, "h" for hours and "d" for days.
 */
  class       duration {
  public:
              duration();
              duration(duration const& other);
              duration(long value);
              duration(std::string const& value);
              ~duration();
    duration& operator=(duration const& other);
    duration& operator=(long value);
    duration& operator=(std::string const& value);
              operator long() const;
    long      get() const;
    void      set(long value);
    void      set(std::string const& value);

  private:
    long      _value;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_DURATION_HH
