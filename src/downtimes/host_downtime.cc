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
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/timed_event.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/objects/comment.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"
#include "compatibility/find.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::downtimes;

host_downtime::host_downtime(std::string const& host_name)
 : downtime(HOST_DOWNTIME, host_name) {}

//host_downtime::host_downtime(downtime const& other) {}
//host_downtime::host_downtime(downtime&& other) {}

host_downtime::~host_downtime() {
  /* send data to event broker */
  broker_downtime_data(
    NEBTYPE_DOWNTIME_DELETE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    HOST_DOWNTIME,
    _hostname.c_str(),
    nullptr,
    entry_time,
    author,
    comment,
    start_time,
    end_time,
    fixed,
    triggered_by,
    duration,
    downtime_id,
    nullptr);
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

  umap<unsigned int, std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it(state::instance().hosts().find(get_host_id(get_hostname().c_str())));

  /* delete downtimes with invalid host names */
  if (it == state::instance().hosts().end() || it->second == nullptr)
    retval = true;
  /* delete downtimes that have expired */
  else if (this->end_time < time(NULL))
    retval = true;

  return retval;
}

void host_downtime::retention(std::ostream& os) const {
  os << "hostdowntime {\n";
  os << "host_name=" << get_hostname() << "\n";
  os << "author=" << this->author << "\n"
    "comment=" << this->comment << "\n"
    "duration=" << this->duration << "\n"
    "end_time=" << static_cast<unsigned long>(this->end_time) << "\n"
    "entry_time=" << static_cast<unsigned long>(this->entry_time) << "\n"
    "fixed=" << this->fixed << "\n"
    "start_time=" << static_cast<unsigned long>(this->start_time) << "\n"
    "triggered_by=" << this->triggered_by << "\n"
    "downtime_id=" << this->downtime_id << "\n"
    "}\n";
}

void host_downtime::print(std::ostream& os) const {
  os << "hostdowntime {\n";
  os << "\thost_name=" << get_hostname() << "\n";
  os << "\tdowntime_id=" << downtime_id << "\n"
            "\tentry_time="
         << static_cast<unsigned long>(this->entry_time) << "\n"
            "\tstart_time="
         << static_cast<unsigned long>(this->start_time) << "\n"
            "\tend_time="
         << static_cast<unsigned long>(this->end_time) << "\n"
            "\ttriggered_by="
         << this->triggered_by << "\n"
            "\tfixed="
         << this->fixed << "\n"
            "\tduration="
         << this->duration << "\n"
            "\tauthor="
         << this->author << "\n"
            "\tcomment="
         << this->comment << "\n"
            "\t}\n\n";
}

int host_downtime::unschedule() {
  umap<unsigned int, std::shared_ptr<com::centreon::engine::host>>::const_iterator
  it(state::instance().hosts().find(get_host_id(get_hostname().c_str())));

  /* delete downtimes with invalid host names */
  if (it == state::instance().hosts().end() || it->second == nullptr)
    return ERROR;
  host* hst = it->second.get();

  /* decrement pending flex downtime if necessary ... */
  if (!this->fixed && this->incremented_pending_downtime)
    hst->set_pending_flex_downtime(hst->get_pending_flex_downtime() - 1);

  /* decrement the downtime depth variable and update status data if necessary
   */
  if (this->is_in_effect) {

    /* send data to event broker */
    broker_downtime_data(
      NEBTYPE_DOWNTIME_STOP,
      NEBFLAG_NONE,
      NEBATTR_DOWNTIME_STOP_CANCELLED,
      get_type(),
      get_hostname().c_str(),
      nullptr,
      this->entry_time,
      this->author,
      this->comment,
      this->start_time,
      this->end_time,
      this->fixed,
      this->triggered_by,
      this->duration,
      this->downtime_id,
      nullptr);

    hst->set_scheduled_downtime_depth(hst->get_scheduled_downtime_depth() - 1);
    update_host_status(hst, false);

    /* log a notice - this is parsed by the history CGI */
    if (hst->get_scheduled_downtime_depth() == 0) {
      logger(log_info_message, basic)
        << "HOST DOWNTIME ALERT: " << hst->get_name()
        << ";CANCELLED; Scheduled downtime for host has been "
           "cancelled.";

      /* send a notification */
      host_notification(
        hst,
        NOTIFICATION_DOWNTIMECANCELLED,
        nullptr,
        nullptr,
        NOTIFICATION_OPTION_NONE);
    }
  }
  return OK;
}

