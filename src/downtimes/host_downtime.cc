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
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/downtimes/host_downtime.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/timed_event.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::downtimes;

host_downtime::host_downtime(std::string const& host_name,
                             time_t entry_time,
                             std::string const& author,
                             std::string const& comment,
                             time_t start_time,
                             time_t end_time,
                             bool fixed,
                             uint64_t triggered_by,
                             int32_t duration,
                             uint64_t downtime_id)
    : downtime(HOST_DOWNTIME,
               host_name,
               entry_time,
               author,
               comment,
               start_time,
               end_time,
               fixed,
               triggered_by,
               duration,
               downtime_id) {}

host_downtime::~host_downtime() {
  comment::delete_comment(_get_comment_id());
  /* send data to event broker */
  broker_downtime_data(NEBTYPE_DOWNTIME_DELETE, NEBFLAG_NONE, NEBATTR_NONE,
                       HOST_DOWNTIME, _hostname.c_str(), nullptr, _entry_time,
                       _author.c_str(), _comment.c_str(), get_start_time(),
                       get_end_time(), is_fixed(), get_triggered_by(),
                       get_duration(), get_downtime_id(), nullptr);
}

/* adds a host downtime entry to the list in memory */
/**
 *  This method tells if the associated host is no more here or if this downtime
 *  has expired.
 *
 * @return a boolean
 */
bool host_downtime::is_stale() const {
  bool retval{false};

  host_map::const_iterator it(host::hosts.find(get_hostname()));

  /* delete downtimes with invalid host names */
  if (it == host::hosts.end() || it->second == nullptr)
    retval = true;
  /* delete downtimes that have expired */
  else if (get_end_time() < time(NULL))
    retval = true;

  return retval;
}

void host_downtime::retention(std::ostream& os) const {
  os << "hostdowntime {\n";
  os << "host_name=" << get_hostname() << "\n";
  os << "author=" << get_author() << "\n"
    "comment=" << get_comment() << "\n"
    "duration=" << get_duration() << "\n"
    "end_time=" << static_cast<unsigned long>(get_end_time()) << "\n"
    "entry_time=" << static_cast<unsigned long>(get_entry_time()) << "\n"
    "fixed=" << is_fixed() << "\n"
    "start_time=" << static_cast<unsigned long>(get_start_time()) << "\n"
    "triggered_by=" << get_triggered_by() << "\n"
    "downtime_id=" << get_downtime_id() << "\n"
    "}\n";
}

void host_downtime::print(std::ostream& os) const {
  os << "hostdowntime {\n";
  os << "\thost_name=" << get_hostname() << "\n";
  os << "\tdowntime_id=" << get_downtime_id() << "\n"
            "\tentry_time="
         << static_cast<unsigned long>(get_entry_time()) << "\n"
            "\tstart_time="
         << static_cast<unsigned long>(get_start_time()) << "\n"
            "\tend_time="
         << static_cast<unsigned long>(get_end_time()) << "\n"
            "\ttriggered_by="
         << get_triggered_by() << "\n"
            "\tfixed="
         << is_fixed() << "\n"
            "\tduration="
         << get_duration() << "\n"
            "\tauthor="
         << get_author() << "\n"
            "\tcomment="
         << get_comment() << "\n"
            "\t}\n\n";
}

int host_downtime::unschedule() {
  host_map::const_iterator it(host::hosts.find(get_hostname()));

  /* delete downtimes with invalid host names */
  if (it == host::hosts.end() || !it->second)
    return ERROR;
  host* hst = it->second.get();

  /* decrement pending flex downtime if necessary ... */
  if (!is_fixed() && _incremented_pending_downtime)
    hst->set_pending_flex_downtime(hst->get_pending_flex_downtime() - 1);

  /* decrement the downtime depth variable and update status data if necessary
   */
  if (is_in_effect()) {

    /* send data to event broker */
    broker_downtime_data(
      NEBTYPE_DOWNTIME_STOP,
      NEBFLAG_NONE,
      NEBATTR_DOWNTIME_STOP_CANCELLED,
      get_type(),
      get_hostname().c_str(),
      nullptr,
      _entry_time,
      get_author().c_str(),
      get_comment().c_str(),
      get_start_time(),
      get_end_time(),
      is_fixed(),
      get_triggered_by(),
      get_duration(),
      get_downtime_id(),
      nullptr);

    hst->set_scheduled_downtime_depth(hst->get_scheduled_downtime_depth() - 1);
    hst->update_status(false);

    /* log a notice - this is parsed by the history CGI */
    if (hst->get_scheduled_downtime_depth() == 0) {
      logger(log_info_message, basic)
        << "HOST DOWNTIME ALERT: " << hst->get_name()
        << ";CANCELLED; Scheduled downtime for host has been "
           "cancelled.";

      /* send a notification */
      hst->notify(
        notifier::notification_downtimecancelled,
        "",
        "",
        notifier::notification_option_none);
    }
  }
  return OK;
}

