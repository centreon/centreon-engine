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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/downtimes/host_downtime.hh"
#include "com/centreon/engine/downtimes/service_downtime.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/timed_event.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::logging;

/**
 *  Remove a service/host downtime from its id.
 *
 * @param type HOST_DOWNTIME or SERVICE_DOWNTIME
 * @param downtime_id The downtime's id
 *
 */
void downtime_manager::delete_downtime(int type, uint64_t downtime_id) {
  std::multimap<time_t, std::shared_ptr<downtime>>::iterator it;
  std::multimap<time_t, std::shared_ptr<downtime>>::iterator end{
      instance()._scheduled_downtimes.end()};

  /* find the downtime we should remove */
  for (it = _scheduled_downtimes.begin(); it != end; ++it) {
    if (it->second->get_downtime_id() == downtime_id &&
        it->second->get_type() == type)
      break;
  }

  _scheduled_downtimes.erase(it);
}

/* unschedules a host or service downtime */
int downtime_manager::unschedule_downtime(int type, uint64_t downtime_id) {
  timed_event* temp_event(nullptr);
  std::shared_ptr<downtime> temp_downtime{find_downtime(type, downtime_id)};

  logger(dbg_functions, basic) << "unschedule_downtime()";

  /* find the downtime entry in the list in memory */
  if (!temp_downtime)
    return ERROR;

  if (temp_downtime->unschedule() == ERROR)
    return ERROR;

  /* remove scheduled entry from event queue */
  for (temp_event = event_list_high; temp_event != nullptr;
       temp_event = temp_event->next) {
    if (temp_event->event_type != EVENT_SCHEDULED_DOWNTIME)
      continue;
    if (((uint64_t)temp_event->event_data) == downtime_id)
      break;
  }
  if (temp_event != nullptr)
    remove_event(temp_event, &event_list_high, &event_list_high_tail);

  /* delete downtime entry */
  if (temp_downtime->get_type() == HOST_DOWNTIME)
    delete_host_downtime(downtime_id);
  else
    delete_service_downtime(downtime_id);

  /* unschedule all downtime entries that were triggered by this one */
  while (true) {
    std::multimap<time_t, std::shared_ptr<downtime>>::iterator it, end;
    for (it = _scheduled_downtimes.begin(), end = _scheduled_downtimes.end();
         it != end; ++it) {
      if (it->second->get_triggered_by() == downtime_id) {
        unschedule_downtime(ANY_DOWNTIME, it->second->get_downtime_id());
        break;
      }
    }

    if (it == _scheduled_downtimes.end())
      break;
  }
  return OK;
}

/* finds a specific downtime entry */
std::shared_ptr<downtime> downtime_manager::find_downtime(
    int type,
    uint64_t downtime_id) {
  for (std::multimap<time_t, std::shared_ptr<downtime>>::iterator
           it{_scheduled_downtimes.begin()},
       end{_scheduled_downtimes.end()};
       it != end; ++it) {
    if (type != ANY_DOWNTIME && it->second->get_type() != type)
      continue;
    if (it->second->get_downtime_id() == downtime_id)
      return it->second;
  }
  return nullptr;
}

/* checks for flexible (non-fixed) host downtime that should start now */
int downtime_manager::check_pending_flex_host_downtime(host* hst) {
  time_t current_time(0L);

  logger(dbg_functions, basic) << "check_pending_flex_host_downtime()";

  if (hst == nullptr)
    return ERROR;

  time(&current_time);

  /* if host is currently up, nothing to do */
  if (hst->get_current_state() == host::state_up)
    return OK;

  /* check all downtime entries */
  for (std::multimap<time_t, std::shared_ptr<downtime>>::iterator
           it{_scheduled_downtimes.begin()},
       end{_scheduled_downtimes.end()};
       it != end; ++it) {
    if (it->second->get_type() != HOST_DOWNTIME || it->second->is_fixed() ||
        it->second->is_in_effect() || it->second->get_triggered_by() != 0)
      continue;

    /* this entry matches our host! */
    host *temp_host(nullptr);
    host_map::const_iterator it_hg(host::hosts.find(it->second->get_hostname()));
    if (it_hg != host::hosts.end())
      temp_host = it_hg->second.get();

    if (temp_host == hst) {
      /* if the time boundaries are okay, start this scheduled downtime */
      if (it->second->get_start_time() <= current_time &&
          current_time <= it->second->get_end_time()) {
        logger(dbg_downtime, basic)
            << "Flexible downtime (id=" << it->second->get_downtime_id()
            << ") for host '" << hst->get_name() << "' starting now...";

        it->second->start_flex_downtime();
        it->second->handle();
      }
    }
  }
  return OK;
}