int host_downtime::subscribe() {
  char start_time_string[MAX_DATETIME_LENGTH] = "";
  char end_time_string[MAX_DATETIME_LENGTH] = "";
  int hours{0};
  int minutes{0};
  int seconds{0};
  char const* type_string(nullptr);

  umap<unsigned int, std::shared_ptr<com::centreon::engine::host>>::const_iterator
  it(state::instance().hosts().find(get_host_id(get_hostname().c_str())));

  /* find the host or service associated with this downtime */
  if (it == state::instance().hosts().end() || it->second == nullptr)
    return ERROR;

  host* hst = it->second.get();

  /* create the comment */
  get_datetime_string(
    &(this->start_time),
    start_time_string,
    MAX_DATETIME_LENGTH,
    SHORT_DATE_TIME);
  get_datetime_string(
    &(this->end_time),
    end_time_string,
    MAX_DATETIME_LENGTH,
    SHORT_DATE_TIME);
  hours = this->duration / 3600;
  minutes = ((this->duration - (hours * 3600)) / 60);
  seconds = this->duration - (hours * 3600) - (minutes * 60);
  type_string = this->get_type() == HOST_DOWNTIME ? "host" : "service";
  std::ostringstream oss;
  if (this->fixed == true)
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
    << " Fixed/Flex:  " << (this->fixed ? "Fixed\n" : "Flexible\n")
    << " Start:       " << start_time_string << "\n"
       " End:         " << end_time_string << "\n"
       " Duration:    " << hours << "h " << minutes << "m " << seconds << "s\n"
       " Downtime ID: " << this->downtime_id << "\n"
       " Trigger ID:  " << this->triggered_by;

  /* add a non-persistent comment to the host or service regarding the scheduled outage */
    add_new_comment(
      HOST_COMMENT,
      DOWNTIME_COMMENT,
      hst->get_name().c_str(),
      NULL,
      time(NULL),
      "(Centreon Engine Process)",
      oss.str().c_str(),
      0,
      COMMENTSOURCE_INTERNAL,
      false,
      (time_t)0,
      &(this->comment_id));

  /*** SCHEDULE DOWNTIME - FLEXIBLE (NON-FIXED) DOWNTIME IS HANDLED AT A LATER POINT ***/

  /* only non-triggered downtime is scheduled... */
  if (this->triggered_by == 0) {
    unsigned long* new_downtime_id{new unsigned long{downtime_id}};
    schedule_new_event(
      EVENT_SCHEDULED_DOWNTIME,
      true,
      this->start_time,
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
  downtime* this_downtime(NULL);
  host* hst(NULL);
  time_t event_time(0L);
  int attr(0);

  logger(dbg_functions, basic)
    << "handle_downtime()";

  umap<unsigned int, std::shared_ptr<com::centreon::engine::host>>::const_iterator
  it(state::instance().hosts().find(get_host_id(get_hostname().c_str())));

  /* find the host or service associated with this downtime */
  if (it == state::instance().hosts().end() || it->second == nullptr)
      return ERROR;

  /* if downtime if flexible and host/svc is in an ok state, don't do anything right now (wait for event handler to kick it off) */
  /* start_flex_downtime variable is set to true by event handler functions */
  if (!this->fixed) {

    /* we're not supposed to force a start of flex downtime... */
    if (!this->start_flex_downtime) {

      /* host is up or service is ok, so we don't really do anything right now */
      if (hst->get_current_state() == HOST_UP) {

        /* increment pending flex downtime counter */
        hst->set_pending_flex_downtime(hst->get_pending_flex_downtime() + 1);
        this->incremented_pending_downtime = true;

        /*** SINCE THE FLEX DOWNTIME MAY NEVER START, WE HAVE TO PROVIDE A WAY OF EXPIRING UNUSED DOWNTIME... ***/

        schedule_new_event(
          EVENT_EXPIRE_DOWNTIME,
          true,
          this->end_time + 1,
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
  if (this->is_in_effect) {

    /* send data to event broker */
    attr = NEBATTR_DOWNTIME_STOP_NORMAL;
    broker_downtime_data(
      NEBTYPE_DOWNTIME_STOP,
      NEBFLAG_NONE,
      attr,
      this->get_type(),
      get_hostname().c_str(),
      nullptr,
      this->entry_time,
      this->author,
      this->comment,
      this->start_time,
      this->end_time,
      this->fixed,
      this->triggered_by,
      this->duration,
      this->downtime_id,
      NULL);

    /* decrement the downtime depth variable */
    hst->set_scheduled_downtime_depth(hst->get_scheduled_downtime_depth() - 1);

      if (hst->get_scheduled_downtime_depth() == 0) {

      logger(dbg_downtime, basic)
        << "Host '" << hst->get_name() << "' has exited from a period "
        "of scheduled downtime (id=" << this->downtime_id
        << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "HOST DOWNTIME ALERT: " << hst->get_name()
        << ";STOPPED; Host has exited from a period of scheduled "
        "downtime";

      /* send a notification */
      host_notification(
        hst,
        NOTIFICATION_DOWNTIMEEND,
        this->author,
        this->comment,
        NOTIFICATION_OPTION_NONE);
      }

    /* update the status data */
      update_host_status(hst, false);

    /* decrement pending flex downtime if necessary */
    if (!this->fixed
        && this->incremented_pending_downtime) {
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
        if (it->second->triggered_by == this->downtime_id) {
          it->second->handle();
          break;
        }
      }

      for (it = downtime_manager::instance().get_scheduled_downtimes().begin();
          it != end;
          ++it) {
        if (it->second->triggered_by == this->downtime_id) {
          it->second->handle();
          break;
        }
      }

      if (it == end)
        break;
    }

    /* delete downtime entry */
    downtime_manager::instance().delete_host_downtime(this->downtime_id);
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
      this->entry_time,
      this->author,
      this->comment,
      this->start_time,
      this->end_time,
      this->fixed,
      this->triggered_by,
      this->duration,
      this->downtime_id, NULL);

    if (hst->get_scheduled_downtime_depth() == 0) {

      logger(dbg_downtime, basic)
        << "Host '" << hst->get_name() << "' has entered a period of "
        "scheduled downtime (id=" << this->downtime_id << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "HOST DOWNTIME ALERT: " << hst->get_name()
        << ";STARTED; Host has entered a period of scheduled downtime";

      /* send a notification */
      host_notification(
        hst,
        NOTIFICATION_DOWNTIMESTART,
        this->author,
        this->comment,
        NOTIFICATION_OPTION_NONE);
    }

    /* increment the downtime depth variable */
    hst->set_scheduled_downtime_depth(hst->get_scheduled_downtime_depth() + 1);

    /* set the in effect flag */
    this->is_in_effect = true;

    /* update the status data */
    update_host_status(hst, false);

    /* schedule an event */
    if (!this->fixed)
      event_time
        = (time_t)((unsigned long)time(NULL) + this->duration);
    else
      event_time = this->end_time;
    unsigned long* new_downtime_id{new unsigned long{this->downtime_id}};
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
      if (it->second->triggered_by == this->downtime_id)
        it->second->handle();
    }
  }
  return OK;
}