int host_downtime::subscribe() {
  logger(dbg_functions, basic)
    << "host_downtime::subscribe()";

  host_map::const_iterator it(host::hosts.find(get_hostname()));

  /* find the host or service associated with this downtime */
  if (it == host::hosts.end() || !it->second)
    return ERROR;

  host* hst = it->second.get();

  /* create the comment */
  time_t start_time{get_start_time()};
  time_t end_time{get_end_time()};
  char start_time_string[MAX_DATETIME_LENGTH] = "";
  char end_time_string[MAX_DATETIME_LENGTH] = "";
  get_datetime_string(
    &start_time,
    start_time_string,
    MAX_DATETIME_LENGTH,
    SHORT_DATE_TIME);
  get_datetime_string(
    &end_time,
    end_time_string,
    MAX_DATETIME_LENGTH,
    SHORT_DATE_TIME);
  int hours{get_duration() / 3600};
  int minutes{(get_duration() - hours * 3600) / 60};
  int seconds{get_duration() - hours * 3600 - minutes * 60};

  char const* type_string{"host"};
  std::ostringstream oss;
  if (is_fixed())
    oss << "This " << type_string
        << " has been scheduled for fixed downtime from "
        << start_time_string << " to " << end_time_string
        << " Notifications for the " << type_string
        << " will not be sent out during that time period.";
  else
    oss << "This " << type_string
        << " has been scheduled for flexible downtime starting between "
        << start_time_string << " and " << end_time_string
        << " and lasting for a period of " << hours << " hours and "
        << minutes << " minutes. Notifications for the " << type_string
        << " will not be sent out during that time period.";

  logger(dbg_downtime, basic) << "Scheduled Downtime Details:";
    logger(dbg_downtime, basic)
      << " Type:        Host Downtime\n"
         " Host:        " << hst->get_name();
  logger(dbg_downtime, basic)
    << " Fixed/Flex:  " << (is_fixed() ? "Fixed\n" : "Flexible\n")
    << " Start:       " << start_time_string << "\n"
       " End:         " << end_time_string << "\n"
       " Duration:    " << hours << "h " << minutes << "m " << seconds << "s\n"
       " Downtime ID: " << get_downtime_id() << "\n"
       " Trigger ID:  " << get_triggered_by();

  /* add a non-persistent comment to the host or service regarding the scheduled outage */
  std::shared_ptr<comment> com{
    new comment(
      comment::host,
      comment::downtime,
      hst->get_name(),
      "",
      time(NULL),
      "(Centreon Engine Process)",
      oss.str(),
      false,
      comment::internal,
      false,
      (time_t)0)};

  comment::comments.insert({com->get_comment_id(), com});
  _comment_id = com->get_comment_id();

  /*** SCHEDULE DOWNTIME - FLEXIBLE (NON-FIXED) DOWNTIME IS HANDLED AT A LATER POINT ***/

  /* only non-triggered downtime is scheduled... */
  if (get_triggered_by() == 0) {
    uint64_t* new_downtime_id{new uint64_t{get_downtime_id()}};
    schedule_new_event(
      EVENT_SCHEDULED_DOWNTIME,
      true,
      get_start_time(),
      false,
      0,
      NULL,
      false,
      (void*)new_downtime_id,
      NULL,
      0);
  }

#ifdef PROBABLY_NOT_NEEDED
  /*** FLEXIBLE DOWNTIME SANITY CHECK - ADDED 02/17/2008 ****/

  /* if host/service is in a non-OK/UP state right now, see if we should start flexible time immediately */
  /* this is new logic added in 3.0rc3 */
  if (!this->fixed) {
      check_pending_flex_host_downtime(hst);
  }
#endif
  return OK;
}

