/*
** Copyright 2000-2008      Ethan Galstad
** Copyright 2011-2013,2016 Centreon
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

#ifndef CCE_DOWNTIMES_DOWTIME_HH
#define CCE_DOWNTIMES_DOWTIME_HH

#include <sstream>
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/service.hh"

CCE_BEGIN()

namespace downtimes {
class downtime {
 public:
  downtime(int type,
           std::string const& host_name,
           time_t entry_time,
           std::string const& author,
           std::string const& comment,
           time_t start_time,
           time_t end_time,
           bool fixed,
           uint64_t triggered_by,
           int32_t duration,
           uint64_t downtime_id);
  downtime(downtime const& other) = delete;
  downtime(downtime&& other) = delete;
  virtual ~downtime();

  int get_type() const;
  virtual bool is_stale() const = 0;
  virtual void schedule() = 0;
  virtual int unschedule() = 0;
  virtual int subscribe() = 0;
  virtual int handle() = 0;
  std::string const& get_hostname() const;
  virtual void print(std::ostream& os) const = 0;
  virtual void retention(std::ostream& os) const = 0;
  std::string const& get_author() const;
  std::string const& get_comment() const;
  uint64_t get_downtime_id() const;
  uint64_t get_triggered_by() const;
  bool is_fixed() const;
  time_t get_entry_time() const;
  time_t get_start_time() const;
  time_t get_end_time() const;
  int32_t get_duration() const;
  bool is_in_effect() const;
  void start_flex_downtime();

 private:
  int _type;

 protected:
  void _set_in_effect(bool in_effect);
  uint64_t _get_comment_id() const;

  std::string _hostname;
  time_t _entry_time;
  std::string _author;
  std::string _comment;
  time_t _start_time;
  time_t _end_time;
  bool _fixed;
  uint64_t _triggered_by;
  int32_t _duration;
  uint64_t _downtime_id;
  bool _in_effect;
  uint64_t _comment_id;
  int _start_flex_downtime;
  bool _incremented_pending_downtime;
};
}  // namespace downtimes

CCE_END()

int handle_scheduled_downtime_by_id(uint64_t downtime_id);

std::ostream& operator<<(std::ostream& os,
                         com::centreon::engine::downtimes::downtime const& dt);

#endif  // !CCE_DOWNTIMES_DOWTIME_HH
