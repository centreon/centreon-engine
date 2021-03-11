/*
 * Copyright 2019-2020 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */

#include "com/centreon/engine/command_manager.hh"

#include <google/protobuf/util/time_util.h>
#include <sys/types.h>
#include <unistd.h>

#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::downtimes;

/**
 *  The default constructor
 */
command_manager::command_manager(): _has_data{false} {}

/**
 * @brief Just an accessor to the command_manager instance.
 *
 * @return A reference to the instance.
 */
command_manager& command_manager::instance() {
  static command_manager instance;
  return instance;
}

void command_manager::enqueue(std::packaged_task<int(void)>&& f) {
  std::lock_guard<std::mutex> lock(_queue_m);
  _queue.emplace_back(std::move(f));
  if (!_has_data) {
    _has_data = true;
    _queue_cv.notify_all();
  }
}

/**
 * @brief Executes external commands stored in _queue.
 * 
 * @param time is send in seconds
 * 
 */
void command_manager::execute(float time) {
  std::unique_lock<std::mutex> lock(_queue_m);
  std::chrono::duration<float> t{time};
  // Wait a while so we don't hog the CPU...
  // While waiting we executes commands in queue.
  while (_queue_cv.wait_for(lock, t, [this] {
    return static_cast<bool>(_has_data); 
  })) { 
    std::deque<std::packaged_task<int()>> queue;
    std::swap(queue, _queue);
    logger(dbg_functions, basic) << "size of queue" << queue.size(); 
    _has_data = false;
    lock.unlock();

    auto end = queue.end();
    auto it = queue.begin();
    while (it != end) {
      (*it)();
      ++it;
      queue.pop_front();
    }
    lock.lock();
  }
}

/* submits a passive service check result for later processing */
int command_manager::process_passive_service_check(
    time_t check_time,
    const std::string& host_name,
    const std::string& svc_description,
    uint32_t return_code,
    const std::string& output) {
  const std::string* real_host_name = nullptr;

  /* skip this service check result if we aren't accepting passive service
   * checks */
  if (!config->accept_passive_service_checks())
    return ERROR;

  /* make sure we have a reasonable return code */
  if (return_code > 3)
    return ERROR;

  /* find the host by its name or address */
  host_map::const_iterator it(host::hosts.find(host_name));
  if (it != host::hosts.end() && it->second)
    real_host_name = &host_name;
  else {
    for (host_map::iterator itt(host::hosts.begin()), end(host::hosts.end());
         itt != end; ++itt) {
      if (itt->second && itt->second->get_address() == host_name) {
        real_host_name = &itt->first;
        it = itt;
        break;
      }
    }
  }

  /* we couldn't find the host */
  if (real_host_name == nullptr) {
    logger(log_runtime_warning, basic)
        << "Warning:  Passive check result was received for service '"
        << svc_description << "' on host '" << host_name
        << "', but the host could not be found!";
    return ERROR;
  }

  /* make sure the service exists */
  service_map::const_iterator found(
      service::services.find({*real_host_name, svc_description}));
  if (found == service::services.end() || !found->second) {
    logger(log_runtime_warning, basic)
        << "Warning:  Passive check result was received for service '"
        << svc_description << "' on host '" << host_name
        << "', but the service could not be found!";
    return ERROR;
  }

  /* skip this is we aren't accepting passive checks for this service */
  if (!found->second->get_accept_passive_checks())
    return ERROR;

  timeval tv;
  gettimeofday(&tv, nullptr);

  timeval set_tv = {.tv_sec = check_time, .tv_usec = 0};

  check_result* result =
      new check_result(service_check, found->second.get(),
                       checkable::check_passive, CHECK_OPTION_NONE, false,
                       static_cast<double>(tv.tv_sec - check_time) +
                           static_cast<double>(tv.tv_usec) / 1000000.0,
                       set_tv, set_tv, false, true, return_code, output);

  /* make sure the return code is within bounds */
  if (result->get_return_code() < 0 || result->get_return_code() > 3)
    result->set_return_code(service::state_unknown);

  if (result->get_latency() < 0.0)
    result->set_latency(0.0);

  checks::checker::instance().add_check_result_to_reap(result);
  return OK;
}

