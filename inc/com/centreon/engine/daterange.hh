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

#ifndef CCE_OBJECTS_DATERANGE_HH
#define CCE_OBJECTS_DATERANGE_HH
#include <ostream>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/timerange.hh"

struct timeperiod_struct;

CCE_BEGIN()
class daterange;
CCE_END()

using daterange_list = std::list<com::centreon::engine::daterange>;

using exception_array = std::array<daterange_list, DATERANGE_TYPES>;

CCE_BEGIN()

class daterange {
 public:
  enum type_range {
    none = -1,
    calendar_date = 0,
    month_date = 1,
    month_day = 2,
    month_week_day = 3,
    week_day = 4
  };

  daterange(type_range type,
            int syear,
            int smon,
            int smday,
            int swday,
            int swday_offset,
            int eyear,
            int emon,
            int emday,
            int ewday,
            int ewday_offset,
            int skip_interval);
  daterange(type_range type);

  type_range get_type() const { return _type; }
  void set_type(type_range type) { _type = type; }
  int get_syear() const { return _syear; }
  void set_syear(int syear) { _syear = syear; }
  int get_smon() const { return _smon; }
  void set_smon(int smon) { _smon = smon; }
  int get_smday() const { return _smday; }
  void set_smday(int smday) { _smday = smday; }
  int get_swday() const { return _swday; }
  void set_swday(int swday) { _swday = swday; }
  int get_swday_offset() const { return _swday_offset; }
  void set_swday_offset(int swday_offset) { _swday_offset = swday_offset; }
  int get_eyear() const { return _eyear; }
  void set_eyear(int eyear) { _eyear = eyear; }
  int get_emon() const { return _emon; }
  void set_emon(int emon) { _emon = emon; }
  int get_emday() const { return _emday; }
  void set_emday(int emday) { _emday = emday; }
  int get_ewday() const { return _ewday; }
  void set_ewday(int ewday) { _ewday = ewday; }
  int get_ewday_offset() const { return _ewday_offset; }
  void set_ewday_offset(int ewday_offset) { _ewday_offset = ewday_offset; }
  int get_skip_interval() const { return _skip_interval; }
  void set_skip_interval(int skip_interval) { _skip_interval = skip_interval; }
  const timerange_list& get_timerange() const { return _timerange; }
  void set_timerange(const timerange_list& timerange) {
    _timerange = timerange;
  }
  void add_timerange(const timerange& toadd) { _timerange.push_back(toadd); }

  bool operator==(daterange const& obj) const;
  bool operator!=(daterange const& obj2) const;
  bool operator<(daterange const& right) const;
  bool is_date_data_equal(daterange const& obj) const;

  static std::string const& get_month_name(unsigned int index);
  static std::string const& get_weekday_name(unsigned int index);

 private:
  type_range _type;
  int _syear;  // Start year.
  int _smon;   // Start month.
  // Start day of month (may 3rd, last day in feb).
  int _smday;
  int _swday;  // Start day of week (thursday).
  // Start weekday offset (3rd thursday, last monday in jan).
  int _swday_offset;
  int _eyear;
  int _emon;
  int _emday;
  int _ewday;
  int _ewday_offset;
  int _skip_interval;
  timerange_list _timerange;
};

std::ostream& operator<<(std::ostream& os,
                         com::centreon::engine::daterange const& obj);

std::ostream& operator<<(std::ostream& os, exception_array const& obj);

CCE_END()

#endif  // !CCE_OBJECTS_DATERANGE_HH
