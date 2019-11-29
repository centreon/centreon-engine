/*
** Copyright 2007-2008 Ethan Galstad
** Copyright 2007,2010 Andreas Ericsson
** Copyright 2010      Max Schubert
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

#include "com/centreon/engine/events/sched_info.hh"
#include <cmath>
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;


/**
 *  Displays service check scheduling information.
 */
void display_scheduling_info() {
  // Notice.
  logger(log_info_message, basic)
      << "\nProjected scheduling information for host and service checks\n"
      << "is listed below.  This information assumes that you are going\n"
      << "to start running Centreon Engine with your current config files.\n\n";

  // Host scheduling information.
  logger(log_info_message, basic)
      << "HOST SCHEDULING INFORMATION\n"
      << "---------------------------\n"
      << "Total hosts:                        " << scheduling_info.total_hosts
      << "\n"
      << "Total scheduled hosts:              "
      << scheduling_info.total_scheduled_hosts << "\n";
  if (config->host_inter_check_delay_method() == configuration::state::icd_none)
    logger(log_info_message, basic)
        << "Host inter-check delay method:      NONE\n";
  else if (config->host_inter_check_delay_method() ==
           configuration::state::icd_dumb)
    logger(log_info_message, basic)
        << "Host inter-check delay method:      DUMB\n";
  else if (config->host_inter_check_delay_method() ==
           configuration::state::icd_smart)
    logger(log_info_message, basic)
        << "Host inter-check delay method:      SMART\n"
        << "Average host check interval:        "
        << scheduling_info.average_host_check_interval << " sec\n";
  else
    logger(log_info_message, basic)
        << "Host inter-check delay method:      USER-SUPPLIED VALUE\n";
  logger(log_info_message, basic)
      << "Host inter-check delay:             "
      << scheduling_info.host_inter_check_delay << " sec\n"
      << "Max host check spread:              "
      << scheduling_info.max_host_check_spread << " min\n"
      << "First scheduled check:              "
      << ((scheduling_info.total_scheduled_hosts == 0)
              ? "N/A\n"
              : ctime(&scheduling_info.first_host_check))
      << "Last scheduled check:               "
      << ((scheduling_info.total_scheduled_hosts == 0)
              ? "N/A\n"
              : ctime(&scheduling_info.last_host_check))
      << "\n";

  // Service scheduling information.
  logger(log_info_message, basic)
      << "SERVICE SCHEDULING INFORMATION\n"
      << "-------------------------------\n"
      << "Total services:                     "
      << scheduling_info.total_services << "\n"
      << "Total scheduled services:           "
      << scheduling_info.total_scheduled_services << "\n";
  if (config->service_inter_check_delay_method() ==
      configuration::state::icd_none)
    logger(log_info_message, basic)
        << "Service inter-check delay method:   NONE\n";
  else if (config->service_inter_check_delay_method() ==
           configuration::state::icd_dumb)
    logger(log_info_message, basic)
        << "Service inter-check delay method:   DUMB\n";
  else if (config->service_inter_check_delay_method() ==
           configuration::state::icd_smart) {
    logger(log_info_message, basic)
        << "Service inter-check delay method:   SMART\n"
        << "Average service check interval:     "
        << scheduling_info.average_service_check_interval << " sec\n";
  } else
    logger(log_info_message, basic)
        << "Service inter-check delay method:   USER-SUPPLIED VALUE\n";
  logger(log_info_message, basic)
      << "Inter-check delay:                  "
      << scheduling_info.service_inter_check_delay << " sec\n"
      << "Interleave factor method:           "
      << ((config->service_interleave_factor_method() ==
           configuration::state::ilf_user)
              ? "USER-SUPPLIED VALUE"
              : "SMART")
      << "\n";
  if (config->service_interleave_factor_method() ==
      configuration::state::ilf_smart)
    logger(log_info_message, basic)
        << "Average services per host:          "
        << scheduling_info.average_services_per_host << "\n";
  logger(log_info_message, basic)
      << "Service interleave factor:          "
      << scheduling_info.service_interleave_factor << "\n"
      << "Max service check spread:           "
      << scheduling_info.max_service_check_spread << " min\n"
      << "First scheduled check:              "
      << ctime(&scheduling_info.first_service_check)
      << "Last scheduled check:               "
      << ctime(&scheduling_info.last_service_check) << "\n";

  // Check processing information.
  logger(log_info_message, basic)
      << "CHECK PROCESSING INFORMATION\n"
      << "----------------------------\n"
      << "Check result reaper interval:       "
      << config->check_reaper_interval() << " sec\n";
  if (config->max_parallel_service_checks() == 0)
    logger(log_info_message, basic)
        << "Max concurrent service checks:      Unlimited\n";
  else
    logger(log_info_message, basic)
        << "Max concurrent service checks:      "
        << config->max_parallel_service_checks() << "\n";
  logger(log_info_message, basic) << "\n";

  // Performance suggestions.
  logger(log_info_message, basic) << "PERFORMANCE SUGGESTIONS\n"
                                  << "-----------------------\n";
  int suggestions(0);

  // MAX REAPER INTERVAL RECOMMENDATION.
  // Assume a 100% (2x) check burst for check reaper.
  // Assume we want a max of 2k files in the result queue
  // at any given time.
  float max_reaper_interval(0.0);
  max_reaper_interval = floor(2000 * scheduling_info.service_inter_check_delay);
  if (max_reaper_interval < 2.0)
    max_reaper_interval = 2.0;
  if (max_reaper_interval > 30.0)
    max_reaper_interval = 30.0;
  if (max_reaper_interval < config->check_reaper_interval()) {
    logger(log_info_message, basic)
        << "* Value for 'check_result_reaper_frequency' should be <= "
        << static_cast<int>(max_reaper_interval) << " seconds\n";
    ++suggestions;
  }
  if (config->check_reaper_interval() < 2) {
    logger(log_info_message, basic)
        << "* Value for 'check_result_reaper_frequency' should be >= 2 "
           "seconds\n";
    ++suggestions;
  }

  // MINIMUM CONCURRENT CHECKS RECOMMENDATION.
  // First method (old) - assume a 100% (2x) service check
  // burst for max concurrent checks.
  float minimum_concurrent_checks(0.0);
  float minimum_concurrent_checks1(0.0);
  float minimum_concurrent_checks2(0.0);
  if (scheduling_info.service_inter_check_delay == 0.0)
    minimum_concurrent_checks1 = ceil(config->check_reaper_interval() * 2.0);
  else
    minimum_concurrent_checks1 =
        ceil((config->check_reaper_interval() * 2.0) /
             scheduling_info.service_inter_check_delay);
  // Second method (new) - assume a 25% (1.25x) service check
  // burst for max concurrent checks.
  minimum_concurrent_checks2 =
      ceil((((double)scheduling_info.total_scheduled_services) /
            scheduling_info.average_service_check_interval) *
           1.25 * config->check_reaper_interval() *
           scheduling_info.average_service_execution_time);
  // Use max of computed values.
  if (minimum_concurrent_checks1 > minimum_concurrent_checks2)
    minimum_concurrent_checks = minimum_concurrent_checks1;
  else
    minimum_concurrent_checks = minimum_concurrent_checks2;
  // Compare with configured value.
  if ((minimum_concurrent_checks > config->max_parallel_service_checks()) &&
      config->max_parallel_service_checks() != 0) {
    logger(log_info_message, basic)
        << "* Value for 'max_concurrent_checks' option should be >= "
        << static_cast<int>(minimum_concurrent_checks) << "\n";
    ++suggestions;
  }
  if (suggestions == 0)
    logger(log_info_message, basic)
        << "I have no suggestions - things look okay.\n";

  return;
}

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(sched_info const& obj1, sched_info const& obj2) throw() {
  return (
      obj1.total_services == obj2.total_services &&
      obj1.total_scheduled_services == obj2.total_scheduled_services &&
      obj1.total_hosts == obj2.total_hosts &&
      obj1.total_scheduled_hosts == obj2.total_scheduled_hosts &&
      obj1.average_services_per_host == obj2.average_services_per_host &&
      obj1.average_scheduled_services_per_host ==
          obj2.average_scheduled_services_per_host &&
      obj1.service_check_interval_total == obj2.service_check_interval_total &&
      obj1.host_check_interval_total == obj2.host_check_interval_total &&
      obj1.average_service_execution_time ==
          obj2.average_service_execution_time &&
      obj1.average_service_check_interval ==
          obj2.average_service_check_interval &&
      obj1.average_host_check_interval == obj2.average_host_check_interval &&
      obj1.average_service_inter_check_delay ==
          obj2.average_service_inter_check_delay &&
      obj1.average_host_inter_check_delay ==
          obj2.average_host_inter_check_delay &&
      obj1.service_inter_check_delay == obj2.service_inter_check_delay &&
      obj1.host_inter_check_delay == obj2.host_inter_check_delay &&
      obj1.service_interleave_factor == obj2.service_interleave_factor &&
      obj1.max_service_check_spread == obj2.max_service_check_spread &&
      obj1.max_host_check_spread == obj2.max_host_check_spread &&
      obj1.first_service_check == obj2.first_service_check &&
      obj1.last_service_check == obj2.last_service_check &&
      obj1.first_host_check == obj2.first_host_check &&
      obj1.last_host_check == obj2.last_host_check);
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(sched_info const& obj1, sched_info const& obj2) throw() {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump command content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The command to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, sched_info const& obj) {
  os << "scheduling_info {\n"
        "  total_services:                      "
     << obj.total_services
     << "\n"
        "  total_scheduled_services:            "
     << obj.total_scheduled_services
     << "\n"
        "  total_hosts:                         "
     << obj.total_hosts
     << "\n"
        "  total_scheduled_hosts:               "
     << obj.total_scheduled_hosts
     << "\n"
        "  average_services_per_host:           "
     << obj.average_services_per_host
     << "\n"
        "  average_scheduled_services_per_host: "
     << obj.average_scheduled_services_per_host
     << "\n"
        "  service_check_interval_total:        "
     << obj.service_check_interval_total
     << "\n"
        "  host_check_interval_total:           "
     << obj.host_check_interval_total
     << "\n"
        "  average_service_execution_time:      "
     << obj.average_service_execution_time
     << "\n"
        "  average_service_check_interval:      "
     << obj.average_service_check_interval
     << "\n"
        "  average_host_check_interval:         "
     << obj.average_host_check_interval
     << "\n"
        "  average_service_inter_check_delay:   "
     << obj.average_service_inter_check_delay
     << "\n"
        "  average_host_inter_check_delay:      "
     << obj.average_host_inter_check_delay
     << "\n"
        "  service_inter_check_delay:           "
     << obj.service_inter_check_delay
     << "\n"
        "  host_inter_check_delay:              "
     << obj.host_inter_check_delay
     << "\n"
        "  service_interleave_factor:           "
     << obj.service_interleave_factor
     << "\n"
        "  max_service_check_spread:            "
     << obj.max_service_check_spread
     << "\n"
        "  max_host_check_spread:               "
     << obj.max_host_check_spread
     << "\n"
        "  first_service_check:                 "
     << string::ctime(obj.first_service_check)
     << "\n"
        "  last_service_check:                  "
     << string::ctime(obj.last_service_check)
     << "\n"
        "  first_host_check:                    "
     << string::ctime(obj.first_host_check)
     << "\n"
        "  last_host_check:                     "
     << string::ctime(obj.last_host_check)
     << "\n"
        "}\n";
  return (os);
}
