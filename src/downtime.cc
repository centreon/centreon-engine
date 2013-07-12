/*
** Copyright 2000-2008 Ethan Galstad
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

#include <cstdlib>
#include <cstring>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/comments.hh"
#include "com/centreon/engine/downtime.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/xdddefault.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/******************************************************************/
/**************** INITIALIZATION/CLEANUP FUNCTIONS ****************/
/******************************************************************/

/* initializes scheduled downtime data */
int initialize_downtime_data() {
  return (xdddefault_initialize_downtime_data());
}

/* cleans up scheduled downtime data */
int cleanup_downtime_data() {
  /* free memory allocated to downtime data */
  free_downtime_data();
  return (OK);
}

/******************************************************************/
/********************** SCHEDULING FUNCTIONS **********************/
/******************************************************************/

/* schedules a host or service downtime */
int schedule_downtime(
      int type,
      char const* host_name,
      char const* service_description,
      time_t entry_time,
      char const* author,
      char const* comment_data,
      time_t start_time,
      time_t end_time,
      int fixed,
      unsigned long triggered_by,
      unsigned long duration,
      unsigned long* new_downtime_id) {
  unsigned long downtime_id = 0L;

  logger(dbg_functions, basic)
    << "schedule_downtime()";

  /* don't add old or invalid downtimes */
  if (start_time >= end_time || end_time <= time(NULL))
    return (ERROR);

  /* add a new downtime entry */
  add_new_downtime(
    type,
    host_name,
    service_description,
    entry_time,
    author,
    comment_data,
    start_time,
    end_time,
    fixed,
    triggered_by,
    duration,
    &downtime_id);

  /* register the scheduled downtime */
  register_downtime(type, downtime_id);

  /* return downtime id */
  if (new_downtime_id != NULL)
    *new_downtime_id = downtime_id;
  return (OK);
}

/* unschedules a host or service downtime */
int unschedule_downtime(int type, unsigned long downtime_id) {
  scheduled_downtime* temp_downtime = NULL;
  scheduled_downtime* next_downtime = NULL;
  host* hst = NULL;
  service* svc = NULL;
  timed_event* temp_event = NULL;
  int attr = 0;

  logger(dbg_functions, basic)
    << "unschedule_downtime()";

  /* find the downtime entry in the list in memory */
  if ((temp_downtime = find_downtime(type, downtime_id)) == NULL)
    return (ERROR);

  /* find the host or service associated with this downtime */
  if (temp_downtime->type == HOST_DOWNTIME) {
    if ((hst = find_host(temp_downtime->host_name)) == NULL)
      return (ERROR);
  }
  else if ((svc = find_service(
                    temp_downtime->host_name,
                    temp_downtime->service_description)) == NULL)
    return (ERROR);

  /* decrement pending flex downtime if necessary ... */
  if (temp_downtime->fixed == false
      && temp_downtime->incremented_pending_downtime == true) {
    if (temp_downtime->type == HOST_DOWNTIME)
      hst->pending_flex_downtime--;
    else
      svc->pending_flex_downtime--;
  }

  /* decrement the downtime depth variable and update status data if necessary */
  if (temp_downtime->is_in_effect == true) {

    /* send data to event broker */
    attr = NEBATTR_DOWNTIME_STOP_CANCELLED;
    broker_downtime_data(
      NEBTYPE_DOWNTIME_STOP,
      NEBFLAG_NONE,
      attr,
      temp_downtime->type,
      temp_downtime->host_name,
      temp_downtime->service_description,
      temp_downtime->entry_time,
      temp_downtime->author,
      temp_downtime->comment,
      temp_downtime->start_time,
      temp_downtime->end_time,
      temp_downtime->fixed,
      temp_downtime->triggered_by,
      temp_downtime->duration,
      temp_downtime->downtime_id,
      NULL);

    if (temp_downtime->type == HOST_DOWNTIME) {
      hst->scheduled_downtime_depth--;
      update_host_status(hst, false);

      /* log a notice - this is parsed by the history CGI */
      if (hst->scheduled_downtime_depth == 0) {
        logger(log_info_message, basic)
          << "HOST DOWNTIME ALERT: " << hst->name
          << ";CANCELLED; Scheduled downtime for host has been "
          "cancelled.";

        /* send a notification */
        host_notification(
          hst,
          NOTIFICATION_DOWNTIMECANCELLED,
          NULL,
          NULL,
          NOTIFICATION_OPTION_NONE);
      }
    }
    else {
      svc->scheduled_downtime_depth--;
      update_service_status(svc, false);

      /* log a notice - this is parsed by the history CGI */
      if (svc->scheduled_downtime_depth == 0) {

        logger(log_info_message, basic)
          << "SERVICE DOWNTIME ALERT: " << svc->host_name << ";"
          << svc->description << ";CANCELLED; Scheduled downtime "
          "for service has been cancelled.";

        /* send a notification */
        service_notification(
          svc,
          NOTIFICATION_DOWNTIMECANCELLED,
          NULL,
          NULL,
          NOTIFICATION_OPTION_NONE);
      }
    }
  }

  /* remove scheduled entry from event queue */
  for (temp_event = event_list_high;
       temp_event != NULL;
       temp_event = temp_event->next) {
    if (temp_event->event_type != EVENT_SCHEDULED_DOWNTIME)
      continue;
    if (((unsigned long)temp_event->event_data) == downtime_id)
      break;
  }
  if (temp_event != NULL)
    remove_event(temp_event, &event_list_high, &event_list_high_tail);

  /* delete downtime entry */
  if (temp_downtime->type == HOST_DOWNTIME)
    delete_host_downtime(downtime_id);
  else
    delete_service_downtime(downtime_id);

  /* unschedule all downtime entries that were triggered by this one */
  while (1) {
    for (temp_downtime = scheduled_downtime_list;
         temp_downtime != NULL;
         temp_downtime = next_downtime) {
      next_downtime = temp_downtime->next;
      if (temp_downtime->triggered_by == downtime_id) {
        unschedule_downtime(ANY_DOWNTIME, temp_downtime->downtime_id);
        break;
      }
    }

    if (temp_downtime == NULL)
      break;
  }
  return (OK);
}