/* process passive host check result */
int command_manager::process_passive_host_check(time_t check_time,
                                                const std::string& host_name,
                                                uint32_t return_code,
                                                const std::string& output) {
  const std::string* real_host_name = nullptr;

  /* skip this host check result if we aren't accepting passive host checks */
  if (!config->accept_passive_service_checks())
    return ERROR;

  /* make sure we have a reasonable return code */
  if (return_code > 2)
    return ERROR;

  /* find the host by its name or address */
  host_map::const_iterator it(host::hosts.find(host_name));
  if (it != host::hosts.end() && it->second)
    real_host_name = &host_name;
  else {
    for (host_map::iterator itt(host::hosts.begin()), end(host::hosts.end());
         itt != end; ++itt) {
      if (itt->second && itt->second->get_address() == host_name) {
        real_host_name = &itt->first;
        it = itt;
        break;
      }
    }
  }

  /* we couldn't find the host */
  if (real_host_name == nullptr) {
    logger(log_runtime_warning, basic)
        << "Warning:  Passive check result was received for host '" << host_name
        << "', but the host could not be found!";
    return ERROR;
  }

  /* skip this is we aren't accepting passive checks for this host */
  if (!it->second->get_accept_passive_checks())
    return ERROR;

  timeval tv;
  gettimeofday(&tv, nullptr);
  timeval tv_start;
  tv_start.tv_sec = check_time;
  tv_start.tv_usec = 0;

  check_result* result =
      new check_result(host_check, it->second.get(), checkable::check_passive,
                       CHECK_OPTION_NONE, false,
                       static_cast<double>(tv.tv_sec - check_time) +
                           static_cast<double>(tv.tv_usec) / 1000000.0,
                       tv_start, tv_start, false, true, return_code, output);

  /* make sure the return code is within bounds */
  if (result->get_return_code() < 0 || result->get_return_code() > 3)
    result->set_return_code(service::state_unknown);

  if (result->get_latency() < 0.0)
    result->set_latency(0.0);

  checks::checker::instance().add_check_result_to_reap(result);
  return OK;
}