/* checks for flexible (non-fixed) service downtime that should start now */
int downtime_manager::check_pending_flex_service_downtime(service* svc) {
  time_t current_time(0L);

  logger(dbg_functions, basic) << "check_pending_flex_service_downtime()";

  if (svc == nullptr)
    return ERROR;

  time(&current_time);

  /* if service is currently ok, nothing to do */
  if (svc->get_current_state() == service::state_ok)
    return OK;

  /* check all downtime entries */
  for (std::multimap<time_t, std::shared_ptr<downtime>>::iterator
           it{_scheduled_downtimes.begin()},
       end{_scheduled_downtimes.end()};
       it != end; ++it) {
    if (it->second->get_type() != SERVICE_DOWNTIME || it->second->is_fixed() ||
        it->second->is_in_effect() || it->second->get_triggered_by() != 0)
      continue;

    service_downtime& dt(
        *std::static_pointer_cast<service_downtime>(it->second));

    service_map::const_iterator
      found(service::services.find({dt.get_hostname(), dt.get_service_description()}));

      /* this entry matches our service! */
    if (found != service::services.end() && found->second.get() == svc) {
      /* if the time boundaries are okay, start this scheduled downtime */
      if (dt.get_start_time() <= current_time && current_time <= dt.get_end_time()) {
        logger(dbg_downtime, basic)
            << "Flexible downtime (id=" << dt.get_downtime_id() << ") for service '"
            << svc->get_description() << "' on host '" << svc->get_hostname()
            << "' starting now...";

        dt.start_flex_downtime();
        dt.handle();
      }
    }
  }
  return OK;
}
std::multimap<time_t, std::shared_ptr<downtime>> const&
downtime_manager::get_scheduled_downtimes() const {
  return _scheduled_downtimes;
}

/* deletes a scheduled service downtime entry */
int downtime_manager::delete_service_downtime(uint64_t downtime_id) {
  /* delete the downtime from memory */
  delete_downtime(SERVICE_DOWNTIME, downtime_id);
  return OK;
}

void downtime_manager::clear_scheduled_downtimes() {
  _scheduled_downtimes.clear();
}

void downtime_manager::add_downtime(downtime* dt) {
  //std::shared_ptr<downtime> tmp{std::make_shared<downtime>(dt)};
  _scheduled_downtimes.insert({dt->get_start_time(), std::shared_ptr<downtime>(dt)});
}

int downtime_manager::check_for_expired_downtime() {
  time_t current_time(0L);

  logger(dbg_functions, basic) << "check_for_expired_downtime()";

  time(&current_time);

  /* check all downtime entries... */
  std::map<time_t, std::shared_ptr<downtime>>::iterator next_it{
      downtime_manager::instance()._scheduled_downtimes.begin()};
  for (std::map<time_t, std::shared_ptr<downtime>>::iterator
           it{_scheduled_downtimes.begin()},
       end{_scheduled_downtimes.end()};
       it != end; it = next_it) {
    downtime& dt(*it->second);
    ++next_it;

    /* this entry should be removed */
    if (!dt.is_in_effect() && dt.get_end_time() < current_time) {
      logger(dbg_downtime, basic)
          << "Expiring "
          << (dt.get_type() == HOST_DOWNTIME ? "host" : "service")
          << " downtime (id=" << dt.get_downtime_id() << ")...";

      /* delete the downtime entry */
      if (dt.get_type() == HOST_DOWNTIME)
        delete_host_downtime(dt.get_downtime_id());
      else
        delete_service_downtime(dt.get_downtime_id());
    }
  }
  return OK;
}

