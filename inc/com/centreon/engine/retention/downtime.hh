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

#ifndef CCE_RETENTION_DOWNTIME_HH
#define CCE_RETENTION_DOWNTIME_HH

#include <list>
#include <string>
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/retention/object.hh"

CCE_BEGIN()

namespace retention {
class downtime : public object {
 public:
  enum type_id { host = 0, service = 1 };

  downtime(type_id type);
  downtime(downtime const& right);
  ~downtime() throw() override;
  downtime& operator=(downtime const& right);
  bool operator==(downtime const& right) const throw();
  bool operator!=(downtime const& right) const throw();
  bool set(char const* key, char const* value) override;

  std::string author() const throw();
  std::string comment_data() const throw();
  unsigned long downtime_id() const throw();
  type_id downtime_type() const throw();
  unsigned long duration() const throw();
  time_t end_time() const throw();
  time_t entry_time() const throw();
  bool fixed() const throw();
  std::string host_name() const throw();
  std::string service_description() const throw();
  time_t start_time() const throw();
  unsigned long triggered_by() const throw();

 private:
  struct setters {
    char const* name;
    bool (*func)(retention::downtime&, char const*);
  };

  bool _set_author(std::string const& value);
  bool _set_comment_data(std::string const& value);
  bool _set_downtime_id(unsigned long value);
  bool _set_duration(unsigned long value);
  bool _set_end_time(time_t value);
  bool _set_entry_time(time_t value);
  bool _set_fixed(bool value);
  bool _set_host_name(std::string const& value);
  bool _set_service_description(std::string const& value);
  bool _set_start_time(time_t value);
  bool _set_triggered_by(unsigned long value);

  std::string _author;
  std::string _comment_data;
  unsigned long _downtime_id;
  type_id _downtime_type;
  unsigned long _duration;
  time_t _end_time;
  time_t _entry_time;
  bool _fixed;
  std::string _host_name;
  std::string _service_description;
  static setters const _setters[];
  time_t _start_time;
  unsigned long _triggered_by;
};

typedef std::shared_ptr<downtime> downtime_ptr;
typedef std::list<downtime_ptr> list_downtime;
}  // namespace retention

CCE_END()

#endif  // !CCE_RETENTION_DOWNTIME_HH