int command_manager::get_stats(std::string const& request, Stats* response) {
  if (request == "default") {
    response->mutable_program_status()->set_modified_host_attributes(
        modified_host_process_attributes);
    response->mutable_program_status()->set_modified_service_attributes(
        modified_service_process_attributes);
    response->mutable_program_status()->set_pid(getpid());
    *response->mutable_program_status()->mutable_program_start() =
        google::protobuf::util::TimeUtil::SecondsToTimestamp(program_start);
    *response->mutable_program_status()->mutable_last_command_check() =
        google::protobuf::util::TimeUtil::SecondsToTimestamp(
            last_command_check);
    *response->mutable_program_status()->mutable_last_log_rotation() =
        google::protobuf::util::TimeUtil::SecondsToTimestamp(last_log_rotation);
    response->mutable_program_status()->set_enable_notifications(
        enable_notifications);
    response->mutable_program_status()->set_active_service_checks_enabled(
        execute_service_checks);
    response->mutable_program_status()->set_passive_service_checks_enabled(
        accept_passive_service_checks);
    response->mutable_program_status()->set_active_host_checks_enabled(
        execute_host_checks);
    response->mutable_program_status()->set_passive_host_checks_enabled(
        accept_passive_host_checks);
    response->mutable_program_status()->set_enable_event_handlers(
        enable_event_handlers);
    response->mutable_program_status()->set_obsess_over_services(
        obsess_over_services);
    response->mutable_program_status()->set_obsess_over_hosts(
        obsess_over_hosts);
    response->mutable_program_status()->set_check_service_freshness(
        check_service_freshness);
    response->mutable_program_status()->set_check_host_freshness(
        check_host_freshness);
    response->mutable_program_status()->set_enable_flap_detection(
        enable_flap_detection);
    response->mutable_program_status()->set_process_performance_data(
        process_performance_data);
    response->mutable_program_status()->set_global_host_event_handler(
        global_host_event_handler);
    response->mutable_program_status()->set_global_service_event_handler(
        global_service_event_handler);
    response->mutable_program_status()->set_next_comment_id(
        comment::get_next_comment_id());
    response->mutable_program_status()->set_next_event_id(next_event_id);
    response->mutable_program_status()->set_next_problem_id(next_problem_id);
    response->mutable_program_status()->set_next_notification_id(
        notifier::get_next_notification_id());

    uint32_t used_external_command_buffer_slots = 0;
    uint32_t high_external_command_buffer_slots = 0;
    // get number f items in the command buffer
    if (config->check_external_commands()) {
      pthread_mutex_lock(&external_command_buffer.buffer_lock);
      used_external_command_buffer_slots = external_command_buffer.items;
      high_external_command_buffer_slots = external_command_buffer.high;
      pthread_mutex_unlock(&external_command_buffer.buffer_lock);
    }
    response->mutable_program_status()->set_total_external_command_buffer_slots(
        config->external_command_buffer_slots());
    response->mutable_program_status()->set_used_external_command_buffer_slots(
        used_external_command_buffer_slots);
    response->mutable_program_status()->set_high_external_command_buffer_slots(
        high_external_command_buffer_slots);
    response->mutable_program_status()->set_high_external_command_buffer_slots(
        high_external_command_buffer_slots);
    response->mutable_program_status()->add_active_scheduled_host_check_stats(
        check_statistics[ACTIVE_SCHEDULED_HOST_CHECK_STATS].minute_stats[0]);
    response->mutable_program_status()->add_active_scheduled_host_check_stats(
        check_statistics[ACTIVE_SCHEDULED_HOST_CHECK_STATS].minute_stats[1]);
    response->mutable_program_status()->add_active_scheduled_host_check_stats(
        check_statistics[ACTIVE_SCHEDULED_HOST_CHECK_STATS].minute_stats[2]);
    response->mutable_program_status()->add_active_ondemand_host_check_stats(
        check_statistics[ACTIVE_ONDEMAND_HOST_CHECK_STATS].minute_stats[0]);
    response->mutable_program_status()->add_active_ondemand_host_check_stats(
        check_statistics[ACTIVE_ONDEMAND_HOST_CHECK_STATS].minute_stats[1]);
    response->mutable_program_status()->add_active_ondemand_host_check_stats(
        check_statistics[ACTIVE_ONDEMAND_HOST_CHECK_STATS].minute_stats[2]);
    response->mutable_program_status()->add_passive_host_check_stats(
        check_statistics[PASSIVE_HOST_CHECK_STATS].minute_stats[0]);
    response->mutable_program_status()->add_passive_host_check_stats(
        check_statistics[PASSIVE_HOST_CHECK_STATS].minute_stats[1]);
    response->mutable_program_status()->add_passive_host_check_stats(
        check_statistics[PASSIVE_HOST_CHECK_STATS].minute_stats[2]);
    response->mutable_program_status()
        ->add_active_scheduled_service_check_stats(
            check_statistics[ACTIVE_SCHEDULED_SERVICE_CHECK_STATS]
                .minute_stats[0]);
    response->mutable_program_status()
        ->add_active_scheduled_service_check_stats(
            check_statistics[ACTIVE_SCHEDULED_SERVICE_CHECK_STATS]
                .minute_stats[1]);
    response->mutable_program_status()
        ->add_active_scheduled_service_check_stats(
            check_statistics[ACTIVE_SCHEDULED_SERVICE_CHECK_STATS]
                .minute_stats[2]);
    response->mutable_program_status()->add_active_ondemand_service_check_stats(
        check_statistics[ACTIVE_ONDEMAND_SERVICE_CHECK_STATS].minute_stats[0]);
    response->mutable_program_status()->add_active_ondemand_service_check_stats(
        check_statistics[ACTIVE_ONDEMAND_SERVICE_CHECK_STATS].minute_stats[1]);
    response->mutable_program_status()->add_active_ondemand_service_check_stats(
        check_statistics[ACTIVE_ONDEMAND_SERVICE_CHECK_STATS].minute_stats[2]);
    response->mutable_program_status()->add_passive_service_check_stats(
        check_statistics[PASSIVE_SERVICE_CHECK_STATS].minute_stats[0]);
    response->mutable_program_status()->add_passive_service_check_stats(
        check_statistics[PASSIVE_SERVICE_CHECK_STATS].minute_stats[1]);
    response->mutable_program_status()->add_passive_service_check_stats(
        check_statistics[PASSIVE_SERVICE_CHECK_STATS].minute_stats[2]);
    response->mutable_program_status()->add_cached_host_check_stats(
        check_statistics[ACTIVE_CACHED_HOST_CHECK_STATS].minute_stats[0]);
    response->mutable_program_status()->add_cached_host_check_stats(
        check_statistics[ACTIVE_CACHED_HOST_CHECK_STATS].minute_stats[1]);
    response->mutable_program_status()->add_cached_host_check_stats(
        check_statistics[ACTIVE_CACHED_HOST_CHECK_STATS].minute_stats[2]);
    response->mutable_program_status()->add_cached_service_check_stats(
        check_statistics[ACTIVE_CACHED_SERVICE_CHECK_STATS].minute_stats[0]);
    response->mutable_program_status()->add_cached_service_check_stats(
        check_statistics[ACTIVE_CACHED_SERVICE_CHECK_STATS].minute_stats[1]);
    response->mutable_program_status()->add_cached_service_check_stats(
        check_statistics[ACTIVE_CACHED_SERVICE_CHECK_STATS].minute_stats[2]);
    response->mutable_program_status()->add_external_command_stats(
        check_statistics[EXTERNAL_COMMAND_STATS].minute_stats[0]);
    response->mutable_program_status()->add_external_command_stats(
        check_statistics[EXTERNAL_COMMAND_STATS].minute_stats[1]);
    response->mutable_program_status()->add_external_command_stats(
        check_statistics[EXTERNAL_COMMAND_STATS].minute_stats[2]);
    response->mutable_program_status()->add_parallel_host_check_stats(
        check_statistics[PARALLEL_HOST_CHECK_STATS].minute_stats[0]);
    response->mutable_program_status()->add_parallel_host_check_stats(
        check_statistics[PARALLEL_HOST_CHECK_STATS].minute_stats[1]);
    response->mutable_program_status()->add_parallel_host_check_stats(
        check_statistics[PARALLEL_HOST_CHECK_STATS].minute_stats[2]);
    response->mutable_program_status()->add_serial_host_check_stats(
        check_statistics[SERIAL_HOST_CHECK_STATS].minute_stats[0]);
    response->mutable_program_status()->add_serial_host_check_stats(
        check_statistics[SERIAL_HOST_CHECK_STATS].minute_stats[1]);
    response->mutable_program_status()->add_serial_host_check_stats(
        check_statistics[SERIAL_HOST_CHECK_STATS].minute_stats[2]);
    response->mutable_program_configuration()->set_hosts_count(
        host::hosts.size());
    get_services_stats(response->mutable_services_stats());
    get_hosts_stats(response->mutable_hosts_stats());
  } else if (request == "start")
    return get_restart_stats(response->mutable_restart_status());
  return 0;
}

