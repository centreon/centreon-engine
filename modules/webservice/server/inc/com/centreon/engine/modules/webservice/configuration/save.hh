/*
** Copyright 2012 Merethis
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

#ifndef CCE_MOD_WS_CONFIGURATION_SAVE_HH
#  define CCE_MOD_WS_CONFIGURATION_SAVE_HH

#include <iostream>

#  include <sstream>
#  include <string>
#  include "com/centreon/engine/objects.hh"
#  include "com/centreon/engine/modules/webservice/namespace.hh"

CCE_MOD_WS_BEGIN()

namespace              configuration {
  class                save {
  public:
                       save();
                       save(save const& right);
                       ~save() throw ();
    save&              operator=(save const& right);
    save&              operator<<(command const& obj);
    save&              operator<<(contact const& obj);
    save&              operator<<(contactgroup const& obj);
    save&              operator<<(host const& obj);
    save&              operator<<(hostdependency const& obj);
    save&              operator<<(hostescalation const& obj);
    save&              operator<<(hostgroup const& obj);
    save&              operator<<(service const& obj);
    save&              operator<<(servicedependency const& obj);
    save&              operator<<(serviceescalation const& obj);
    save&              operator<<(servicegroup const& obj);
    save&              operator<<(timeperiod const& obj);
    template<typename T>
    void               add_list(T const* lst) {
      for (T const* obj(lst); obj; obj = obj->next) {
        *this << *obj;
        _stream << "\n";
      }
    }
    void               backup(std::string const& filename) const;
    void               clear();
    std::string        to_string() const;

  private:
    void               _add_commands(
                         char const* key,
                         commandsmember const* obj);
    void               _add_contacts(
                         char const* key,
                         contactsmember const* obj);
    void               _add_contactgroups(
                         char const* key,
                         contactgroupsmember const* obj);
    void               _add_customvariables(
                         customvariablesmember const* obj);
    void               _add_daterange(daterange const* obj);
    void               _add_hosts(
                         char const* key,
                         hostsmember const* obj);
    template<typename T>
    void               _add_line(char const* key, T value);
    void               _add_string(char const* key, char const* value);
    void               _add_string(
                         char const* key,
                         std::string const& value);
    void               _add_services(
                         char const* key,
                         servicesmember const* obj);
    void               _add_timeperiodexclusion(
                         char const* key,
                         timeperiodexclusion const* obj);
    void               _add_timerange(
                         char const* key,
                         timerange const* obj);
    static std::string _build_daterange_calendar_date(
                         daterange const* obj);
    static std::string _build_daterange_month_date(
                         daterange const* obj);
    static std::string _build_daterange_month_day(daterange const* obj);
    static std::string _build_daterange_month_week_day(
                         daterange const* obj);
    static std::string _build_daterange_week_day(daterange const* obj);
    static char const* _get_month(unsigned int index);
    static char const* _get_weekday(unsigned int index);
    save&              _internal_copy(save const& right);

    std::ostringstream _stream;
  };
}

CCE_MOD_WS_END()

#endif // !CCE_MOD_WS_CONFIGURATION_SAVE_HH
