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
#  define CCE_RETENTION_DOWNTIME_HH

#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/retention/object.hh"

CCE_BEGIN()

namespace         retention {
  class           downtime
    : public object {
  public:
    enum          type_id {
      host = 0,
      service = 1
    };

                  downtime(type_id type);
                  ~downtime() throw ();
    bool          set(
                    std::string const& key,
                    std::string const& value);

  private:
    void          _add_host_downtime() throw ();
    void          _add_service_downtime() throw ();
    void          _finished() throw ();

    std::string   _author;
    std::string   _comment_data;
    unsigned long _downtime_id;
    type_id       _downtime_type;
    unsigned long _duration;
    time_t        _end_time;
    time_t        _entry_time;
    bool          _fixed;
    std::string   _host_name;
    std::string   _service_description;
    time_t        _start_time;
    unsigned long _triggered_by;
  };
}

CCE_END()

#endif // !CCE_RETENTION_DOWNTIME_HH