void command_manager::schedule_and_propagate_downtime(host* temp_host,
                                     time_t entry_time,
                                     char const* author,
                                     char const* comment_data,
                                     time_t start_time,
                                     time_t end_time,
                                     int fixed,
                                     unsigned long triggered_by,
                                     unsigned long duration) {
  /* check all child hosts... */
  for (host_map_unsafe::iterator it(temp_host->child_hosts.begin()),
       end(temp_host->child_hosts.end());
       it != end; ++it) {
    if (it->second == nullptr)
      continue;
    /* recurse... */
    schedule_and_propagate_downtime(it->second, entry_time, author,
                                    comment_data, start_time, end_time, fixed,
                                    triggered_by, duration);

    /* schedule downtime for this host */
    downtime_manager::instance().schedule_downtime(
        downtime::host_downtime, it->first, "", entry_time, author,
        comment_data, start_time, end_time, fixed, triggered_by, duration,
        nullptr);
  }
}

int command_manager::get_services_stats(ServicesStats* sstats) {
  time_t now;
  time(&now);
  uint32_t checked_services = 0;
  uint32_t scheduled_services = 0;
  uint32_t actively_checked = 0;
  uint32_t passively_checked = 0;

  double global_min_state_change = std::numeric_limits<double>::max(),
         global_max_state_change = std::numeric_limits<double>::min(),
         global_avg_state_change = 0;

  double active_min_state_change = std::numeric_limits<double>::max(),
         active_max_state_change = std::numeric_limits<double>::min(),
         active_avg_state_change = 0;

  double passive_min_state_change = std::numeric_limits<double>::max(),
         passive_max_state_change = std::numeric_limits<double>::min(),
         passive_avg_state_change = 0;

  double active_min_latency = std::numeric_limits<double>::max(),
         active_max_latency = std::numeric_limits<double>::min(),
         active_avg_latency = 0;

  double passive_min_latency = std::numeric_limits<double>::max(),
         passive_max_latency = std::numeric_limits<double>::min(),
         passive_avg_latency = 0;

  double active_min_execution_time = std::numeric_limits<double>::max(),
         active_max_execution_time = std::numeric_limits<double>::min(),
         active_avg_execution_time = 0;

  uint32_t active_checks_last_1min = 0, active_checks_last_5min = 0,
           active_checks_last_15min = 0, active_checks_last_1hour = 0;

  uint32_t passive_checks_last_1min = 0, passive_checks_last_5min = 0,
           passive_checks_last_15min = 0, passive_checks_last_1hour = 0;

  uint32_t ok = 0, warning = 0, critical = 0, unknown = 0;

  uint32_t downtime = 0, flapping = 0;

  auto min_max_sum = [](double v, double& min, double& max, double& sum) {
    sum += v;
    if (v < min)
      min = v;
    if (v > max)
      max = v;
  };

  for (auto it = service::services_by_id.begin(),
            end = service::services_by_id.end();
       it != end; ++it) {
    service* svc = it->second.get();

    /******************************** GLOBAL **********************************/
    min_max_sum(svc->get_percent_state_change(), global_min_state_change,
                global_max_state_change, global_avg_state_change);

    if (svc->has_been_checked())
      ++checked_services;
    if (svc->get_should_be_scheduled())
      ++scheduled_services;

    switch (svc->get_current_state()) {
      case service::state_ok:
        ++ok;
        break;
      case service::state_warning:
        ++warning;
        break;
      case service::state_critical:
        ++critical;
        break;
      case service::state_unknown:
        ++unknown;
        break;
    }

    if (svc->is_in_downtime())
      ++downtime;
    if (svc->get_is_flapping())
      ++flapping;

    if (svc->get_check_type() == checkable::check_active) {
      /****************************** ACTIVE **********************************/
      ++actively_checked;
      min_max_sum(svc->get_latency(), active_min_latency, active_max_latency,
                  active_avg_latency);

      min_max_sum(svc->get_execution_time(), active_min_execution_time,
                  active_max_execution_time, active_avg_execution_time);

      min_max_sum(svc->get_percent_state_change(), active_min_state_change,
                  active_max_state_change, active_avg_state_change);

      uint32_t time_diff = now - svc->get_last_check();
      if (time_diff <= 60)
        ++active_checks_last_1min;
      if (time_diff <= 300)
        ++active_checks_last_5min;
      if (time_diff <= 900)
        ++active_checks_last_15min;
      if (time_diff <= 900)
        ++active_checks_last_1hour;

    } else {
      /****************************** PASSIVE *********************************/
      ++passively_checked;
      min_max_sum(svc->get_latency(), passive_min_latency, passive_max_latency,
                  passive_avg_latency);

      min_max_sum(svc->get_percent_state_change(), passive_min_state_change,
                  passive_max_state_change, passive_avg_state_change);

      uint32_t time_diff = now - svc->get_last_check();
      if (time_diff <= 60)
        ++passive_checks_last_1min;
      if (time_diff <= 300)
        ++passive_checks_last_5min;
      if (time_diff <= 900)
        ++passive_checks_last_15min;
      if (time_diff <= 900)
        ++passive_checks_last_1hour;
    }
  }

  /********************************* SUMMARY **********************************/
  size_t size = service::services.size();
  if (size)
    global_avg_state_change /= size;

  if (actively_checked) {
    active_avg_state_change /= actively_checked;
    active_avg_latency /= actively_checked;
    active_avg_execution_time /= actively_checked;
  } else {
    active_min_state_change = active_max_state_change = 0;
    active_min_latency = active_max_latency = 0;
  }

  if (passively_checked) {
    passive_avg_state_change /= passively_checked;
    passive_avg_latency /= passively_checked;
  } else {
    passive_min_state_change = passive_max_state_change = 0;
    passive_min_latency = passive_max_latency = 0;
  }

  sstats->set_services_count(size);
  sstats->set_checked_services(checked_services);
  sstats->set_scheduled_services(scheduled_services);
  sstats->set_actively_checked(actively_checked);

  sstats->set_min_state_change(global_min_state_change);
  sstats->set_max_state_change(global_max_state_change);
  sstats->set_average_state_change(global_avg_state_change);

  sstats->mutable_active_services()->set_min_latency(active_min_latency);
  sstats->mutable_active_services()->set_max_latency(active_max_latency);
  sstats->mutable_active_services()->set_average_latency(active_avg_latency);

  sstats->mutable_active_services()->set_min_execution_time(
      active_min_execution_time);
  sstats->mutable_active_services()->set_max_execution_time(
      active_max_execution_time);
  sstats->mutable_active_services()->set_average_execution_time(
      active_avg_execution_time);

  sstats->mutable_active_services()->set_min_state_change(
      active_min_state_change);
  sstats->mutable_active_services()->set_max_state_change(
      active_max_state_change);
  sstats->mutable_active_services()->set_average_state_change(
      active_avg_state_change);

  sstats->mutable_active_services()->set_checks_last_1min(
      active_checks_last_1min);
  sstats->mutable_active_services()->set_checks_last_5min(
      active_checks_last_5min);
  sstats->mutable_active_services()->set_checks_last_15min(
      active_checks_last_15min);
  sstats->mutable_active_services()->set_checks_last_1hour(
      active_checks_last_1hour);

  sstats->mutable_passive_services()->set_min_latency(passive_min_latency);
  sstats->mutable_passive_services()->set_max_latency(passive_max_latency);
  sstats->mutable_passive_services()->set_average_latency(passive_avg_latency);

  sstats->mutable_passive_services()->set_min_state_change(
      passive_min_state_change);
  sstats->mutable_passive_services()->set_max_state_change(
      passive_max_state_change);
  sstats->mutable_passive_services()->set_average_state_change(
      passive_avg_state_change);

  sstats->mutable_passive_services()->set_checks_last_1min(
      passive_checks_last_1min);
  sstats->mutable_passive_services()->set_checks_last_5min(
      passive_checks_last_5min);
  sstats->mutable_passive_services()->set_checks_last_15min(
      passive_checks_last_15min);
  sstats->mutable_passive_services()->set_checks_last_1hour(
      passive_checks_last_1hour);

  sstats->set_ok(ok);
  sstats->set_warning(warning);
  sstats->set_critical(critical);
  sstats->set_unknown(unknown);

  sstats->set_flapping(flapping);
  sstats->set_downtime(downtime);
  return 0;
}

