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

#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/downtimes/host_downtime.hh"
#include "com/centreon/engine/downtimes/service_downtime.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/timed_event.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "compatibility/find.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::logging;

/**
 *  Remove a service/host downtime from its id.
 *
 * @param type HOST_DOWNTIME or SERVICE_DOWNTIME
 * @param downtime_id The downtime's id
 *
 */
void downtime_manager::delete_downtime(int type, unsigned long downtime_id) {
  std::multimap<time_t, std::shared_ptr<downtime>>::iterator it;
  std::multimap<time_t, std::shared_ptr<downtime>>::iterator end{
      instance()._scheduled_downtimes.end()};

  /* find the downtime we should remove */
  for (it = _scheduled_downtimes.begin(); it != end; ++it) {
    if (it->second->downtime_id == downtime_id &&
        it->second->get_type() == type)
      break;
  }

  /* remove the downtime from the list in memory */
  /* first remove the comment associated with this downtime */
  if (it->second->get_type() == HOST_DOWNTIME)
    delete_host_comment(it->second->comment_id);
  else
    delete_service_comment(it->second->comment_id);

  _scheduled_downtimes.erase(it);
}

/* unschedules a host or service downtime */
int downtime_manager::unschedule_downtime(int type, unsigned long downtime_id) {
  downtime* next_downtime(NULL);
  host* hst(NULL);
  service* svc(NULL);
  timed_event* temp_event(NULL);
  int attr(0);
  std::shared_ptr<downtime> temp_downtime{find_downtime(type, downtime_id)};

  logger(dbg_functions, basic) << "unschedule_downtime()";

  /* find the downtime entry in the list in memory */
  if (!temp_downtime)
    return ERROR;

  if (temp_downtime->unschedule() == ERROR)
    return ERROR;

  /* remove scheduled entry from event queue */
  for (temp_event = event_list_high; temp_event != NULL;
       temp_event = temp_event->next) {
    if (temp_event->event_type != EVENT_SCHEDULED_DOWNTIME)
      continue;
    if (((unsigned long)temp_event->event_data) == downtime_id)
      break;
  }
  if (temp_event != NULL)
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
      if (it->second->triggered_by == downtime_id) {
        unschedule_downtime(ANY_DOWNTIME, it->second->downtime_id);
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
    unsigned long downtime_id) {
  for (std::multimap<time_t, std::shared_ptr<downtime>>::iterator
           it{_scheduled_downtimes.begin()},
       end{_scheduled_downtimes.end()};
       it != end; ++it) {
    if (type != ANY_DOWNTIME && it->second->get_type() != type)
      continue;
    if (it->second->downtime_id == downtime_id)
      return it->second;
  }
  return nullptr;
}

/* checks for flexible (non-fixed) host downtime that should start now */
int downtime_manager::check_pending_flex_host_downtime(host* hst) {
  downtime* temp_downtime(NULL);
  time_t current_time(0L);

  logger(dbg_functions, basic) << "check_pending_flex_host_downtime()";

  if (hst == NULL)
    return ERROR;

  time(&current_time);

  /* if host is currently up, nothing to do */
  if (hst->get_current_state() == HOST_UP)
    return OK;

  /* check all downtime entries */
  for (std::multimap<time_t, std::shared_ptr<downtime>>::iterator
           it{_scheduled_downtimes.begin()},
       end{_scheduled_downtimes.end()};
       it != end; ++it) {
    if (it->second->get_type() != HOST_DOWNTIME || it->second->fixed ||
        it->second->is_in_effect || it->second->triggered_by != 0)
      continue;

    /* this entry matches our host! */
    if (::find_host(it->second->get_hostname().c_str()) == hst) {
      /* if the time boundaries are okay, start this scheduled downtime */
      if (it->second->start_time <= current_time &&
          current_time <= it->second->end_time) {
        logger(dbg_downtime, basic)
            << "Flexible downtime (id=" << it->second->downtime_id
            << ") for host '" << hst->get_name() << "' starting now...";

        it->second->start_flex_downtime = true;
        temp_downtime->handle();
      }
    }
  }
  return OK;
}

/* checks for flexible (non-fixed) service downtime that should start now */
int downtime_manager::check_pending_flex_service_downtime(service* svc) {
  downtime* temp_downtime(NULL);
  time_t current_time(0L);

  logger(dbg_functions, basic) << "check_pending_flex_service_downtime()";

  if (svc == NULL)
    return ERROR;

  time(&current_time);

  /* if service is currently ok, nothing to do */
  if (svc->current_state == STATE_OK)
    return OK;

  /* check all downtime entries */
  for (std::multimap<time_t, std::shared_ptr<downtime>>::iterator
           it{_scheduled_downtimes.begin()},
       end{_scheduled_downtimes.end()};
       it != end; ++it) {
    if (it->second->get_type() != SERVICE_DOWNTIME || it->second->fixed ||
        it->second->is_in_effect || it->second->triggered_by != 0)
      continue;

    service_downtime& dt{
        *std::static_pointer_cast<service_downtime>(it->second)};

    /* this entry matches our service! */
    if (::find_service(dt.get_hostname().c_str(),
                       dt.get_service_description().c_str()) == svc) {
      /* if the time boundaries are okay, start this scheduled downtime */
      if (dt.start_time <= current_time && current_time <= dt.end_time) {
        logger(dbg_downtime, basic)
            << "Flexible downtime (id=" << dt.downtime_id << ") for service '"
            << svc->description << "' on host '" << svc->host_name
            << "' starting now...";

        dt.start_flex_downtime = true;
        temp_downtime->handle();
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
int downtime_manager::delete_service_downtime(unsigned long downtime_id) {
  /* delete the downtime from memory */
  delete_downtime(SERVICE_DOWNTIME, downtime_id);
  return OK;
}

void downtime_manager::clear_scheduled_downtimes() {
  _scheduled_downtimes.clear();
}

int downtime_manager::check_for_expired_downtime() {
  downtime* temp_downtime(NULL);
  downtime* next_downtime(NULL);
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
    downtime& dt{*it->second};
    ++next_it;

    /* this entry should be removed */
    if (!dt.is_in_effect && dt.end_time < current_time) {
      logger(dbg_downtime, basic)
          << "Expiring "
          << (dt.get_type() == HOST_DOWNTIME ? "host" : "service")
          << " downtime (id=" << dt.downtime_id << ")...";

      /* delete the downtime entry */
      if (dt.get_type() == HOST_DOWNTIME)
        delete_host_downtime(dt.downtime_id);
      else
        delete_service_downtime(dt.downtime_id);
    }
  }
  return OK;
}

/*
** Deletes all host and service downtimes on a host by hostname,
** optionally filtered by service description, start time and comment.
** All char* must be set or NULL - "" will silently fail to match.
** Returns number deleted.
*/
int downtime_manager::
    delete_downtime_by_hostname_service_description_start_time_comment(
        char const* hostname,
        char const* service_description,
        time_t start_time,
        char const* comment) {
  int deleted{0};

  /* Do not allow deletion of everything - must have at least 1 filter on. */
  if ((NULL == hostname) && (NULL == service_description) &&
      (0 == start_time) && (NULL == comment))
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

    if (comment != NULL && strcmp(it->second->comment, comment) != 0)
      continue;
    if (HOST_DOWNTIME == it->second->get_type()) {
      /* If service is specified, then do not delete the host downtime. */
      if (service_description != NULL)
        continue;
      if (hostname != NULL &&
          strcmp(it->second->get_hostname().c_str(), hostname) != 0)
        continue;
    } else if (SERVICE_DOWNTIME == it->second->get_type()) {
      if (hostname != NULL &&
          strcmp(it->second->get_hostname().c_str(), hostname) != 0)
        continue;
      if (service_description) {
        service_downtime* svc{
            dynamic_cast<service_downtime*>(it->second.get())};

        if (!svc ||
            strcmp(svc->get_service_description().c_str(), service_description))
          continue;
      }
    }

    downtime_manager::instance().unschedule_downtime(it->second->get_type(),
                                                     it->second->downtime_id);
    ++deleted;
  }
  return deleted;
}

void downtime_manager::insert_downtime(std::shared_ptr<downtime> dt) {
  time_t start{dt->start_time};
  _scheduled_downtimes.insert({start, dt});
}

/* deletes a scheduled host downtime entry */
int downtime_manager::delete_host_downtime(unsigned long downtime_id) {
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
       it != end;) {
    save = true;
    com::centreon::engine::downtimes::downtime& temp_downtime{*it->second};

    if (!temp_downtime.triggered_by)
      continue;

    if (!find_downtime(ANY_DOWNTIME, temp_downtime.triggered_by))
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
unsigned long downtime_manager::get_next_downtime_id() {
  if (_next_id == 0) {
    for (std::pair<
             time_t,
             std::shared_ptr<com::centreon::engine::downtimes::downtime>> const&
             dt : _scheduled_downtimes)
      if (dt.second->downtime_id >= _next_id)
        _next_id = dt.second->downtime_id;
  }

  _next_id++;
  return _next_id;
}

std::shared_ptr<downtime> downtime_manager::add_host_downtime(std::string const& host_name,
                                        time_t entry_time,
                                        char const* author,
                                        char const* comment_data,
                                        time_t start_time,
                                        time_t end_time,
                                        bool fixed,
                                        unsigned long triggered_by,
                                        unsigned long duration,
                                        unsigned long downtime_id) {
  /* don't add triggered downtimes that don't have a valid parent */
  if (triggered_by > 0 && !find_downtime(ANY_DOWNTIME, triggered_by))
    throw engine_error()
        << "can not add triggered host downtime without a valid parent";

  /* we don't have enough info */
  if (host_name.empty())
    throw engine_error()
        << "can not create a host downtime on host with empty name";

  /* allocate memory for the downtime */
  std::shared_ptr<downtime> retval{new host_downtime(host_name)};

  if (author)
    retval->author = string::dup(author);
  if (comment_data)
    retval->comment = string::dup(comment_data);

  retval->entry_time = entry_time;
  retval->start_time = start_time;
  retval->end_time = end_time;
  retval->fixed = fixed;
  retval->triggered_by = triggered_by;
  retval->duration = duration;
  retval->downtime_id = downtime_id;

  _scheduled_downtimes.insert({retval->start_time, retval});

  /* send data to event broker */
  broker_downtime_data(NEBTYPE_DOWNTIME_LOAD, NEBFLAG_NONE, NEBATTR_NONE,
                       HOST_DOWNTIME, host_name.c_str(), nullptr, entry_time,
                       author, comment_data, start_time, end_time, fixed,
                       triggered_by, duration, downtime_id, NULL);
  return retval;
}

/* saves a host downtime entry */
std::shared_ptr<downtime> downtime_manager::add_new_host_downtime(
    std::string const& host_name,
    time_t entry_time,
    char const* author,
    char const* comment_data,
    time_t start_time,
    time_t end_time,
    bool fixed,
    unsigned long triggered_by,
    unsigned long duration,
    unsigned long* downtime_id) {
  if (host_name.empty())
    throw engine_error()
        << "can not create a host downtime on host with empty name";

  /* find the next valid downtime id */
  unsigned long new_downtime_id{get_next_downtime_id()};

  /* add downtime to list in memory */
  std::shared_ptr<downtime> retval{add_host_downtime(
      host_name, entry_time, author, comment_data, start_time, end_time, fixed,
      triggered_by, duration, new_downtime_id)};

  /* save downtime id */
  if (downtime_id)
    *downtime_id = new_downtime_id;

  /* send data to event broker */
  broker_downtime_data(NEBTYPE_DOWNTIME_ADD, NEBFLAG_NONE, NEBATTR_NONE,
                       HOST_DOWNTIME, host_name.c_str(), NULL, entry_time,
                       author, comment_data, start_time, end_time, fixed,
                       triggered_by, duration, new_downtime_id, NULL);
  return retval;
}

/* adds a service downtime entry to the list in memory */
std::shared_ptr<downtime> downtime_manager::add_service_downtime(
    std::string const& host_name,
    std::string const& svc_description,
    time_t entry_time,
    char const* author,
    char const* comment_data,
    time_t start_time,
    time_t end_time,
    bool fixed,
    unsigned long triggered_by,
    unsigned long duration,
    unsigned long downtime_id) {
  /* don't add triggered downtimes that don't have a valid parent */
  if (triggered_by > 0 && !find_downtime(ANY_DOWNTIME, triggered_by))
    throw engine_error()
        << "can not add triggered service downtime without a valid parent";

  if (host_name.empty() || svc_description.empty())
    throw engine_error()
        << "can not create a host downtime on host with empty name";

  std::shared_ptr<downtime> retval{
      new service_downtime(host_name, svc_description)};

  if (author)
    retval->author = string::dup(author);

  if (comment_data)
    retval->comment = string::dup(comment_data);

  retval->entry_time = entry_time;
  retval->start_time = start_time;
  retval->end_time = end_time;
  retval->fixed = fixed;
  retval->triggered_by = triggered_by;
  retval->duration = duration;
  retval->downtime_id = downtime_id;

  _scheduled_downtimes.insert({retval->start_time, retval});

  /* send data to event broker */
  broker_downtime_data(NEBTYPE_DOWNTIME_LOAD, NEBFLAG_NONE, NEBATTR_NONE,
                       SERVICE_DOWNTIME, host_name.c_str(),
                       svc_description.c_str(), entry_time, author,
                       comment_data, start_time, end_time, fixed, triggered_by,
                       duration, downtime_id, NULL);
  return retval;
}

/* saves a service downtime entry */
std::shared_ptr<downtime> downtime_manager::add_new_service_downtime(
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
    unsigned long* downtime_id) {
  if (host_name.empty() || service_description.empty())
    throw engine_error() << "can not create a service downtime on host with "
                            "empty name or service with empty description";

  /* find the next valid downtime id */
  unsigned long new_downtime_id{get_next_downtime_id()};

  /* add downtime to list in memory */
  std::shared_ptr<downtime> retval{add_service_downtime(host_name, service_description, entry_time, author,
                       comment_data, start_time, end_time, fixed, triggered_by,
                       duration, new_downtime_id)};

  /* save downtime id */
  if (downtime_id != NULL)
    *downtime_id = new_downtime_id;

  /* send data to event broker */
  broker_downtime_data(NEBTYPE_DOWNTIME_ADD, NEBFLAG_NONE, NEBATTR_NONE,
                       SERVICE_DOWNTIME, host_name.c_str(),
                       service_description.c_str(), entry_time, author,
                       comment_data, start_time, end_time, fixed, triggered_by,
                       duration, new_downtime_id, NULL);

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
                                        unsigned long triggered_by,
                                        unsigned long duration,
                                        unsigned long* new_downtime_id) {
  unsigned long downtime_id(0L);

  logger(dbg_functions, basic) << "schedule_downtime()";

  /* don't add old or invalid downtimes */
  if (start_time >= end_time || end_time <= time(NULL))
    return ERROR;

  /* add a new downtime entry */
  std::shared_ptr<downtime> dt;
  if (type == HOST_DOWNTIME)
    dt = add_new_host_downtime(host_name, entry_time, author, comment_data,
                               start_time, end_time, fixed, triggered_by,
                               duration, &downtime_id);
  else
    dt = add_new_service_downtime(host_name, service_description, entry_time,
                                  author, comment_data, start_time, end_time,
                                  fixed, triggered_by, duration, &downtime_id);

  /* register the scheduled downtime */
  register_downtime(type, downtime_id);

  /* return downtime id */
  if (new_downtime_id)
    *new_downtime_id = downtime_id;
  return OK;
}