/* registers scheduled downtime (schedules it, adds comments, etc.) */
int register_downtime(int type, unsigned long downtime_id) {
  char start_time_string[MAX_DATETIME_LENGTH] = "";
  char end_time_string[MAX_DATETIME_LENGTH] = "";
  scheduled_downtime* temp_downtime = NULL;
  host* hst = NULL;
  service* svc = NULL;
  char const* type_string = NULL;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  unsigned long* new_downtime_id = NULL;

  logger(dbg_functions, basic)
    << "register_downtime()";

  /* find the downtime entry in memory */
  temp_downtime = find_downtime(type, downtime_id);
  if (temp_downtime == NULL)
    return (ERROR);

  /* find the host or service associated with this downtime */
  if (temp_downtime->type == HOST_DOWNTIME) {
    if ((hst = find_host(temp_downtime->host_name)) == NULL)
      return (ERROR);
  }
  else if ((svc = find_service(
                    temp_downtime->host_name,
                    temp_downtime->service_description)) == NULL)
    return (ERROR);

  /* create the comment */
  get_datetime_string(
    &(temp_downtime->start_time),
    start_time_string,
    MAX_DATETIME_LENGTH,
    SHORT_DATE_TIME);
  get_datetime_string(
    &(temp_downtime->end_time),
    end_time_string,
    MAX_DATETIME_LENGTH,
    SHORT_DATE_TIME);
  hours = temp_downtime->duration / 3600;
  minutes = ((temp_downtime->duration - (hours * 3600)) / 60);
  seconds = temp_downtime->duration - (hours * 3600) - (minutes * 60);
  type_string = temp_downtime->type == HOST_DOWNTIME ? "host" : "service";
  std::ostringstream oss;
  if (temp_downtime->fixed == true)
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

  logger(dbg_downtime, basic)
    << "Scheduled Downtime Details:";
  if (temp_downtime->type == HOST_DOWNTIME) {
    logger(dbg_downtime, basic)
      << " Type:        Host Downtime\n"
      " Host:        " << hst->name;
  }
  else {
    logger(dbg_downtime, basic)
      << " Type:        Service Downtime\n"
      " Host:        " << svc->host_name << "\n"
      " Service:     " << svc->description;
  }
  logger(dbg_downtime, basic)
    << " Fixed/Flex:  " << (temp_downtime->fixed == true ? "Fixed\n" : "Flexible\n")
    << " Start:       " << temp_downtime->downtime_id << "\n"
    " End:         " << temp_downtime->downtime_id << "\n"
    " Duration:    " << hours << "h " << minutes << "m " << seconds << "s\n"
    " Downtime ID: " << temp_downtime->downtime_id << "\n"
    " Trigger ID:  " << temp_downtime->triggered_by;

  /* add a non-persistent comment to the host or service regarding the scheduled outage */
  if (temp_downtime->type == SERVICE_DOWNTIME)
    add_new_comment(
      SERVICE_COMMENT,
      DOWNTIME_COMMENT,
      svc->host_name,
      svc->description,
      time(NULL),
      "(Centreon Engine Process)",
      oss.str().c_str(),
      0,
      COMMENTSOURCE_INTERNAL,
      false,
      (time_t)0,
      &(temp_downtime->comment_id));
  else
    add_new_comment(
      HOST_COMMENT,
      DOWNTIME_COMMENT,
      hst->name,
      NULL,
      time(NULL),
      "(Centreon Engine Process)",
      oss.str().c_str(),
      0,
      COMMENTSOURCE_INTERNAL,
      false,
      (time_t)0,
      &(temp_downtime->comment_id));

  /*** SCHEDULE DOWNTIME - FLEXIBLE (NON-FIXED) DOWNTIME IS HANDLED AT A LATER POINT ***/

  /* only non-triggered downtime is scheduled... */
  if (temp_downtime->triggered_by == 0) {
    new_downtime_id = new unsigned long;
    *new_downtime_id = downtime_id;
    schedule_new_event(
      EVENT_SCHEDULED_DOWNTIME,
      true,
      temp_downtime->start_time,
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
  if (temp_downtime->fixed == false) {
    if (temp_downtime->type == HOST_DOWNTIME)
      check_pending_flex_host_downtime(hst);
    else
      check_pending_flex_service_downtime(svc);
  }
#endif
  return (OK);
}

/* handles scheduled downtime (id passed from timed event queue) */
int handle_scheduled_downtime_by_id(unsigned long downtime_id) {
  scheduled_downtime* temp_downtime = NULL;
  /* find the downtime entry */
  if (!(temp_downtime = find_downtime(ANY_DOWNTIME, downtime_id)))
    return (ERROR);
  /* handle the downtime */
  return (handle_scheduled_downtime(temp_downtime));
}

/* handles scheduled host or service downtime */
int handle_scheduled_downtime(scheduled_downtime*  temp_downtime) {
  scheduled_downtime* this_downtime = NULL;
  host* hst = NULL;
  service* svc = NULL;
  time_t event_time = 0L;
  unsigned long* new_downtime_id = NULL;
  int attr = 0;

  logger(dbg_functions, basic)
    << "handle_scheduled_downtime()";

  if (temp_downtime == NULL)
    return (ERROR);

  /* find the host or service associated with this downtime */
  if (temp_downtime->type == HOST_DOWNTIME) {
    if ((hst = find_host(temp_downtime->host_name)) == NULL)
      return (ERROR);
  }
  else if ((svc = find_service(
                    temp_downtime->host_name,
                    temp_downtime->service_description)) == NULL)
    return (ERROR);

  /* if downtime if flexible and host/svc is in an ok state, don't do anything right now (wait for event handler to kick it off) */
  /* start_flex_downtime variable is set to true by event handler functions */
  if (temp_downtime->fixed == false) {

    /* we're not supposed to force a start of flex downtime... */
    if (temp_downtime->start_flex_downtime == false) {

      /* host is up or service is ok, so we don't really do anything right now */
      if ((temp_downtime->type == HOST_DOWNTIME
           && hst->current_state == HOST_UP)
          || (temp_downtime->type == SERVICE_DOWNTIME
              && svc->current_state == STATE_OK)) {

        /* increment pending flex downtime counter */
        if (temp_downtime->type == HOST_DOWNTIME)
          hst->pending_flex_downtime++;
        else
          svc->pending_flex_downtime++;
        temp_downtime->incremented_pending_downtime = true;

        /*** SINCE THE FLEX DOWNTIME MAY NEVER START, WE HAVE TO PROVIDE A WAY OF EXPIRING UNUSED DOWNTIME... ***/

        schedule_new_event(
          EVENT_EXPIRE_DOWNTIME,
          true,
          temp_downtime->end_time + 1,
          false,
          0,
          NULL,
          false,
          NULL,
          NULL,
          0);
        return (OK);
      }
    }
  }

  /* have we come to the end of the scheduled downtime? */
  if (temp_downtime->is_in_effect == true) {

    /* send data to event broker */
    attr = NEBATTR_DOWNTIME_STOP_NORMAL;
    broker_downtime_data(
      NEBTYPE_DOWNTIME_STOP,
      NEBFLAG_NONE,
      attr,
      temp_downtime->type,
      temp_downtime->host_name,
      temp_downtime->service_description,
      temp_downtime->entry_time,
      temp_downtime->author,
      temp_downtime->comment,
      temp_downtime->start_time,
      temp_downtime->end_time,
      temp_downtime->fixed,
      temp_downtime->triggered_by,
      temp_downtime->duration,
      temp_downtime->downtime_id,
      NULL);

    /* decrement the downtime depth variable */
    if (temp_downtime->type == HOST_DOWNTIME)
      hst->scheduled_downtime_depth--;
    else
      svc->scheduled_downtime_depth--;

    if (temp_downtime->type == HOST_DOWNTIME
        && hst->scheduled_downtime_depth == 0) {

      logger(dbg_downtime, basic)
        << "Host '" << hst->name << "' has exited from a period "
        "of scheduled downtime (id=" << temp_downtime->downtime_id
        << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "HOST DOWNTIME ALERT: " << hst->name
        << ";STOPPED; Host has exited from a period of scheduled "
        "downtime";

      /* send a notification */
      host_notification(
        hst,
        NOTIFICATION_DOWNTIMEEND,
        temp_downtime->author,
        temp_downtime->comment,
        NOTIFICATION_OPTION_NONE);
    }
    else if (temp_downtime->type == SERVICE_DOWNTIME
             && svc->scheduled_downtime_depth == 0) {

      logger(dbg_downtime, basic)
        << "Service '" << svc->description << "' on host '"
        << svc->host_name << "' has exited from a period of "
        "scheduled downtime (id=" << temp_downtime->downtime_id << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "SERVICE DOWNTIME ALERT: " << svc->host_name
        << ";" << svc->description
        << ";STOPPED; Service has exited from a period of scheduled "
        "downtime";

      /* send a notification */
      service_notification(
        svc,
        NOTIFICATION_DOWNTIMEEND,
        temp_downtime->author,
        temp_downtime->comment,
        NOTIFICATION_OPTION_NONE);
    }

    /* update the status data */
    if (temp_downtime->type == HOST_DOWNTIME)
      update_host_status(hst, false);
    else
      update_service_status(svc, false);

    /* decrement pending flex downtime if necessary */
    if (temp_downtime->fixed == false
        && temp_downtime->incremented_pending_downtime == true) {
      if (temp_downtime->type == HOST_DOWNTIME) {
        if (hst->pending_flex_downtime > 0)
          hst->pending_flex_downtime--;
      }
      else if (svc->pending_flex_downtime > 0)
        svc->pending_flex_downtime--;
    }

    /* handle (stop) downtime that is triggered by this one */
    while (1) {

      /* list contents might change by recursive calls, so we use this inefficient method to prevent segfaults */
      for (this_downtime = scheduled_downtime_list;
           this_downtime != NULL;
           this_downtime = this_downtime->next) {
        if (this_downtime->triggered_by == temp_downtime->downtime_id) {
          handle_scheduled_downtime(this_downtime);
          break;
        }
      }

      if (this_downtime == NULL)
        break;
    }

    /* delete downtime entry */
    if (temp_downtime->type == HOST_DOWNTIME)
      delete_host_downtime(temp_downtime->downtime_id);
    else
      delete_service_downtime(temp_downtime->downtime_id);
  }
  /* else we are just starting the scheduled downtime */
  else {

    /* send data to event broker */
    broker_downtime_data(
      NEBTYPE_DOWNTIME_START,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      temp_downtime->type,
      temp_downtime->host_name,
      temp_downtime->service_description,
      temp_downtime->entry_time,
      temp_downtime->author,
      temp_downtime->comment,
      temp_downtime->start_time,
      temp_downtime->end_time,
      temp_downtime->fixed,
      temp_downtime->triggered_by,
      temp_downtime->duration,
      temp_downtime->downtime_id, NULL);

    if (temp_downtime->type == HOST_DOWNTIME
        && hst->scheduled_downtime_depth == 0) {

      logger(dbg_downtime, basic)
        << "Host '" << hst->name << "' has entered a period of "
        "scheduled downtime (id=" << temp_downtime->downtime_id << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "HOST DOWNTIME ALERT: " << hst->name
        << ";STARTED; Host has entered a period of scheduled downtime";

      /* send a notification */
      host_notification(
        hst,
        NOTIFICATION_DOWNTIMESTART,
        temp_downtime->author,
        temp_downtime->comment,
        NOTIFICATION_OPTION_NONE);
    }
    else if (temp_downtime->type == SERVICE_DOWNTIME
             && svc->scheduled_downtime_depth == 0) {

      logger(dbg_downtime, basic)
        << "Service '" << svc->description << "' on host '"
        << svc->host_name << "' has entered a period of scheduled "
        "downtime (id=" << temp_downtime->downtime_id << ").";

      /* log a notice - this one is parsed by the history CGI */
      logger(log_info_message, basic)
        << "SERVICE DOWNTIME ALERT: " << svc->host_name
        << ";" << svc->description
        << ";STARTED; Service has entered a period of scheduled "
        "downtime";

      /* send a notification */
      service_notification(
        svc,
        NOTIFICATION_DOWNTIMESTART,
        temp_downtime->author,
        temp_downtime->comment,
        NOTIFICATION_OPTION_NONE);
    }

    /* increment the downtime depth variable */
    if (temp_downtime->type == HOST_DOWNTIME)
      hst->scheduled_downtime_depth++;
    else
      svc->scheduled_downtime_depth++;

    /* set the in effect flag */
    temp_downtime->is_in_effect = true;

    /* update the status data */
    if (temp_downtime->type == HOST_DOWNTIME)
      update_host_status(hst, false);
    else
      update_service_status(svc, false);

    /* schedule an event */
    if (temp_downtime->fixed == false)
      event_time
        = (time_t)((unsigned long)time(NULL) + temp_downtime->duration);
    else
      event_time = temp_downtime->end_time;
    new_downtime_id = new unsigned long;
    *new_downtime_id = temp_downtime->downtime_id;
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
    for (this_downtime = scheduled_downtime_list;
         this_downtime != NULL;
         this_downtime = this_downtime->next) {
      if (this_downtime->triggered_by == temp_downtime->downtime_id)
        handle_scheduled_downtime(this_downtime);
    }
  }
  return (OK);
}

/* checks for flexible (non-fixed) host downtime that should start now */
int check_pending_flex_host_downtime(host* hst) {
  scheduled_downtime* temp_downtime = NULL;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_pending_flex_host_downtime()";

  if (hst == NULL)
    return (ERROR);

  time(&current_time);

  /* if host is currently up, nothing to do */
  if (hst->current_state == HOST_UP)
    return (OK);

  /* check all downtime entries */
  for (temp_downtime = scheduled_downtime_list;
       temp_downtime != NULL;
       temp_downtime = temp_downtime->next) {
    if (temp_downtime->type != HOST_DOWNTIME
        || temp_downtime->fixed == true
        || temp_downtime->is_in_effect == true
        || temp_downtime->triggered_by != 0)
      continue;

    /* this entry matches our host! */
    if (find_host(temp_downtime->host_name) == hst) {
      /* if the time boundaries are okay, start this scheduled downtime */
      if (temp_downtime->start_time <= current_time
          && current_time <= temp_downtime->end_time) {

        logger(dbg_downtime, basic)
          << "Flexible downtime (id=" << temp_downtime->downtime_id
          << ") for host '" << hst->name << "' starting now...";

        temp_downtime->start_flex_downtime = true;
        handle_scheduled_downtime(temp_downtime);
      }
    }
  }
  return (OK);
}

/* checks for flexible (non-fixed) service downtime that should start now */
int check_pending_flex_service_downtime(service* svc) {
  scheduled_downtime* temp_downtime = NULL;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_pending_flex_service_downtime()";

  if (svc == NULL)
    return (ERROR);

  time(&current_time);

  /* if service is currently ok, nothing to do */
  if (svc->current_state == STATE_OK)
    return (OK);

  /* check all downtime entries */
  for (temp_downtime = scheduled_downtime_list;
       temp_downtime != NULL;
       temp_downtime = temp_downtime->next) {

    if (temp_downtime->type != SERVICE_DOWNTIME
        || temp_downtime->fixed == true
        || temp_downtime->is_in_effect == true
        || temp_downtime->triggered_by != 0)
      continue;

    /* this entry matches our service! */
    if (find_service(
          temp_downtime->host_name,
          temp_downtime->service_description) == svc) {
      /* if the time boundaries are okay, start this scheduled downtime */
      if (temp_downtime->start_time <= current_time
          && current_time <= temp_downtime->end_time) {
        logger(dbg_downtime, basic)
          << "Flexible downtime (id=" << temp_downtime->downtime_id
          << ") for service '" << svc->description << "' on host '"
          << svc->host_name << "' starting now...";

        temp_downtime->start_flex_downtime = true;
        handle_scheduled_downtime(temp_downtime);
      }
    }
  }
  return (OK);
}

/* checks for (and removes) expired downtime entries */
int check_for_expired_downtime() {
  scheduled_downtime* temp_downtime = NULL;
  scheduled_downtime* next_downtime = NULL;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_for_expired_downtime()";

  time(&current_time);

  /* check all downtime entries... */
  for (temp_downtime = scheduled_downtime_list;
       temp_downtime != NULL;
       temp_downtime = next_downtime) {
    next_downtime = temp_downtime->next;

    /* this entry should be removed */
    if (temp_downtime->is_in_effect == false
        && temp_downtime->end_time < current_time) {
      logger(dbg_downtime, basic)
        << "Expiring "
        << (temp_downtime->type == HOST_DOWNTIME ? "host" : "service")
        << " downtime (id=" << temp_downtime->downtime_id << ")...";

      /* delete the downtime entry */
      if (temp_downtime->type == HOST_DOWNTIME)
        delete_host_downtime(temp_downtime->downtime_id);
      else
        delete_service_downtime(temp_downtime->downtime_id);
    }
  }
  return (OK);
}

/******************************************************************/
/************************* SAVE FUNCTIONS *************************/
/******************************************************************/

/* save a host or service downtime */
int add_new_downtime(
      int type,
      char const* host_name,
      char const* service_description,
      time_t entry_time,
      char const* author,
      char const* comment_data,
      time_t start_time,
      time_t end_time,
      int fixed,
      unsigned long triggered_by,
      unsigned long duration,
      unsigned long* downtime_id) {
  if (type == HOST_DOWNTIME)
    return (add_new_host_downtime(
              host_name,
              entry_time,
              author,
              comment_data,
              start_time,
              end_time,
              fixed,
              triggered_by,
              duration,
              downtime_id));
  return (add_new_service_downtime(
            host_name,
            service_description,
            entry_time,
            author,
            comment_data,
            start_time,
            end_time,
            fixed,
            triggered_by,
            duration,
            downtime_id));
}

/* saves a host downtime entry */
int add_new_host_downtime(
      char const* host_name,
      time_t entry_time,
      char const* author,
      char const* comment_data,
      time_t start_time,
      time_t end_time,
      int fixed,
      unsigned long triggered_by,
      unsigned long duration,
      unsigned long* downtime_id) {
  int result = OK;
  unsigned long new_downtime_id = 0L;

  if (host_name == NULL)
    return (ERROR);

  result = xdddefault_add_new_host_downtime(
             host_name,
             entry_time,
             author,
             comment_data,
             start_time,
             end_time,
             fixed,
             triggered_by,
             duration,
             &new_downtime_id);

  /* save downtime id */
  if (downtime_id != NULL)
    *downtime_id = new_downtime_id;

  /* send data to event broker */
  broker_downtime_data(
    NEBTYPE_DOWNTIME_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    HOST_DOWNTIME,
    host_name,
    NULL,
    entry_time,
    author,
    comment_data,
    start_time,
    end_time,
    fixed,
    triggered_by,
    duration,
    new_downtime_id,
    NULL);
  return (result);
}

/* saves a service downtime entry */
int add_new_service_downtime(
      char const* host_name,
      char const* service_description,
      time_t entry_time,
      char const* author,
      char const* comment_data,
      time_t start_time,
      time_t end_time,
      int fixed,
      unsigned long triggered_by,
      unsigned long duration,
      unsigned long* downtime_id) {
  int result = OK;
  unsigned long new_downtime_id = 0L;

  if (host_name == NULL || service_description == NULL)
    return (ERROR);

  result = xdddefault_add_new_service_downtime(
             host_name,
             service_description,
             entry_time,
             author,
             comment_data,
             start_time,
             end_time,
             fixed,
             triggered_by,
             duration,
             &new_downtime_id);

  /* save downtime id */
  if (downtime_id != NULL)
    *downtime_id = new_downtime_id;

  /* send data to event broker */
  broker_downtime_data(
    NEBTYPE_DOWNTIME_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    SERVICE_DOWNTIME,
    host_name,
    service_description,
    entry_time,
    author,
    comment_data,
    start_time,
    end_time,
    fixed,
    triggered_by,
    duration,
    new_downtime_id,
    NULL);
  return (result);
}

/******************************************************************/
/*********************** DELETION FUNCTIONS ***********************/
/******************************************************************/

/* deletes a scheduled host or service downtime entry from the list in memory */
int delete_downtime(int type, unsigned long downtime_id) {
  scheduled_downtime* this_downtime = NULL;
  scheduled_downtime* last_downtime = NULL;
  scheduled_downtime* next_downtime = NULL;

  /* find the downtime we should remove */
  for (this_downtime = scheduled_downtime_list,
         last_downtime = scheduled_downtime_list;
       this_downtime != NULL;
       this_downtime = next_downtime) {
    next_downtime = this_downtime->next;

    /* we found the downtime we should delete */
    if (this_downtime->downtime_id == downtime_id
        && this_downtime->type == type)
      break;

    last_downtime = this_downtime;
  }

  if (this_downtime == NULL)
    return (ERROR);

  /* remove the downtime from the list in memory */
  /* first remove the comment associated with this downtime */
  if (this_downtime->type == HOST_DOWNTIME)
    delete_host_comment(this_downtime->comment_id);
  else
    delete_service_comment(this_downtime->comment_id);

  /* send data to event broker */
  broker_downtime_data(
    NEBTYPE_DOWNTIME_DELETE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    type,
    this_downtime->host_name,
    this_downtime->service_description,
    this_downtime->entry_time,
    this_downtime->author,
    this_downtime->comment,
    this_downtime->start_time,
    this_downtime->end_time,
    this_downtime->fixed,
    this_downtime->triggered_by,
    this_downtime->duration,
    downtime_id,
    NULL);

  if (scheduled_downtime_list == this_downtime)
    scheduled_downtime_list = this_downtime->next;
  else
    last_downtime->next = next_downtime;

  /* free memory */
  delete[] this_downtime->host_name;
  delete[] this_downtime->service_description;
  delete[] this_downtime->author;
  delete[] this_downtime->comment;
  delete this_downtime;

  return (OK);
}

/* deletes a scheduled host downtime entry */
int delete_host_downtime(unsigned long downtime_id) {
  /* delete the downtime from memory */
  delete_downtime(HOST_DOWNTIME, downtime_id);
  return (xdddefault_delete_host_downtime(downtime_id));
}

/* deletes a scheduled service downtime entry */
int delete_service_downtime(unsigned long downtime_id) {
  /* delete the downtime from memory */
  delete_downtime(SERVICE_DOWNTIME, downtime_id);
  return (xdddefault_delete_service_downtime(downtime_id));
}

/*
** Deletes all host and service downtimes on a host by hostname,
** optionally filtered by service description, start time and comment.
** All char* must be set or NULL - "" will silently fail to match.
** Returns number deleted.
*/
int delete_downtime_by_hostname_service_description_start_time_comment(
      char const* hostname,
      char const* service_description,
      time_t start_time,
      char const* comment) {
  scheduled_downtime* temp_downtime;
  scheduled_downtime* next_downtime;
  int deleted(0);

  /* Do not allow deletion of everything - must have at least 1 filter on. */
  if ((NULL == hostname)
      && (NULL == service_description)
      && (0 == start_time)
      && (NULL == comment))
    return (deleted);

  for (temp_downtime = scheduled_downtime_list;
       temp_downtime != NULL;
       temp_downtime = next_downtime) {
    next_downtime = temp_downtime->next;

    if ((start_time != 0) && (temp_downtime->start_time != start_time))
      continue;
    if ((comment != NULL)
        && (strcmp(temp_downtime->comment, comment) != 0))
      continue;
    if (HOST_DOWNTIME == temp_downtime->type) {
      /* If service is specified, then do not delete the host downtime. */
      if (service_description != NULL)
	continue;
      if ((hostname != NULL)
	  && (strcmp(temp_downtime->host_name, hostname) != 0))
	continue;
    }
    else if (SERVICE_DOWNTIME == temp_downtime->type) {
      if ((hostname != NULL)
          && (strcmp(temp_downtime->host_name, hostname) != 0))
	continue;
      if ((service_description != NULL)
	  && (strcmp(
                temp_downtime->service_description,
                service_description) != 0))
	continue;
    }

    unschedule_downtime(
      temp_downtime->type,
      temp_downtime->downtime_id);
    ++deleted;
  }
  return (deleted);
}

/******************************************************************/
/******************** ADDITION FUNCTIONS **************************/
/******************************************************************/

/* adds a host downtime entry to the list in memory */
int add_host_downtime(
      char const* host_name,
      time_t entry_time,
      char const* author,
      char const* comment_data,
      time_t start_time,
      time_t end_time,
      int fixed,
      unsigned long triggered_by,
      unsigned long duration,
      unsigned long downtime_id) {
  return (add_downtime(
            HOST_DOWNTIME,
            host_name,
            NULL,
            entry_time,
            author,
            comment_data,
            start_time,
            end_time,
            fixed,
            triggered_by,
            duration,
            downtime_id));
}

/* adds a service downtime entry to the list in memory */
int add_service_downtime(
      char const* host_name,
      char const* svc_description,
      time_t entry_time,
      char const* author,
      char const* comment_data,
      time_t start_time,
      time_t end_time,
      int fixed,
      unsigned long triggered_by,
      unsigned long duration,
      unsigned long downtime_id) {
  return (add_downtime(
            SERVICE_DOWNTIME,
            host_name,
            svc_description,
            entry_time, author,
            comment_data,
            start_time,
            end_time,
            fixed,
            triggered_by,
            duration,
            downtime_id));
}

/* adds a host or service downtime entry to the list in memory */
int add_downtime(
      int downtime_type,
      char const* host_name,
      char const* svc_description,
      time_t entry_time,
      char const* author,
      char const* comment_data,
      time_t start_time,
      time_t end_time,
      int fixed,
      unsigned long triggered_by,
      unsigned long duration,
      unsigned long downtime_id) {

  /* don't add triggered downtimes that don't have a valid parent */
  if (triggered_by > 0
      && find_downtime(ANY_DOWNTIME, triggered_by) == NULL)
    return (ERROR);

  /* we don't have enough info */
  if (host_name == NULL
      || (downtime_type == SERVICE_DOWNTIME && svc_description == NULL))
    return (ERROR);

  /* allocate memory for the downtime */
  scheduled_downtime* new_downtime(new scheduled_downtime);
  memset(new_downtime, 0, sizeof(*new_downtime));

  /* duplicate vars */
  new_downtime->host_name = string::dup(host_name);
  if (downtime_type == SERVICE_DOWNTIME)
    new_downtime->service_description = string::dup(svc_description);
  if (author)
    new_downtime->author = string::dup(author);
  if (comment_data)
    new_downtime->comment = string::dup(comment_data);

  new_downtime->type = downtime_type;
  new_downtime->entry_time = entry_time;
  new_downtime->start_time = start_time;
  new_downtime->end_time = end_time;
  new_downtime->fixed = (fixed > 0) ? true : false;
  new_downtime->triggered_by = triggered_by;
  new_downtime->duration = duration;
  new_downtime->downtime_id = downtime_id;

  if (defer_downtime_sorting) {
    new_downtime->next = scheduled_downtime_list;
    scheduled_downtime_list = new_downtime;
  }
  else {
    /* add new downtime to downtime list, sorted by start time */
    scheduled_downtime* last_downtime(scheduled_downtime_list);
    scheduled_downtime* temp_downtime(NULL);
    for (temp_downtime = scheduled_downtime_list;
         temp_downtime != NULL;
         temp_downtime = temp_downtime->next) {
      if (new_downtime->start_time < temp_downtime->start_time) {
        new_downtime->next = temp_downtime;
        if (temp_downtime == scheduled_downtime_list)
          scheduled_downtime_list = new_downtime;
        else
          last_downtime->next = new_downtime;
        break;
      }
      else
        last_downtime = temp_downtime;
    }
    if (scheduled_downtime_list == NULL) {
      new_downtime->next = NULL;
      scheduled_downtime_list = new_downtime;
    }
    else if (temp_downtime == NULL) {
      new_downtime->next = NULL;
      last_downtime->next = new_downtime;
    }
  }

  /* send data to event broker */
  broker_downtime_data(
    NEBTYPE_DOWNTIME_LOAD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    downtime_type,
    host_name,
    svc_description,
    entry_time,
    author,
    comment_data,
    start_time,
    end_time,
    fixed,
    triggered_by,
    duration,
    downtime_id,
    NULL);
  return (OK);
}

static int downtime_compar(void const* p1, void const* p2) {
  scheduled_downtime* d1 = *(scheduled_downtime**)p1;
  scheduled_downtime* d2 = *(scheduled_downtime**)p2;
  return ((d1->start_time < d2->start_time)
          ? -1
          : (d1->start_time - d2->start_time));
}

int sort_downtime() {
  if (!defer_downtime_sorting)
    return (OK);
  defer_downtime_sorting = 0;

  unsigned long i = 0;
  unsigned long unsorted_downtimes = 0;
  scheduled_downtime* temp_downtime = scheduled_downtime_list;
  while (temp_downtime != NULL) {
    temp_downtime = temp_downtime->next;
    unsorted_downtimes++;
  }

  if (!unsorted_downtimes)
    return (OK);

  scheduled_downtime** array
    = new scheduled_downtime*[unsorted_downtimes];

  while (scheduled_downtime_list) {
    array[i++] = scheduled_downtime_list;
    scheduled_downtime_list = scheduled_downtime_list->next;
  }

  qsort((void*)array, i, sizeof(*array), downtime_compar);
  scheduled_downtime_list = temp_downtime = array[0];
  for (i = 1; i < unsorted_downtimes; i++) {
    temp_downtime->next = array[i];
    temp_downtime = temp_downtime->next;
  }
  temp_downtime->next = NULL;
  delete[] array;
  return (OK);
}

/******************************************************************/
/************************ SEARCH FUNCTIONS ************************/
/******************************************************************/

/* finds a specific downtime entry */
scheduled_downtime* find_downtime(int type, unsigned long downtime_id) {
  scheduled_downtime* temp_downtime = NULL;

  for (temp_downtime = scheduled_downtime_list;
       temp_downtime != NULL;
       temp_downtime = temp_downtime->next) {
    if (type != ANY_DOWNTIME && temp_downtime->type != type)
      continue;
    if (temp_downtime->downtime_id == downtime_id)
      return (temp_downtime);
  }
  return (NULL);
}

/* finds a specific host downtime entry */
scheduled_downtime* find_host_downtime(unsigned long downtime_id) {
  return (find_downtime(HOST_DOWNTIME, downtime_id));
}

/* finds a specific service downtime entry */
scheduled_downtime* find_service_downtime(unsigned long downtime_id) {
  return (find_downtime(SERVICE_DOWNTIME, downtime_id));
}

/******************************************************************/
/********************* CLEANUP FUNCTIONS **************************/
/******************************************************************/

/* frees memory allocated for the scheduled downtime data */
void free_downtime_data() {
  scheduled_downtime* this_downtime = NULL;
  scheduled_downtime* next_downtime = NULL;

  /* free memory for the scheduled_downtime list */
  for (this_downtime = scheduled_downtime_list;
       this_downtime != NULL;
       this_downtime = next_downtime) {
    next_downtime = this_downtime->next;
    delete[] this_downtime->host_name;
    delete[] this_downtime->service_description;
    delete[] this_downtime->author;
    delete[] this_downtime->comment;
    delete this_downtime;
  }

  /* reset list pointer */
  scheduled_downtime_list = NULL;
  return;
}