int command_manager::get_hosts_stats(HostsStats* hstats) {
  time_t now;
  time(&now);
  uint32_t checked_hosts = 0;
  uint32_t scheduled_hosts = 0;
  uint32_t actively_checked = 0;
  uint32_t passively_checked = 0;

  double global_min_state_change = std::numeric_limits<double>::max(),
         global_max_state_change = std::numeric_limits<double>::min(),
         global_avg_state_change = 0;

  double active_min_state_change = std::numeric_limits<double>::max(),
         active_max_state_change = std::numeric_limits<double>::min(),
         active_avg_state_change = 0;

  double passive_min_state_change = std::numeric_limits<double>::max(),
         passive_max_state_change = std::numeric_limits<double>::min(),
         passive_avg_state_change = 0;

  double active_min_latency = std::numeric_limits<double>::max(),
         active_max_latency = std::numeric_limits<double>::min(),
         active_avg_latency = 0;

  double passive_min_latency = std::numeric_limits<double>::max(),
         passive_max_latency = std::numeric_limits<double>::min(),
         passive_avg_latency = 0;

  double active_min_execution_time = std::numeric_limits<double>::max(),
         active_max_execution_time = std::numeric_limits<double>::min(),
         active_avg_execution_time = 0;

  uint32_t active_checks_last_1min = 0, active_checks_last_5min = 0,
           active_checks_last_15min = 0, active_checks_last_1hour = 0;

  uint32_t passive_checks_last_1min = 0, passive_checks_last_5min = 0,
           passive_checks_last_15min = 0, passive_checks_last_1hour = 0;

  uint32_t up = 0, down = 0, unreachable = 0;

  uint32_t downtime = 0, flapping = 0;

  auto min_max_sum = [](double v, double& min, double& max, double& sum) {
    sum += v;
    if (v < min)
      min = v;
    if (v > max)
      max = v;
  };

  for (auto it = host::hosts_by_id.begin(), end = host::hosts_by_id.end();
       it != end; ++it) {
    host* hst = it->second.get();

    /******************************** GLOBAL **********************************/
    min_max_sum(hst->get_percent_state_change(), global_min_state_change,
                global_max_state_change, global_avg_state_change);

    if (hst->has_been_checked())
      ++checked_hosts;
    if (hst->get_should_be_scheduled())
      ++scheduled_hosts;

    switch (hst->get_current_state()) {
      case host::state_up:
        ++up;
        break;
      case host::state_down:
        ++down;
        break;
      case host::state_unreachable:
        ++unreachable;
        break;
    }

    if (hst->is_in_downtime())
      ++downtime;
    if (hst->get_is_flapping())
      ++flapping;

    if (hst->get_check_type() == checkable::check_active) {
      /****************************** ACTIVE **********************************/
      ++actively_checked;
      min_max_sum(hst->get_latency(), active_min_latency, active_max_latency,
                  active_avg_latency);

      min_max_sum(hst->get_execution_time(), active_min_execution_time,
                  active_max_execution_time, active_avg_execution_time);

      min_max_sum(hst->get_percent_state_change(), active_min_state_change,
                  active_max_state_change, active_avg_state_change);

      uint32_t time_diff = now - hst->get_last_check();
      if (time_diff <= 60)
        ++active_checks_last_1min;
      if (time_diff <= 300)
        ++active_checks_last_5min;
      if (time_diff <= 900)
        ++active_checks_last_15min;
      if (time_diff <= 900)
        ++active_checks_last_1hour;

    } else {
      /****************************** PASSIVE *********************************/
      ++passively_checked;
      min_max_sum(hst->get_latency(), passive_min_latency, passive_max_latency,
                  passive_avg_latency);

      min_max_sum(hst->get_percent_state_change(), passive_min_state_change,
                  passive_max_state_change, passive_avg_state_change);

      uint32_t time_diff = now - hst->get_last_check();
      if (time_diff <= 60)
        ++passive_checks_last_1min;
      if (time_diff <= 300)
        ++passive_checks_last_5min;
      if (time_diff <= 900)
        ++passive_checks_last_15min;
      if (time_diff <= 900)
        ++passive_checks_last_1hour;
    }
  }

  /********************************* SUMMARY **********************************/
  size_t size = host::hosts.size();
  if (size)
    global_avg_state_change /= size;

  if (actively_checked) {
    active_avg_state_change /= actively_checked;
    active_avg_latency /= actively_checked;
    active_avg_execution_time /= actively_checked;
  } else {
    active_min_state_change = active_max_state_change = 0;
    active_min_latency = active_max_latency = 0;
  }

  if (passively_checked) {
    passive_avg_state_change /= passively_checked;
    passive_avg_latency /= passively_checked;
  } else {
    passive_min_state_change = passive_max_state_change = 0;
    passive_min_latency = passive_max_latency = 0;
  }

  hstats->set_hosts_count(size);
  hstats->set_checked_hosts(checked_hosts);
  hstats->set_scheduled_hosts(scheduled_hosts);
  hstats->set_actively_checked(actively_checked);

  hstats->set_min_state_change(global_min_state_change);
  hstats->set_max_state_change(global_max_state_change);
  hstats->set_average_state_change(global_avg_state_change);

  hstats->mutable_active_hosts()->set_min_latency(active_min_latency);
  hstats->mutable_active_hosts()->set_max_latency(active_max_latency);
  hstats->mutable_active_hosts()->set_average_latency(active_avg_latency);

  hstats->mutable_active_hosts()->set_min_execution_time(
      active_min_execution_time);
  hstats->mutable_active_hosts()->set_max_execution_time(
      active_max_execution_time);
  hstats->mutable_active_hosts()->set_average_execution_time(
      active_avg_execution_time);

  hstats->mutable_active_hosts()->set_min_state_change(active_min_state_change);
  hstats->mutable_active_hosts()->set_max_state_change(active_max_state_change);
  hstats->mutable_active_hosts()->set_average_state_change(
      active_avg_state_change);

  hstats->mutable_active_hosts()->set_checks_last_1min(active_checks_last_1min);
  hstats->mutable_active_hosts()->set_checks_last_5min(active_checks_last_5min);
  hstats->mutable_active_hosts()->set_checks_last_15min(
      active_checks_last_15min);
  hstats->mutable_active_hosts()->set_checks_last_1hour(
      active_checks_last_1hour);

  hstats->mutable_passive_hosts()->set_min_latency(passive_min_latency);
  hstats->mutable_passive_hosts()->set_max_latency(passive_max_latency);
  hstats->mutable_passive_hosts()->set_average_latency(passive_avg_latency);

  hstats->mutable_passive_hosts()->set_min_state_change(
      passive_min_state_change);
  hstats->mutable_passive_hosts()->set_max_state_change(
      passive_max_state_change);
  hstats->mutable_passive_hosts()->set_average_state_change(
      passive_avg_state_change);

  hstats->mutable_passive_hosts()->set_checks_last_1min(
      passive_checks_last_1min);
  hstats->mutable_passive_hosts()->set_checks_last_5min(
      passive_checks_last_5min);
  hstats->mutable_passive_hosts()->set_checks_last_15min(
      passive_checks_last_15min);
  hstats->mutable_passive_hosts()->set_checks_last_1hour(
      passive_checks_last_1hour);

  hstats->set_up(up);
  hstats->set_down(down);
  hstats->set_unreachable(unreachable);

  hstats->set_flapping(flapping);
  hstats->set_downtime(downtime);
  return 0;
}

