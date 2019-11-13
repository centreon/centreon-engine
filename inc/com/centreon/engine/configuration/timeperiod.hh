/*
** Copyright 2011-2013,2017 Centreon
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
#define CCE_CONFIGURATION_TIMEPERIOD_HH

#include <list>
#include <set>
#include <string>
#include <vector>
#include "com/centreon/engine/configuration/daterange.hh"
#include "com/centreon/engine/configuration/group.hh"
#include "com/centreon/engine/configuration/object.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/opt.hh"

CCE_BEGIN()

namespace configuration {
class timeperiod : public object {
 public:
  typedef std::string key_type;

  timeperiod(key_type const& key = "");
  timeperiod(timeperiod const& right);
  ~timeperiod() throw() override;
  timeperiod& operator=(timeperiod const& right);
  bool operator==(timeperiod const& right) const throw();
  bool operator!=(timeperiod const& right) const throw();
  bool operator<(timeperiod const& right) const throw();
  void check_validity() const override;
  key_type const& key() const throw();
  void merge(object const& obj) override;
  bool parse(char const* key, char const* value) override;
  bool parse(std::string const& line) override;

  std::string const& alias() const throw();
  std::vector<std::list<daterange> > const& exceptions() const throw();
  set_string const& exclude() const throw();
  std::string const& timeperiod_name() const throw();
  std::vector<std::list<timerange> > const& timeranges() const throw();

 private:
  typedef bool (*setter_func)(timeperiod&, char const*);

  bool _add_calendar_date(std::string const& line);
  bool _add_other_date(std::string const& line);
  bool _add_week_day(std::string const& key, std::string const& value);
  static bool _build_timeranges(std::string const& line,
                                std::list<timerange>& timeranges);
  static bool _build_time_t(std::string const& time_str, unsigned long& ret);
  static bool _has_similar_daterange(std::list<daterange> const& lst,
                                     daterange const& range) throw();
  static bool _get_month_id(std::string const& name, unsigned int& id);
  static bool _get_day_id(std::string const& name, unsigned int& id);
  bool _set_alias(std::string const& value);
  bool _set_exclude(std::string const& value);
  bool _set_timeperiod_name(std::string const& value);

  std::string _alias;
  static std::unordered_map<std::string, setter_func> const _setters;
  std::vector<std::list<daterange> > _exceptions;
  group<set_string> _exclude;
  std::string _timeperiod_name;
  std::vector<std::list<timerange> > _timeranges;
};

typedef std::shared_ptr<timeperiod> timeperiod_ptr;
typedef std::set<timeperiod> set_timeperiod;
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_TIMEPERIOD_HH
