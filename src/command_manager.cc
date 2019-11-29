/*
 * Copyright 2019 Centreon (https://www.centreon.com/)
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

#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/command_manager.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/**
 *  The default constructor
 */
command_manager::command_manager() {}

/**
 * @brief Just an accessor to the command_manager instance.
 *
 * @return A reference to the instance.
 */
command_manager& command_manager::instance() {
  static command_manager instance;
  return instance;
}

void command_manager::enqueue(std::function<int(void)>&& f) {
  std::lock_guard<std::mutex> lock(_queue_m);
  _queue.push_back(f);
}

void command_manager::execute() {
  std::unique_lock<std::mutex> lock(_queue_m);
  if (_queue.empty())
    return;
  auto end = _queue.end();
  lock.unlock();

  auto it = _queue.begin();
  while (it != end) {
    auto fn = *it;
    fn();
    ++it;
    _queue.pop_front();
  }
}

/* submits a passive service check result for later processing */
int command_manager::process_passive_service_check(time_t check_time,
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

  timeval set_tv;
  set_tv.tv_sec = check_time;
  set_tv.tv_usec = 0;

  check_result result(service_check, found->second->get_host_id(),
                      found->second->get_service_id(), checkable::check_passive,
                      CHECK_OPTION_NONE, false,
                      (double)((double)(tv.tv_sec - check_time) +
                               (double)(tv.tv_usec / 1000.0) / 1000.0),
                      set_tv, set_tv, false, true, return_code, output);

  /* make sure the return code is within bounds */
  if (result.get_return_code() < 0 || result.get_return_code() > 3)
    result.set_return_code(service::state_unknown);

  if (result.get_latency() < 0.0) {
    result.set_latency(0.0);
  }

  checks::checker::instance().push_check_result(std::move(result));

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

  check_result result(host_check, it->second->get_host_id(), 0UL,
                      checkable::check_passive, CHECK_OPTION_NONE, false,
                      (double)((double)(tv.tv_sec - check_time) +
                               (double)(tv.tv_usec / 1000.0) / 1000.0),
                      tv_start, tv_start, false, true, return_code, output);

  /* make sure the return code is within bounds */
  if (result.get_return_code() < 0 || result.get_return_code() > 3)
    result.set_return_code(service::state_unknown);

  if (result.get_latency() < 0.0)
    result.set_latency(0.0);

  checks::checker::instance().push_check_result(std::move(result));

  return OK;
}
