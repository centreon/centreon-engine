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

#include "com/centreon/engine/downtimes/service_downtime.hh"
#include <map>
#include <sstream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/timed_event.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::downtimes;

service_downtime::service_downtime(std::string const& host_name,
                                   std::string const& service_desc,
                                   time_t entry_time,
                                   std::string const& author,
                                   std::string const& comment_data,
                                   time_t start_time,
                                   time_t end_time,
                                   bool fixed,
                                   uint64_t triggered_by,
                                   int32_t duration,
                                   uint64_t downtime_id)
    : downtime{SERVICE_DOWNTIME, host_name,  entry_time, author,
               comment_data,     start_time, end_time,   fixed,
               triggered_by,     duration,   downtime_id},
      _service_description{service_desc} {}

/* finds a specific service downtime entry */
downtime* find_service_downtime(uint64_t downtime_id) {
  return downtime_manager::instance()
      .find_downtime(SERVICE_DOWNTIME, downtime_id)
      .get();
}

service_downtime::~service_downtime() {
  comment::delete_comment(_get_comment_id());
  /* send data to event broker */
  broker_downtime_data(
      NEBTYPE_DOWNTIME_DELETE, NEBFLAG_NONE, NEBATTR_NONE, SERVICE_DOWNTIME,
      get_hostname().c_str(), get_service_description().c_str(), _entry_time,
      get_author().c_str(), get_comment().c_str(), get_start_time(),
      get_end_time(), is_fixed(), get_triggered_by(), get_duration(),
      get_downtime_id(), nullptr);
}

/**
 *  This method tells if the associated host is no more here or if this downtime
 *  has expired.
 *
 * @return a boolean
 */
bool service_downtime::is_stale() const {
  bool retval{false};
  host_map::const_iterator it(host::hosts.find(get_hostname()));

  service_map::const_iterator found(
      service::services.find({get_hostname(), get_service_description()}));

  /* delete downtimes with invalid host names */
  if (it == host::hosts.end() || !it->second)
    retval = true;
  /* delete downtimes with invalid service descriptions */
  else if (found == service::services.end() || !found->second)
    retval = true;
  /* delete downtimes that have expired */
  else if (get_end_time() < time(nullptr))
    retval = true;

  return retval;
}

/**
 *  Dump retention of downtime.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The downtime to dump.
 *
 *  @return The output stream.
 */
void service_downtime::retention(std::ostream& os) const {
  os << "servicedowntime {\n";
  os << "host_name=" << get_hostname() << "\n";
  os << "service_description=" << get_service_description() << "\n";
  os << "author=" << get_author()
     << "\n"
        "comment="
     << get_comment()
     << "\n"
        "duration="
     << get_duration()
     << "\n"
        "end_time="
     << static_cast<uint32_t>(this->get_end_time())
     << "\n"
        "entry_time="
     << static_cast<uint32_t>(get_entry_time())
     << "\n"
        "fixed="
     << is_fixed()
     << "\n"
        "start_time="
     << static_cast<uint32_t>(get_start_time())
     << "\n"
        "triggered_by="
     << get_triggered_by()
     << "\n"
        "downtime_id="
     << get_downtime_id()
     << "\n"
        "}\n";
}

void service_downtime::print(std::ostream& os) const {
  os << "servicedowntime {\n";
  os << "\thost_name=" << get_hostname() << "\n";
  os << "\tservice_description=" << get_service_description() << "\n";
  os << "\tdowntime_id=" << get_downtime_id()
     << "\n"
        "\tentry_time="
     << static_cast<unsigned long>(_entry_time)
     << "\n"
        "\tstart_time="
     << static_cast<unsigned long>(get_start_time())
     << "\n"
        "\tend_time="
     << static_cast<unsigned long>(get_end_time())
     << "\n"
        "\ttriggered_by="
     << get_triggered_by()
     << "\n"
        "\tfixed="
     << is_fixed()
     << "\n"
        "\tduration="
     << get_duration()
     << "\n"
        "\tauthor="
     << get_author()
     << "\n"
        "\tcomment="
     << get_comment()
     << "\n"
        "\t}\n\n";
}