int command_manager::get_restart_stats(RestartStats* response) {
  *response->mutable_apply_start() =
      ::google::protobuf::util::TimeUtil::TimeTToTimestamp(
          std::chrono::system_clock::to_time_t(
              restart_apply_stats.apply_start));
  *response->mutable_objects_expansion() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.objects_expansion.count());
  *response->mutable_objects_difference() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.objects_difference.count());
  *response->mutable_apply_config() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_config.count());

  *response->mutable_apply_timeperiods() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_timeperiods.count());
  *response->mutable_apply_connectors() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_connectors.count());
  *response->mutable_apply_commands() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_commands.count());
  *response->mutable_apply_contacts() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_contacts.count());
  *response->mutable_apply_hosts() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_hosts.count());
  *response->mutable_apply_services() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_services.count());
  *response->mutable_resolve_hosts() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.resolve_hosts.count());
  *response->mutable_resolve_services() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.resolve_services.count());
  *response->mutable_apply_host_dependencies() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_host_dependencies.count());
  *response->mutable_resolve_host_dependencies() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.resolve_host_dependencies.count());
  *response->mutable_apply_service_dependencies() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_service_dependencies.count());
  *response->mutable_resolve_service_dependencies() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.resolve_service_dependencies.count());
  *response->mutable_apply_host_escalations() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_host_escalations.count());
  *response->mutable_resolve_host_escalations() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.resolve_host_escalations.count());
  *response->mutable_apply_service_escalations() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_service_escalations.count());
  *response->mutable_resolve_service_escalations() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.resolve_service_escalations.count());
  *response->mutable_apply_new_config() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_new_config.count());
  *response->mutable_apply_scheduler() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.apply_scheduler.count());
  *response->mutable_check_circular_paths() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.check_circular_paths.count());
  *response->mutable_reload_modules() =
      ::google::protobuf::util::TimeUtil::MillisecondsToDuration(
          restart_apply_stats.reload_modules.count());

  *response->mutable_apply_end() =
      ::google::protobuf::util::TimeUtil::TimeTToTimestamp(
          std::chrono::system_clock::to_time_t(restart_apply_stats.apply_end));
  return 0;
}
