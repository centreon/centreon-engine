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
#  define CCE_DOWNTIMES_DOWTIME_HH

#  include <sstream>
//#  include <time.h>
#  include "com/centreon/engine/host.hh"
#  include "com/centreon/engine/objects/service.hh"

CCE_BEGIN()

namespace                      downtimes {
class                          downtime {
 public:
                               downtime(int type, std::string const& host_name);
                               downtime(downtime const& other);
                               downtime(downtime&& other);
  virtual                      ~downtime();

  int                          get_type() const;
  time_t                       entry_time;
  time_t                       start_time;
  time_t                       end_time;
  int                          fixed;
  unsigned long                triggered_by;
  unsigned long                duration;
  unsigned long                downtime_id;
  char*                        author;
  char*                        comment;
  unsigned long                comment_id;
  int                          is_in_effect;
  int                          start_flex_downtime;
  int                          incremented_pending_downtime;
  virtual bool                 is_stale() const = 0;
  virtual int                  unschedule() = 0;
  virtual int                  subscribe() = 0;
  virtual int                  handle() = 0;
  std::string const&           get_hostname() const;
  virtual void                 print(std::ostream& os) const = 0;
  virtual void                 retention(std::ostream& os) const = 0;

 private:
  int                          _type;

 protected:
  std::string                  _hostname;
};
}

CCE_END()

int                 handle_scheduled_downtime_by_id(
                      unsigned long downtime_id);
int                 register_downtime(
                      int type,
                      unsigned long downtime_id);

std::ostream& operator<<(std::ostream& os, com::centreon::engine::downtimes::downtime const& dt);

#endif // !CCE_DOWNTIMES_DOWTIME_HH
