/*
** Copyright 2019 Centreon
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

#ifndef CCE_DOWNTIMES_HOST_DOWTIME_HH
#  define CCE_DOWNTIMES_HOST_DOWTIME_HH

#  include "com/centreon/engine/downtimes/downtime.hh"

CCE_BEGIN()

namespace                           downtimes {
class                               host_downtime : public downtime {
 public:
                                    host_downtime(
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
  virtual                           ~host_downtime();

  virtual bool                      is_stale() const override;
  virtual void                      schedule() override;
  virtual int                       unschedule() override;
  virtual int                       subscribe() override;
  virtual int                       handle() override;
  virtual void                      print(std::ostream& os) const override;
  virtual void                      retention(std::ostream& os) const override;
};
}

CCE_END()

#endif // !CCE_DOWNTIMES_HOST_DOWTIME_HH
