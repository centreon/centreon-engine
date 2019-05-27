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
#  define CCE_OBJECTS_DATERANGE_HH
#    include <ostream>
#    include "com/centreon/engine/timerange.hh"

struct timeperiod_struct;

CCE_BEGIN()
class daterange;
CCE_END()

typedef std::list<std::shared_ptr<com::centreon::engine::daterange>>
  daterange_list;

CCE_BEGIN()

class            daterange {
 public:
                 daterange(
                   int type,
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

  int            get_type() const;
  void           set_type(int type);
  int            get_syear() const;
  void           set_syear(int syear);
  int            get_smon() const;
  void           set_smon(int smon);
  int            get_smday() const;
  void           set_smday(int smday);
  int            get_swday() const;
  void           set_swday(int swday);
  int            get_swday_offset() const;
  void           set_swday_offset(int swday_offset);
  int            get_eyear() const;
  void           set_eyear(int eyear);
  int            get_emon() const;
  void           set_emon(int emon);
  int            get_emday() const;
  void           set_emday(int emday);
  int            get_ewday() const;
  void           set_ewday(int ewday);
  int            get_ewday_offset() const;
  void           set_ewday_offset(int ewday_offset);
  int            get_skip_interval() const;
  void           set_skip_interval(int skip_interval);

  bool           operator==(daterange const& obj) throw ();
  bool           operator!=(daterange const& obj2) throw ();

  timerange_list times;

  static std::string const&
                 get_month_name(unsigned int index);
  static std::string const&
                 get_weekday_name(unsigned int index);

 private:
  int            _type;
  int            _syear;        // Start year.
  int            _smon;         // Start month.
  // Start day of month (may 3rd, last day in feb).
  int            _smday;
  int            _swday;        // Start day of week (thursday).
  // Start weekday offset (3rd thursday, last monday in jan).
  int            _swday_offset;
  int            _eyear;
  int            _emon;
  int            _emday;
  int            _ewday;
  int            _ewday_offset;
  int            _skip_interval;
};

CCE_END()

std::ostream&      operator<<(std::ostream& os, com::centreon::engine::daterange const& obj);

#endif // !CCE_OBJECTS_DATERANGE_HH