/*
** Deletes all host and service downtimes on a host by hostname,
** optionally filtered by service description, start time and comment.
** All char* must be set or nullptr - "" will silently fail to match.
** Returns number deleted.
*/
int downtime_manager::
    delete_downtime_by_hostname_service_description_start_time_comment(
        std::string const& hostname,
        std::string const& service_description,
        time_t start_time,
        std::string const& comment) {
  int deleted{0};

  /* Do not allow deletion of everything - must have at least 1 filter on. */
  if (hostname.empty() && service_description.empty() && start_time == 0 &&
      comment.empty())
    return deleted;

  std::pair<std::multimap<time_t, std::shared_ptr<downtime>>::iterator,
            std::multimap<time_t, std::shared_ptr<downtime>>::iterator>
      range;

  if (start_time != 0)
    range = _scheduled_downtimes.equal_range(start_time);
  else
    range = {_scheduled_downtimes.begin(), _scheduled_downtimes.end()};

  std::map<time_t, std::shared_ptr<downtime>>::iterator it,
      next_it{range.first};
  std::map<time_t, std::shared_ptr<downtime>>::iterator end{range.second};

  for (it = _scheduled_downtimes.begin(); it != end; it = next_it) {
    ++next_it;

    if (!comment.empty() && it->second->get_comment() != comment)
      continue;
    if (HOST_DOWNTIME == it->second->get_type()) {
      /* If service is specified, then do not delete the host downtime. */
      if (!service_description.empty())
        continue;
      if (!hostname.empty() && it->second->get_hostname() != hostname)
        continue;
    } else if (SERVICE_DOWNTIME == it->second->get_type()) {
      if (!hostname.empty() && it->second->get_hostname() != hostname)
        continue;
      if (!service_description.empty()) {
        service_downtime* svc{
            dynamic_cast<service_downtime*>(it->second.get())};

        if (!svc || svc->get_service_description() != service_description)
          continue;
      }
    }

    downtime_manager::instance().unschedule_downtime(it->second->get_type(),
                                                     it->second->get_downtime_id());
    ++deleted;
  }
  return deleted;
}

void downtime_manager::insert_downtime(std::shared_ptr<downtime> dt) {
  time_t start{dt->get_start_time()};
  _scheduled_downtimes.insert({start, dt});
}

/* deletes a scheduled host downtime entry */
int downtime_manager::delete_host_downtime(uint64_t downtime_id) {
  /* delete the downtime from memory */
  delete_downtime(HOST_DOWNTIME, downtime_id);
  return OK;
}

/**
 * Initialize downtime data
 *
 * @return OK or ERROR if an error occured.
 */
void downtime_manager::initialize_downtime_data() {
  /* clean up the old downtime data */
  xdddefault_validate_downtime_data();

  _next_id = 0;
}

/* removes invalid and old downtime entries from the downtime file */
int downtime_manager::xdddefault_validate_downtime_data() {
  int update_file = false;
  int save = true;

  /* remove stale downtimes */
  for (std::map<time_t,
                std::shared_ptr<com::centreon::engine::downtimes::downtime>>::
           iterator it(_scheduled_downtimes.begin()),
       end(_scheduled_downtimes.end());
       it != end;) {
    std::shared_ptr<com::centreon::engine::downtimes::downtime> temp_downtime(
        it->second);

    /* delete downtimes with invalid host names, invalid service descriptions
     * or that have expired. */
    if (temp_downtime->is_stale()) {
      update_file = true;
      it = _scheduled_downtimes.erase(it);
    } else
      ++it;
  }

  /* remove triggered downtimes without valid parents */
  for (std::map<time_t,
                std::shared_ptr<com::centreon::engine::downtimes::downtime>>::
           iterator it{_scheduled_downtimes.begin()},
       end{_scheduled_downtimes.end()};
       it != end;
       ++it) {
    save = true;
    downtimes::downtime& temp_downtime(*it->second);

    if (!temp_downtime.get_triggered_by())
      continue;

    if (!find_downtime(ANY_DOWNTIME, temp_downtime.get_triggered_by()))
      save = false;

    /* delete the downtime */
    if (!save) {
      update_file = true;
      it = _scheduled_downtimes.erase(it);
    }
  }

  return OK;
}

/**
 *  Return the next downtime id to use.
 *
 * @return an id as an unsigned long.
 */
uint64_t downtime_manager::get_next_downtime_id() {
  if (_next_id == 0) {
    for (std::pair<
             time_t,
             std::shared_ptr<com::centreon::engine::downtimes::downtime>> const&
             dt : _scheduled_downtimes)
      if (dt.second->get_downtime_id() >= _next_id)
        _next_id = dt.second->get_downtime_id();
  }

  _next_id++;
  return _next_id;
}