int host_downtime::handle() {
  host* hst{NULL};
  time_t event_time{0L};
  int attr{0};

  logger(dbg_functions, basic)
    << "handle_downtime()";

  host_map::const_iterator it(host::hosts.find(get_hostname()));

  /* find the host or service associated with this downtime */
  if (it == host::hosts.end() || it->second == nullptr)
      return ERROR;

  /* if downtime is flexible and host/svc is in an ok state, don't do anything right now (wait for event handler to kick it off) */
  /* start_flex_downtime variable is set to true by event handler functions */
  if (!is_fixed()) {

    /* we're not supposed to force a start of flex downtime... */
    if (!_start_flex_downtime) {

      /* host is up, so we don't really do anything right now */
      if (hst->get_current_state() == host::state_up) {

        /* increment pending flex downtime counter */
        hst->set_pending_flex_downtime(hst->get_pending_flex_downtime() + 1);
        _incremented_pending_downtime = true;

        /*** SINCE THE FLEX DOWNTIME MAY NEVER START, WE HAVE TO PROVIDE A WAY OF EXPIRING UNUSED DOWNTIME... ***/

        schedule_new_event(
          EVENT_EXPIRE_DOWNTIME,
          true,
          get_end_time() + 1,
          false,
          0,
          NULL,
          false,
          NULL,
          NULL,
          0);
        return OK;
      }
    }
  }

  /* have we come to the end of the scheduled downtime? */
  if (is_in_effect()) {

    /* send data to event broker */
    attr = NEBATTR_DOWNTIME_STOP_NORMAL;
    broker_downtime_data(
      NEBTYPE_DOWNTIME_STOP,
      NEBFLAG_NONE,
      attr,
      get_type(),
      get_hostname().c_str(),
      nullptr,
      _entry_time,
      get_author().c_str(),
      get_comment().c_str(),
      get_start_time(),
      get_end_time(),
      is_fixed(),
      get_triggered_by(),
      get_duration(),
      get_downtime_id(),
      NULL);

    /* decrement the downtime depth variable */
    hst->set_scheduled_downtime_depth(hst->get_scheduled_downtime_depth() - 1);

      if (hst->get_scheduled_downtime_depth() == 0) {

      logger(dbg_downtime, basic)
        << "Host '" << hst->get_name() << "' has exited from a period "
        "of scheduled downtime (id=" << get_downtime_id()
        << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "HOST DOWNTIME ALERT: " << hst->get_name()
        << ";STOPPED; Host has exited from a period of scheduled "
        "downtime";

      /* send a notification */
      hst->notify(
        notifier::notification_downtimeend,
        get_author(),
        get_comment(),
        notifier::notification_option_none);
      }

    /* update the status data */
      hst->update_status(false);

    /* decrement pending flex downtime if necessary */
    if (!is_fixed()
        && _incremented_pending_downtime) {
        if (hst->get_pending_flex_downtime() > 0)
          hst->set_pending_flex_downtime(hst->get_pending_flex_downtime() - 1);
    }

    /* handle (stop) downtime that is triggered by this one */
    while (true) {
      std::multimap<time_t, std::shared_ptr<downtime>>::const_iterator it;
      std::multimap<time_t, std::shared_ptr<downtime>>::const_iterator end{downtime_manager::instance().get_scheduled_downtimes().end()};

      /* list contents might change by recursive calls, so we use this inefficient method to prevent segfaults */
      for (it = downtime_manager::instance().get_scheduled_downtimes().begin();
          it != end;
          ++it) {
        if (it->second->get_triggered_by() == get_downtime_id()) {
          it->second->handle();
          break;
        }
      }

      for (it = downtime_manager::instance().get_scheduled_downtimes().begin();
          it != end;
          ++it) {
        if (it->second->get_triggered_by() == get_downtime_id()) {
          it->second->handle();
          break;
        }
      }

      if (it == end)
        break;
    }

    /* delete downtime entry */
    downtime_manager::instance().delete_host_downtime(get_downtime_id());
  }
  /* else we are just starting the scheduled downtime */
  else {

    /* send data to event broker */
    broker_downtime_data(
      NEBTYPE_DOWNTIME_START,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      get_type(),
      get_hostname().c_str(),
      nullptr,
      _entry_time,
      get_author().c_str(),
      get_comment().c_str(),
      get_start_time(),
      get_end_time(),
      is_fixed(),
      get_triggered_by(),
      get_duration(),
      get_downtime_id(), nullptr);

    if (hst->get_scheduled_downtime_depth() == 0) {

      logger(dbg_downtime, basic)
        << "Host '" << hst->get_name() << "' has entered a period of "
        "scheduled downtime (id=" << get_downtime_id() << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "HOST DOWNTIME ALERT: " << hst->get_name()
        << ";STARTED; Host has entered a period of scheduled downtime";

      /* send a notification */
      hst->notify(
        notifier::notification_downtimestart,
        get_author(),
        get_comment(),
        notifier::notification_option_none);
    }

    /* increment the downtime depth variable */
    hst->set_scheduled_downtime_depth(hst->get_scheduled_downtime_depth() + 1);

    /* set the in effect flag */
    _set_in_effect(true);

    /* update the status data */
    hst->update_status(false);

    /* schedule an event */
    if (!is_fixed())
      event_time
        = (time_t)((uint64_t)time(NULL) + get_duration());
    else
      event_time = get_end_time();
    uint64_t* new_downtime_id{new uint64_t{get_downtime_id()}};
    schedule_new_event(
      EVENT_SCHEDULED_DOWNTIME,
      true,
      event_time,
      false,
      0,
      NULL,
      false,
      (void*)new_downtime_id,
      NULL,
      0);

    /* handle (start) downtime that is triggered by this one */
    std::multimap<time_t, std::shared_ptr<downtime>>::const_iterator it,
      end{downtime_manager::instance().get_scheduled_downtimes().end()};

    for (it = downtime_manager::instance().get_scheduled_downtimes().begin();
        it != end;
        ++it) {
      if (it->second->get_triggered_by() == get_downtime_id())
        it->second->handle();
    }
  }
  return OK;
}

void host_downtime::schedule() {
  downtime_manager::instance().add_downtime(this);

  /* send data to event broker */
  broker_downtime_data(NEBTYPE_DOWNTIME_LOAD, NEBFLAG_NONE, NEBATTR_NONE,
                       HOST_DOWNTIME, _hostname.c_str(), nullptr, _entry_time,
                       _author.c_str(), _comment.c_str(), _start_time, _end_time, _fixed,
                       _triggered_by, _duration, _downtime_id, nullptr);
}
