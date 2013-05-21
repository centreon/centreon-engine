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

#ifndef CCE_CONFIGURATION_TIMEPERIOD_HH
#  define CCE_CONFIGURATION_TIMEPERIOD_HH

#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    timeperiod
    : public object {
  public:
                           timeperiod();
                           timeperiod(timeperiod const& right);
                           ~timeperiod() throw ();
    timeperiod&            operator=(timeperiod const& right);
    bool                   operator==(
                             timeperiod const& right) const throw ();
    bool                   operator!=(
                             timeperiod const& right) const throw ();
    /*
    std::string const&     alias() const throw ();
    std::list<std::string> const&
                           exclude() const throw ();
    std::string const&     timeperiod_name() const throw ();
    */
    void                   merge(object const& obj);
    bool                   parse(
                             std::string const& key,
                             std::string const& value);
    bool                   parse(std::string const& line);

  private:
    bool                   _set_alias(std::string const& value);
    bool                   _set_exclude(std::string const& value);
    bool                   _set_timeperiod_name(std::string const& value);

    std::string            _alias;
    std::list<std::string> _exclude;
    std::string            _timeperiod_name;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_TIMEPERIOD_HH


