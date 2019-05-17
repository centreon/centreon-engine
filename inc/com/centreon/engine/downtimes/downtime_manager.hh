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

#ifndef CCE_DOWNTIMES_DOWTIME_MANAGER_HH
#  define CCE_DOWNTIMES_DOWTIME_MANAGER_HH

#include <map>
#include "com/centreon/engine/downtimes/downtime.hh"

CCE_BEGIN()

namespace                           downtimes {
class                               downtime_manager {
 public:
  static downtime_manager&          instance() {
    static downtime_manager instance;
    return instance;
  }
  std::multimap<time_t, std::shared_ptr<downtime>> const&
                                    get_scheduled_downtimes() const;

  void                              delete_downtime(int type, unsigned long downtime_id);
  int                               unschedule_downtime(int type, unsigned long downtime_id);
  std::shared_ptr<downtime>         find_downtime(int type, unsigned long downtime_id);
  int                               check_pending_flex_host_downtime(host* hst);
  int                               check_pending_flex_service_downtime(service* svc);
  void                              clear_scheduled_downtimes();
  int                               check_for_expired_downtime();
  int                               delete_downtime_by_hostname_service_description_start_time_comment(
                                      std::string const& hostname,
                                      char const* service_description,
                                      time_t start_time,
                                      char const* comment);
  void                              insert_downtime(std::shared_ptr<downtime> dt);
  int                               delete_host_downtime(unsigned long downtime_id);
  int                               delete_service_downtime(unsigned long downtime_id);
  void                              initialize_downtime_data();
  int                               xdddefault_validate_downtime_data();
  unsigned long                     get_next_downtime_id();
  std::shared_ptr<downtime>         add_host_downtime(
                                      std::string const& host_name,
                                      time_t             entry_time,
                                      char const*        author,
                                      char const*        comment_data,
                                      time_t             start_time,
                                      time_t             end_time,
                                      bool               fixed,
                                      unsigned long      triggered_by,
                                      unsigned long      duration,
                                      unsigned long      downtime_id);
  std::shared_ptr<downtime>         add_new_host_downtime(std::string const& host_name,
                                      time_t             entry_time,
                                      char const*        author,
                                      char const*        comment_data,
                                      time_t             start_time,
                                      time_t             end_time,
                                      bool               fixed,
                                      unsigned long      triggered_by,
                                      unsigned long      duration,
                                      unsigned long*     downtime_id);
  std::shared_ptr<downtime>         add_service_downtime(
                                      std::string const& host_name,
                                      std::string const& svc_description,
                                      time_t             entry_time,
                                      char const*        author,
                                      char const*        comment_data,
                                      time_t             start_time,
                                      time_t             end_time,
                                      bool               fixed,
                                      unsigned long      triggered_by,
                                      unsigned long      duration,
                                      unsigned long      downtime_id);
  std::shared_ptr<downtime>         add_new_service_downtime(
                                      std::string const& host_name,
                                      std::string const& service_description,
                                      time_t entry_time,
                                      char const* author,
                                      char const* comment_data,
                                      time_t start_time,
                                      time_t end_time,
                                      bool fixed,
                                      unsigned long triggered_by,
                                      unsigned long duration,
                                      unsigned long* downtime_id);
  int                               schedule_downtime(
                                      int                type,
                                      std::string const& host_name,
                                      std::string const& service_description,
                                      time_t             entry_time,
                                      char const*        author,
                                      char const*        comment_data,
                                      time_t             start_time,
                                      time_t             end_time,
                                      bool               fixed,
                                      unsigned long      triggered_by,
                                      unsigned long      duration,
                                      unsigned long*     new_downtime_id);

 private:
                                    downtime_manager() = default;
  std::multimap<time_t, std::shared_ptr<downtime>>
                                    _scheduled_downtimes;
  unsigned long                     _next_id;
};
}

CCE_END()


#endif // !CCE_DOWNTIMES_DOWNTIME_MANAGER_HH
