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

#ifndef CCE_DOWNTIMES_SERVICE_DOWTIME_HH
#  define CCE_DOWNTIMES_SERVICE_DOWTIME_HH

#  include "com/centreon/engine/downtimes/downtime.hh"

CCE_BEGIN()

namespace                           downtimes {
class                               service_downtime : public downtime {
 public:
                                    service_downtime(std::string const& hostname, std::string const& service_desc);
                                    service_downtime(downtime const& other);
                                    service_downtime(downtime&& other);
  virtual                           ~service_downtime();
  void                              set_service_description(std::string const& descr);
  std::string const&                get_service_description() const;
  virtual bool                      is_stale() const override;
  virtual int                       unschedule() override;
  virtual int                       subscribe() override;
  virtual int                       handle() override;
  virtual void                      print(std::ostream& os) const override;
  virtual void                      retention(std::ostream& os) const override;

 private:
  std::string                       _service_description;
};
}

CCE_END()

int                 add_service_downtime(
                      std::string const& host_name,
                      std::string const& svc_description,
                      time_t entry_time,
                      char const* author,
                      char const* comment_data,
                      time_t start_time,
                      time_t end_time,
                      int fixed,
                      unsigned long triggered_by,
                      unsigned long duration,
                      unsigned long downtime_id);
int                 add_new_service_downtime(
                      std::string const& host_name,
                      std::string const& service_description,
                      time_t entry_time,
                      char const* author,
                      char const* comment_data,
                      time_t start_time,
                      time_t end_time,
                      int fixed,
                      unsigned long triggered_by,
                      unsigned long duration,
                      unsigned long* downtime_id);
int                 delete_service_downtime(
                      unsigned long downtime_id);
int                 check_pending_flex_service_downtime(service* svc);

com::centreon::engine::downtimes::downtime* find_service_downtime(unsigned long downtime_id);

#endif // !CCE_DOWNTIMES_SERVICE_DOWTIME_HH