/* saves a host downtime entry */
downtime* downtime_manager::add_new_host_downtime(
    std::string const& host_name,
    time_t entry_time,
    char const* author,
    char const* comment_data,
    time_t start_time,
    time_t end_time,
    bool fixed,
    uint64_t triggered_by,
    unsigned long duration,
    uint64_t* downtime_id) {
  if (host_name.empty())
    throw engine_error()
        << "can not create a host downtime on host with empty name";

  /* find the next valid downtime id */
  uint64_t new_downtime_id{get_next_downtime_id()};

  /* add downtime to list in memory */
  host_downtime* retval{new host_downtime(
      host_name, entry_time, author, comment_data, start_time, end_time, fixed,
      triggered_by, duration, new_downtime_id)};
  retval->schedule();

  /* save downtime id */
  if (downtime_id)
    *downtime_id = new_downtime_id;

  /* send data to event broker */
  broker_downtime_data(NEBTYPE_DOWNTIME_ADD, NEBFLAG_NONE, NEBATTR_NONE,
                       HOST_DOWNTIME, host_name.c_str(), nullptr, entry_time,
                       author, comment_data, start_time, end_time, fixed,
                       triggered_by, duration, new_downtime_id, nullptr);
  return retval;
}

/* saves a service downtime entry */
downtime* downtime_manager::add_new_service_downtime(
    std::string const& host_name,
    std::string const& service_description,
    time_t entry_time,
    std::string const& author,
    std::string const& comment_data,
    time_t start_time,
    time_t end_time,
    bool fixed,
    uint64_t triggered_by,
    unsigned long duration,
    uint64_t* downtime_id) {
  if (host_name.empty() || service_description.empty())
    throw engine_error() << "can not create a service downtime on host with "
                            "empty name or service with empty description";

  /* find the next valid downtime id */
  uint64_t new_downtime_id{get_next_downtime_id()};

  /* add downtime to list in memory */
  service_downtime* retval{new service_downtime(host_name, service_description, entry_time, author,
                       comment_data, start_time, end_time, fixed, triggered_by,
                       duration, new_downtime_id)};
  retval->schedule();

  /* save downtime id */
  if (downtime_id)
    *downtime_id = new_downtime_id;

  /* send data to event broker */
  broker_downtime_data(NEBTYPE_DOWNTIME_ADD, NEBFLAG_NONE, NEBATTR_NONE,
                       SERVICE_DOWNTIME, host_name.c_str(),
                       service_description.c_str(), entry_time, author.c_str(),
                       comment_data.c_str(), start_time, end_time, fixed, triggered_by,
                       duration, new_downtime_id, nullptr);

  return retval;
}

/* schedules a host or service downtime */
int downtime_manager::schedule_downtime(int type,
                                        std::string const& host_name,
                                        std::string const& service_description,
                                        time_t entry_time,
                                        char const* author,
                                        char const* comment_data,
                                        time_t start_time,
                                        time_t end_time,
                                        bool fixed,
                                        uint64_t triggered_by,
                                        unsigned long duration,
                                        uint64_t* new_downtime_id) {
  uint64_t downtime_id{0L};

  logger(dbg_functions, basic) << "schedule_downtime()";

  /* don't add old or invalid downtimes */
  if (start_time >= end_time || end_time <= time(nullptr))
    return ERROR;

  /* add a new downtime entry */
  if (type == HOST_DOWNTIME)
    add_new_host_downtime(host_name, entry_time, author, comment_data,
                               start_time, end_time, fixed, triggered_by,
                               duration, &downtime_id);
  else
    add_new_service_downtime(host_name, service_description, entry_time,
                                  author, comment_data, start_time, end_time,
                                  fixed, triggered_by, duration, &downtime_id);

  /* register the scheduled downtime */
  register_downtime(type, downtime_id);

  /* return downtime id */
  if (new_downtime_id)
    *new_downtime_id = downtime_id;
  return OK;
}

/* registers scheduled downtime (schedules it, adds comments, etc.) */
int downtime_manager::register_downtime(int type, uint64_t downtime_id) {
  /* find the downtime entry in memory */
  std::shared_ptr<downtime> temp_downtime{find_downtime(type, downtime_id)};
  if (!temp_downtime)
    return ERROR;

  if (temp_downtime->subscribe() == ERROR)
    return ERROR;

  return OK;
}