int service_downtime::unschedule() {
  service_map::const_iterator found(
      service::services.find({get_hostname(), get_service_description()}));

  /* find the host or service associated with this downtime */
  if (found == service::services.end() || !found->second.get())
    return ERROR;

  /* decrement pending flex downtime if necessary ... */
  if (!is_fixed() && _incremented_pending_downtime)
    found->second->dec_pending_flex_downtime();

  /* decrement the downtime depth variable and update status data if necessary
   */
  if (is_in_effect()) {
    /* send data to event broker */
    broker_downtime_data(
        NEBTYPE_DOWNTIME_STOP, NEBFLAG_NONE, NEBATTR_DOWNTIME_STOP_CANCELLED,
        get_type(), get_hostname().c_str(), get_service_description().c_str(),
        _entry_time, get_author().c_str(), get_comment().c_str(),
        get_start_time(), get_end_time(), is_fixed(), get_triggered_by(),
        get_duration(), get_downtime_id(), nullptr);

    found->second->dec_scheduled_downtime_depth();
    found->second->update_status(false);

    /* log a notice - this is parsed by the history CGI */
    if (found->second->get_scheduled_downtime_depth() == 0) {
      logger(log_info_message, basic)
          << "SERVICE DOWNTIME ALERT: " << found->second->get_hostname() << ";"
          << found->second->get_description()
          << ";CANCELLED; Scheduled downtime "
             "for service has been cancelled.";

      /* send a notification */
      found->second->notify(notifier::reason_downtimecancelled, "", "",
                            notifier::notification_option_none);
    }
  }
  return OK;
}

int service_downtime::subscribe() {
  logger(dbg_functions, basic) << "service_downtime::subscribe()";

  service_map::const_iterator found(
      service::services.find({get_hostname(), get_service_description()}));

  /* find the host or service associated with this downtime */
  if (found == service::services.end() || !found->second)
    return ERROR;

  /* create the comment */
  time_t start_time{get_start_time()};
  time_t end_time{get_end_time()};
  char start_time_string[MAX_DATETIME_LENGTH] = "";
  char end_time_string[MAX_DATETIME_LENGTH] = "";
  get_datetime_string(&start_time, start_time_string, MAX_DATETIME_LENGTH,
                      SHORT_DATE_TIME);
  get_datetime_string(&end_time, end_time_string, MAX_DATETIME_LENGTH,
                      SHORT_DATE_TIME);
  int hours{get_duration() / 3600};
  int minutes{(get_duration() - hours * 3600) / 60};
  int seconds{get_duration() - hours * 3600 - minutes * 60};

  char const* type_string{"service"};
  std::ostringstream oss;
  if (is_fixed())
    oss << "This " << type_string
        << " has been scheduled for fixed downtime from " << start_time_string
        << " to " << end_time_string << " Notifications for the " << type_string
        << " will not be sent out during that time period.";
  else
    oss << "This " << type_string
        << " has been scheduled for flexible downtime starting between "
        << start_time_string << " and " << end_time_string
        << " and lasting for a period of " << hours << " hours and " << minutes
        << " minutes. Notifications for the " << type_string
        << " will not be sent out during that time period.";

  logger(dbg_downtime, basic) << "Scheduled Downtime Details:";
  logger(dbg_downtime, basic) << " Type:        Service Downtime\n"
                                 " Host:        "
                              << found->second->get_hostname()
                              << "\n"
                                 " Service:     "
                              << found->second->get_description();
  logger(dbg_downtime, basic)
      << " Fixed/Flex:  " << (is_fixed() ? "Fixed\n" : "Flexible\n")
      << " Start:       " << start_time_string
      << "\n"
         " End:         "
      << end_time_string
      << "\n"
         " Duration:    "
      << hours << "h " << minutes << "m " << seconds
      << "s\n"
         " Downtime ID: "
      << get_downtime_id()
      << "\n"
         " Trigger ID:  "
      << get_triggered_by();

  /* add a non-persistent comment to the host or service regarding the scheduled
   * outage */
  std::shared_ptr<comment> com{new comment(
      comment::service, comment::downtime, found->second->get_hostname(),
      found->second->get_description(), time(nullptr),
      "(Centreon Engine Process)", oss.str(), false, comment::internal, false,
      (time_t)0)};

  comment::comments.insert({com->get_comment_id(), com});
  _comment_id = com->get_comment_id();

  /*** SCHEDULE DOWNTIME - FLEXIBLE (NON-FIXED) DOWNTIME IS HANDLED AT A LATER
   * POINT ***/

  /* only non-triggered downtime is scheduled... */
  if (get_triggered_by() == 0) {
    uint64_t* new_downtime_id{new uint64_t{get_downtime_id()}};
    timed_event* evt =
        new timed_event(EVENT_SCHEDULED_DOWNTIME, get_start_time(), false, 0,
                        nullptr, false, (void*)new_downtime_id, nullptr, 0);
    evt->schedule(true);
  }

#ifdef PROBABLY_NOT_NEEDED
  /*** FLEXIBLE DOWNTIME SANITY CHECK - ADDED 02/17/2008 ****/

  /* if host/service is in a non-OK/UP state right now, see if we should start
   * flexible time immediately */
  /* this is new logic added in 3.0rc3 */
  if (!is_fixed())
    check_pending_flex_service_downtime(svc);
#endif
  return OK;
}

int service_downtime::handle() {
  time_t event_time(0L);
  int attr(0);

  logger(dbg_functions, basic) << "handle_downtime()";

  service_map::const_iterator found(
      service::services.find({get_hostname(), get_service_description()}));

  /* find the host or service associated with this downtime */
  if (found == service::services.end() || !found->second)
    return ERROR;

  /* if downtime if flexible and host/svc is in an ok state, don't do anything
   * right now (wait for event handler to kick it off) */
  /* start_flex_downtime variable is set to true by event handler functions */
  if (!is_fixed()) {
    /* we're not supposed to force a start of flex downtime... */
    if (!_start_flex_downtime) {
      /* host is up or service is ok, so we don't really do anything right now
       */
      if (found->second->get_current_state() == service::state_ok) {
        /* increment pending flex downtime counter */
        found->second->inc_pending_flex_downtime();
        _incremented_pending_downtime = true;

        /*** SINCE THE FLEX DOWNTIME MAY NEVER START, WE HAVE TO PROVIDE A WAY
         * OF EXPIRING UNUSED DOWNTIME... ***/

        timed_event* evt =
            new timed_event(EVENT_EXPIRE_DOWNTIME, get_end_time() + 1, false, 0,
                            nullptr, false, nullptr, nullptr, 0);
        evt->schedule(true);
        return OK;
      }
    }
  }

  /* have we come to the end of the scheduled downtime? */
  if (is_in_effect()) {
    /* send data to event broker */
    attr = NEBATTR_DOWNTIME_STOP_NORMAL;
    broker_downtime_data(
        NEBTYPE_DOWNTIME_STOP, NEBFLAG_NONE, attr, this->get_type(),
        get_hostname().c_str(), get_service_description().c_str(), _entry_time,
        get_author().c_str(), get_comment().c_str(), get_start_time(),
        get_end_time(), is_fixed(), get_triggered_by(), get_duration(),
        get_downtime_id(), nullptr);

    /* decrement the downtime depth variable */
    found->second->dec_scheduled_downtime_depth();

    if (found->second->get_scheduled_downtime_depth() == 0) {
      logger(dbg_downtime, basic)
          << "Service '" << found->second->get_description() << "' on host '"
          << found->second->get_hostname()
          << "' has exited from a period of "
             "scheduled downtime (id="
          << get_downtime_id() << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
          << "SERVICE DOWNTIME ALERT: " << found->second->get_hostname() << ";"
          << found->second->get_description()
          << ";STOPPED; Service has exited from a period of scheduled "
             "downtime";

      /* send a notification */
      found->second->notify(notifier::reason_downtimeend, get_author(),
                            get_comment(), notifier::notification_option_none);
    }

    /* update the status data */
    found->second->update_status(false);

    /* decrement pending flex downtime if necessary */
    if (!is_fixed() && _incremented_pending_downtime) {
      if (found->second->get_pending_flex_downtime() > 0)
        found->second->dec_pending_flex_downtime();
    }

    /* handle (stop) downtime that is triggered by this one */
    while (true) {
      std::multimap<time_t, std::shared_ptr<downtime>>::const_iterator it,
          end{downtime_manager::instance().get_scheduled_downtimes().end()};

      /* list contents might change by recursive calls, so we use this
       * inefficient method to prevent segfaults */
      for (it = downtime_manager::instance().get_scheduled_downtimes().begin();
           it != end; ++it) {
        if (it->second->get_triggered_by() == get_downtime_id()) {
          it->second->handle();
          break;
        }
      }

      for (it = downtime_manager::instance().get_scheduled_downtimes().begin();
           it != end; ++it) {
        if (it->second->get_triggered_by() == get_downtime_id()) {
          it->second->handle();
          break;
        }
      }

      if (it == end)
        break;
    }

    /* delete downtime entry */
    downtime_manager::instance().delete_service_downtime(get_downtime_id());
  }
  /* else we are just starting the scheduled downtime */
  else {
    /* send data to event broker */
    broker_downtime_data(
        NEBTYPE_DOWNTIME_START, NEBFLAG_NONE, NEBATTR_NONE, get_type(),
        get_hostname().c_str(), get_service_description().c_str(), _entry_time,
        get_author().c_str(), get_comment().c_str(), get_start_time(),
        get_end_time(), is_fixed(), get_triggered_by(), get_duration(),
        get_downtime_id(), nullptr);

    if (found->second->get_scheduled_downtime_depth() == 0) {
      logger(dbg_downtime, basic)
          << "Service '" << found->second->get_description() << "' on host '"
          << found->second->get_hostname()
          << "' has entered a period of scheduled "
             "downtime (id="
          << get_downtime_id() << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
          << "SERVICE DOWNTIME ALERT: " << found->second->get_hostname() << ";"
          << found->second->get_description()
          << ";STARTED; Service has entered a period of scheduled "
             "downtime";

      /* send a notification */
      found->second->notify(notifier::reason_downtimestart, get_author(),
                            get_comment(), notifier::notification_option_none);
    }

    /* increment the downtime depth variable */
    found->second->inc_scheduled_downtime_depth();

    /* set the in effect flag */
    _set_in_effect(true);

    /* update the status data */
    found->second->update_status(false);

    /* schedule an event */
    if (!is_fixed())
      event_time = (time_t)((unsigned long)time(nullptr) + get_duration());
    else
      event_time = get_end_time() + 1;
    uint64_t* new_downtime_id{new uint64_t{get_downtime_id()}};
    timed_event* evt =
        new timed_event(EVENT_SCHEDULED_DOWNTIME, event_time, false, 0, nullptr,
                        false, (void*)new_downtime_id, nullptr, 0);
    evt->schedule(true);

    /* handle (start) downtime that is triggered by this one */
    std::multimap<time_t, std::shared_ptr<downtime>>::const_iterator it,
        end = downtime_manager::instance().get_scheduled_downtimes().end();

    for (it = downtime_manager::instance().get_scheduled_downtimes().begin();
         it != end; ++it) {
      if (it->second->get_triggered_by() == get_downtime_id())
        it->second->handle();
    }
  }
  return OK;
}

std::string const& service_downtime::get_service_description() const {
  return _service_description;
}

void service_downtime::schedule() {
  downtime_manager::instance().add_downtime(this);

  /* send data to event broker */
  broker_downtime_data(
      NEBTYPE_DOWNTIME_LOAD, NEBFLAG_NONE, NEBATTR_NONE, SERVICE_DOWNTIME,
      _hostname.c_str(), _service_description.c_str(), _entry_time,
      _author.c_str(), _comment.c_str(), _start_time, _end_time, _fixed,
      _triggered_by, _duration, _downtime_id, nullptr);
}
