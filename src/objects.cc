/*
** Copyright 1999-2008 Ethan Galstad
** Copyright 2011      Merethis
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

#include <stdio.h>
#include <string.h>
#include "xodtemplate.hh"
#include "globals.hh"
#include "skiplist.hh"
#include "logging.hh"
#include "engine.hh"
#include "objects.hh"
#include "logging/logger.hh"

using namespace com::centreon::engine::logging;

/******************************************************************/
/******* TOP-LEVEL HOST CONFIGURATION DATA INPUT FUNCTION *********/
/******************************************************************/

/* read all host configuration data from external source */
int read_object_config_data(char* main_config_file,
			    int options,
			    int cache,
			    int precache) {
  /* initialize object skiplists */
  init_object_skiplists();

  /* read in data from all text host config files (template-based) */
  return (xodtemplate_read_config_data(main_config_file, options, cache, precache) != OK);
}

/******************************************************************/
/******************** SKIPLIST FUNCTIONS **************************/
/******************************************************************/

int init_object_skiplists(void) {
  unsigned int x = 0;

  for (x = 0; x < NUM_OBJECT_SKIPLISTS; x++)
    object_skiplists[x] = NULL;

  object_skiplists[HOST_SKIPLIST] = skiplist_new(15, 0.5, FALSE, FALSE, skiplist_compare_host);
  object_skiplists[SERVICE_SKIPLIST] = skiplist_new(15, 0.5, FALSE, FALSE, skiplist_compare_service);

  object_skiplists[COMMAND_SKIPLIST] = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_command);
  object_skiplists[TIMEPERIOD_SKIPLIST] = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_timeperiod);
  object_skiplists[CONTACT_SKIPLIST] = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_contact);
  object_skiplists[CONTACTGROUP_SKIPLIST] = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_contactgroup);
  object_skiplists[HOSTGROUP_SKIPLIST] = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_hostgroup);
  object_skiplists[SERVICEGROUP_SKIPLIST] = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_servicegroup);

  object_skiplists[HOSTESCALATION_SKIPLIST] = skiplist_new(15, 0.5, TRUE, FALSE, skiplist_compare_hostescalation);
  object_skiplists[SERVICEESCALATION_SKIPLIST] = skiplist_new(15, 0.5, TRUE, FALSE, skiplist_compare_serviceescalation);
  object_skiplists[HOSTDEPENDENCY_SKIPLIST] = skiplist_new(15, 0.5, TRUE, FALSE, skiplist_compare_hostdependency);
  object_skiplists[SERVICEDEPENDENCY_SKIPLIST] = skiplist_new(15, 0.5, TRUE, FALSE, skiplist_compare_servicedependency);

  return (OK);
}

int free_object_skiplists(void) {
  unsigned int x = 0;

  for (x = 0; x < NUM_OBJECT_SKIPLISTS; x++)
    skiplist_free(&object_skiplists[x]);
  return (OK);
}

int skiplist_compare_text(const char* val1a,
			  const char* val1b,
                          const char* val2a,
			  const char* val2b) {
  int result = 0;

  /* check first name */
  if (val1a == NULL && val2a == NULL)
    result = 0;
  else if (val1a == NULL)
    result = 1;
  else if (val2a == NULL)
    result = -1;
  else
    result = strcmp(val1a, val2a);

  /* check second name if necessary */
  if (result == 0) {
    if (val1b == NULL && val2b == NULL)
      result = 0;
    else if (val1b == NULL)
      result = 1;
    else if (val2b == NULL)
      result = -1;
    else
      result = strcmp(val1b, val2b);
  }

  return (result);
}

int skiplist_compare_host(void const* a, void const* b) {
  host const* oa = static_cast<host const*>(a);
  host const* ob = static_cast<host const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int skiplist_compare_service(void const* a, void const* b) {
  service const* oa = static_cast<service const*>(a);
  service const* ob = static_cast<service const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->host_name,
				oa->description,
				ob->host_name,
				ob->description));
}

int skiplist_compare_command(void const* a, void const* b) {
  command const* oa = static_cast<command const*>(a);
  command const* ob = static_cast<command const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int skiplist_compare_timeperiod(void const* a, void const* b) {
  timeperiod const* oa = static_cast<timeperiod const*>(a);
  timeperiod const* ob = static_cast<timeperiod const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int skiplist_compare_contact(void const* a, void const* b) {
  contact const* oa = static_cast<contact const*>(a);
  contact const* ob = static_cast<contact const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int skiplist_compare_contactgroup(void const* a, void const* b) {
  contactgroup const* oa = static_cast<contactgroup const*>(a);
  contactgroup const* ob = static_cast<contactgroup const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->group_name, NULL, ob->group_name, NULL));
}

int skiplist_compare_hostgroup(void const* a, void const* b) {
  hostgroup const* oa = static_cast<hostgroup const*>(a);
  hostgroup const* ob = static_cast<hostgroup const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->group_name, NULL, ob->group_name, NULL));
}

int skiplist_compare_servicegroup(void const* a, void const* b) {
  servicegroup const* oa = static_cast<servicegroup const*>(a);
  servicegroup const* ob = static_cast<servicegroup const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->group_name, NULL, ob->group_name, NULL));
}

int skiplist_compare_hostescalation(void const* a, void const* b) {
  hostescalation const* oa = static_cast<hostescalation const*>(a);
  hostescalation const* ob = static_cast<hostescalation const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->host_name, NULL, ob->host_name, NULL));
}

int skiplist_compare_serviceescalation(void const* a, void const* b) {
  serviceescalation const* oa = static_cast<serviceescalation const*>(a);
  serviceescalation const* ob = static_cast<serviceescalation const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->host_name,
				oa->description,
				ob->host_name,
				ob->description));
}


int skiplist_compare_hostdependency(void const* a, void const* b) {
  hostdependency const* oa = static_cast<hostdependency const*>(a);
  hostdependency const* ob = static_cast<hostdependency const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->dependent_host_name,
				NULL,
				ob->dependent_host_name,
				NULL));
}

int skiplist_compare_servicedependency(void const* a, void const* b) {
  servicedependency const* oa = static_cast<servicedependency const*>(a);
  servicedependency const* ob = static_cast<servicedependency const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->dependent_host_name,
				oa->dependent_service_description,
				ob->dependent_host_name,
				ob->dependent_service_description));
}

int get_host_count(void) {
  if (object_skiplists[HOST_SKIPLIST])
    return (object_skiplists[HOST_SKIPLIST]->items);
  return (0);
}

int get_service_count(void) {
  if (object_skiplists[SERVICE_SKIPLIST])
    return (object_skiplists[SERVICE_SKIPLIST]->items);
  return (0);
}

/******************************************************************/
/**************** OBJECT ADDITION FUNCTIONS ***********************/
/******************************************************************/

/* add a new timeperiod to the list in memory */
timeperiod* add_timeperiod(char const* name, char const* alias) {
  timeperiod* new_timeperiod = NULL;
  int result = OK;

  /* make sure we have the data we need */
  if ((name == NULL || !strcmp(name, ""))
      || (alias == NULL || !strcmp(alias, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Name or alias for timeperiod is NULL\n");
    return (NULL);
  }

  /* allocate memory for the new timeperiod */
  new_timeperiod = new timeperiod;
  memset(new_timeperiod, 0, sizeof(*new_timeperiod));

  /* copy string vars */
  new_timeperiod->name = my_strdup(name);
  new_timeperiod->alias = my_strdup(alias);

  /* add new timeperiod to skiplist */
  result = skiplist_insert(object_skiplists[TIMEPERIOD_SKIPLIST], (void*)new_timeperiod);
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Timeperiod '%s' has already been defined\n", name);
    result = ERROR;
    break;

  case SKIPLIST_OK:
    result = OK;
    break;

  default:
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Could not add timeperiod '%s' to skiplist\n", name);
    result = ERROR;
    break;
  }

  /* handle errors */
  if (result == ERROR) {
    delete[] new_timeperiod->alias;
    delete[] new_timeperiod->name;
    delete new_timeperiod;
    return (NULL);
  }

  /* timeperiods are registered alphabetically, so add new items to tail of list */
  if (timeperiod_list == NULL) {
    timeperiod_list = new_timeperiod;
    timeperiod_list_tail = timeperiod_list;
  }
  else {
    timeperiod_list_tail->next = new_timeperiod;
    timeperiod_list_tail = new_timeperiod;
  }

  return (new_timeperiod);
}

/* adds a new exclusion to a timeperiod */
timeperiodexclusion* add_exclusion_to_timeperiod(timeperiod* period, char const* name) {
  timeperiodexclusion* new_timeperiodexclusion = NULL;

  /* make sure we have enough data */
  if (period == NULL || name == NULL)
    return (NULL);

  new_timeperiodexclusion = new timeperiodexclusion;
  new_timeperiodexclusion->timeperiod_name = my_strdup(name);

  new_timeperiodexclusion->next = period->exclusions;
  period->exclusions = new_timeperiodexclusion;

  return (new_timeperiodexclusion);
}

/* add a new timerange to a timeperiod */
timerange* add_timerange_to_timeperiod(timeperiod* period,
				       int day,
                                       unsigned long start_time,
                                       unsigned long end_time) {
  timerange* new_timerange = NULL;

  /* make sure we have the data we need */
  if (period == NULL)
    return (NULL);

  if (day < 0 || day > 6) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
	  "Error: Day %d is not valid for timeperiod '%s'\n",
	  day, period->name);
    return (NULL);
  }
  if (start_time > 86400) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
	  "Error: Start time %lu on day %d is not valid for timeperiod '%s'\n",
	  start_time, day, period->name);
    return (NULL);
  }
  if (end_time > 86400) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: End time %lu on day %d is not value for timeperiod '%s'\n",
          end_time, day, period->name);
    return (NULL);
  }

  /* allocate memory for the new time range */
  new_timerange = new timerange;

  new_timerange->range_start = start_time;
  new_timerange->range_end = end_time;

  /* add the new time range to the head of the range list for this day */
  new_timerange->next = period->days[day];
  period->days[day] = new_timerange;

  return (new_timerange);
}

/* add a new exception to a timeperiod */
daterange* add_exception_to_timeperiod(timeperiod* period,
				       int type,
                                       int syear,
				       int smon,
				       int smday,
                                       int swday,
				       int swday_offset,
                                       int eyear,
				       int emon,
				       int emday,
                                       int ewday,
				       int ewday_offset,
                                       int skip_interval) {
  daterange* new_daterange = NULL;

  /* make sure we have the data we need */
  if (period == NULL)
    return (NULL);

  /* allocate memory for the date range range */
  new_daterange = new daterange;

  new_daterange->times = NULL;
  new_daterange->next = NULL;

  new_daterange->type = type;
  new_daterange->syear = syear;
  new_daterange->smon = smon;
  new_daterange->smday = smday;
  new_daterange->swday = swday;
  new_daterange->swday_offset = swday_offset;
  new_daterange->eyear = eyear;
  new_daterange->emon = emon;
  new_daterange->emday = emday;
  new_daterange->ewday = ewday;
  new_daterange->ewday_offset = ewday_offset;
  new_daterange->skip_interval = skip_interval;

  /* add the new date range to the head of the range list for this exception type */
  new_daterange->next = period->exceptions[type];
  period->exceptions[type] = new_daterange;

  return (new_daterange);
}

/* add a new timerange to a daterange */
timerange* add_timerange_to_daterange(daterange* drange,
                                      unsigned long start_time,
                                      unsigned long end_time) {
  timerange* new_timerange = NULL;

  /* make sure we have the data we need */
  if (drange == NULL)
    return (NULL);

  if (start_time > 86400) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Start time %lu is not valid for timeperiod\n",
          start_time);
    return (NULL);
  }
  if (end_time > 86400) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: End time %lu is not value for timeperiod\n",
          end_time);
    return (NULL);
  }

  /* allocate memory for the new time range */
  new_timerange = new timerange;

  new_timerange->range_start = start_time;
  new_timerange->range_end = end_time;

  /* add the new time range to the head of the range list for this date range */
  new_timerange->next = drange->times;
  drange->times = new_timerange;

  return (new_timerange);
}

/* add a new host definition */
host* add_host(char const* name,
	       char const* display_name,
	       char const* alias,
               char const* address,
	       char const* check_period,
	       int initial_state,
               double check_interval,
	       double retry_interval,
               int max_attempts,
	       int notify_up,
	       int notify_down,
               int notify_unreachable,
	       int notify_flapping,
               int notify_downtime,
	       double notification_interval,
               double first_notification_delay,
               char const* notification_period,
	       int notifications_enabled,
               char const* check_command,
	       int checks_enabled,
               int accept_passive_checks,
	       char const* event_handler,
               int event_handler_enabled,
	       int flap_detection_enabled,
               double low_flap_threshold,
	       double high_flap_threshold,
               int flap_detection_on_up,
	       int flap_detection_on_down,
               int flap_detection_on_unreachable,
	       int stalk_on_up,
               int stalk_on_down,
	       int stalk_on_unreachable,
               int process_perfdata,
	       int failure_prediction_enabled,
               char const* failure_prediction_options,
	       int check_freshness,
               int freshness_threshold,
	       char const* notes,
	       char const* notes_url,
               char const* action_url,
	       char const* icon_image,
	       char const* icon_image_alt,
               char const* vrml_image,
	       char const* statusmap_image,
	       int x_2d,
               int y_2d,
	       int have_2d_coords,
	       double x_3d,
	       double y_3d,
               double z_3d,
	       int have_3d_coords,
	       int should_be_drawn,
               int retain_status_information,
               int retain_nonstatus_information,
	       int obsess_over_host) {
  host* new_host = NULL;
  int result = OK;
  unsigned int x = 0;

  /* make sure we have the data we need */
  if ((name == NULL || !strcmp(name, ""))
      || (address == NULL || !strcmp(address, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Host name or address is NULL\n");
    return (NULL);
  }

  /* check values */
  if (max_attempts <= 0) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Invalid max_check_attempts value for host '%s'\n",
          name);
    return (NULL);
  }
  if (check_interval < 0) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Invalid check_interval value for host '%s'\n", name);
    return (NULL);
  }
  if (notification_interval < 0) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Invalid notification_interval value for host '%s'\n",
          name);
    return (NULL);
  }
  if (first_notification_delay < 0) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Invalid first_notification_delay value for host '%s'\n",
          name);
    return (NULL);
  }
  if (freshness_threshold < 0) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Invalid freshness_threshold value for host '%s'\n",
          name);
    return (NULL);
  }

  /* allocate memory for a new host */
  new_host = new host;
  memset(new_host, 0, sizeof(*new_host));

  /* duplicate string vars */
  new_host->name = my_strdup(name);
  new_host->display_name = my_strdup((display_name == NULL) ? name : display_name);
  new_host->alias = my_strdup((alias == NULL) ? name : alias);
  new_host->address = my_strdup(address);
  if (check_period)
    new_host->check_period = my_strdup(check_period);
  if (notification_period)
    new_host->notification_period = my_strdup(notification_period);
  if (check_command)
    new_host->host_check_command = my_strdup(check_command);
  if (event_handler)
    new_host->event_handler = my_strdup(event_handler);
  if (failure_prediction_options)
    new_host->failure_prediction_options = my_strdup(failure_prediction_options);
  if (notes)
    new_host->notes = my_strdup(notes);
  if (notes_url)
    new_host->notes_url = my_strdup(notes_url);
  if (action_url)
    new_host->action_url = my_strdup(action_url);
  if (icon_image)
    new_host->icon_image = my_strdup(icon_image);
  if (icon_image_alt)
    new_host->icon_image_alt = my_strdup(icon_image_alt);
  if (vrml_image)
    new_host->vrml_image = my_strdup(vrml_image);
  if (statusmap_image)
    new_host->statusmap_image = my_strdup(statusmap_image);

  /* duplicate non-string vars */
  new_host->max_attempts = max_attempts;
  new_host->check_interval = check_interval;
  new_host->retry_interval = retry_interval;
  new_host->notification_interval = notification_interval;
  new_host->first_notification_delay = first_notification_delay;
  new_host->notify_on_recovery = (notify_up > 0) ? TRUE : FALSE;
  new_host->notify_on_down = (notify_down > 0) ? TRUE : FALSE;
  new_host->notify_on_unreachable = (notify_unreachable > 0) ? TRUE : FALSE;
  new_host->notify_on_flapping = (notify_flapping > 0) ? TRUE : FALSE;
  new_host->notify_on_downtime = (notify_downtime > 0) ? TRUE : FALSE;
  new_host->flap_detection_enabled = (flap_detection_enabled > 0) ? TRUE : FALSE;
  new_host->low_flap_threshold = low_flap_threshold;
  new_host->high_flap_threshold = high_flap_threshold;
  new_host->flap_detection_on_up = (flap_detection_on_up > 0) ? TRUE : FALSE;
  new_host->flap_detection_on_down = (flap_detection_on_down > 0) ? TRUE : FALSE;
  new_host->flap_detection_on_unreachable = (flap_detection_on_unreachable > 0) ? TRUE : FALSE;
  new_host->stalk_on_up = (stalk_on_up > 0) ? TRUE : FALSE;
  new_host->stalk_on_down = (stalk_on_down > 0) ? TRUE : FALSE;
  new_host->stalk_on_unreachable = (stalk_on_unreachable > 0) ? TRUE : FALSE;
  new_host->process_performance_data = (process_perfdata > 0) ? TRUE : FALSE;
  new_host->check_freshness = (check_freshness > 0) ? TRUE : FALSE;
  new_host->freshness_threshold = freshness_threshold;
  new_host->checks_enabled = (checks_enabled > 0) ? TRUE : FALSE;
  new_host->accept_passive_host_checks = (accept_passive_checks > 0) ? TRUE : FALSE;
  new_host->event_handler_enabled = (event_handler_enabled > 0) ? TRUE : FALSE;
  new_host->failure_prediction_enabled = (failure_prediction_enabled > 0) ? TRUE : FALSE;
  new_host->x_2d = x_2d;
  new_host->y_2d = y_2d;
  new_host->have_2d_coords = (have_2d_coords > 0) ? TRUE : FALSE;
  new_host->x_3d = x_3d;
  new_host->y_3d = y_3d;
  new_host->z_3d = z_3d;
  new_host->have_3d_coords = (have_3d_coords > 0) ? TRUE : FALSE;
  new_host->should_be_drawn = (should_be_drawn > 0) ? TRUE : FALSE;
  new_host->obsess_over_host = (obsess_over_host > 0) ? TRUE : FALSE;
  new_host->retain_status_information = (retain_status_information > 0) ? TRUE : FALSE;
  new_host->retain_nonstatus_information = (retain_nonstatus_information > 0) ? TRUE : FALSE;
  new_host->current_state = initial_state;
  new_host->current_event_id = 0L;
  new_host->last_event_id = 0L;
  new_host->current_problem_id = 0L;
  new_host->last_problem_id = 0L;
  new_host->last_state = initial_state;
  new_host->last_hard_state = initial_state;
  new_host->check_type = HOST_CHECK_ACTIVE;
  new_host->last_host_notification = (time_t)0;
  new_host->next_host_notification = (time_t)0;
  new_host->next_check = (time_t)0;
  new_host->should_be_scheduled = TRUE;
  new_host->last_check = (time_t)0;
  new_host->current_attempt = (initial_state == HOST_UP) ? 1 : max_attempts;
  new_host->state_type = HARD_STATE;
  new_host->execution_time = 0.0;
  new_host->is_executing = FALSE;
  new_host->latency = 0.0;
  new_host->last_state_change = (time_t)0;
  new_host->last_hard_state_change = (time_t)0;
  new_host->last_time_up = (time_t)0;
  new_host->last_time_down = (time_t)0;
  new_host->last_time_unreachable = (time_t)0;
  new_host->has_been_checked = FALSE;
  new_host->is_being_freshened = FALSE;
  new_host->problem_has_been_acknowledged = FALSE;
  new_host->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
  new_host->notifications_enabled = (notifications_enabled > 0) ? TRUE : FALSE;
  new_host->notified_on_down = FALSE;
  new_host->notified_on_unreachable = FALSE;
  new_host->current_notification_number = 0;
  new_host->current_notification_id = 0L;
  new_host->no_more_notifications = FALSE;
  new_host->check_flapping_recovery_notification = FALSE;
  new_host->scheduled_downtime_depth = 0;
  new_host->check_options = CHECK_OPTION_NONE;
  new_host->pending_flex_downtime = 0;
  for (x = 0; x < MAX_STATE_HISTORY_ENTRIES; x++)
    new_host->state_history[x] = STATE_OK;
  new_host->state_history_index = 0;
  new_host->last_state_history_update = (time_t)0;
  new_host->is_flapping = FALSE;
  new_host->flapping_comment_id = 0;
  new_host->percent_state_change = 0.0;
  new_host->total_services = 0;
  new_host->total_service_check_interval = 0L;
  new_host->modified_attributes = MODATTR_NONE;
  new_host->circular_path_checked = FALSE;
  new_host->contains_circular_path = FALSE;

  /* add new host to skiplist */
  result = skiplist_insert(object_skiplists[HOST_SKIPLIST], (void*)new_host);
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Host '%s' has already been defined\n", name);
    result = ERROR;
    break;

  case SKIPLIST_OK:
    result = OK;
    break;

  default:
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Could not add host '%s' to skiplist\n", name);
    result = ERROR;
    break;
  }

  /* handle errors */
  if (result == ERROR) {
    delete[] new_host->plugin_output;
    delete[] new_host->long_plugin_output;
    delete[] new_host->perf_data;
    delete[] new_host->statusmap_image;
    delete[] new_host->vrml_image;
    delete[] new_host->icon_image_alt;
    delete[] new_host->icon_image;
    delete[] new_host->action_url;
    delete[] new_host->notes_url;
    delete[] new_host->notes;
    delete[] new_host->failure_prediction_options;
    delete[] new_host->event_handler;
    delete[] new_host->host_check_command;
    delete[] new_host->notification_period;
    delete[] new_host->check_period;
    delete[] new_host->address;
    delete[] new_host->alias;
    delete[] new_host->display_name;
    delete[] new_host->name;
    delete new_host;
    return (NULL);
  }

  /* hosts are sorted alphabetically, so add new items to tail of list */
  if (host_list == NULL) {
    host_list = new_host;
    host_list_tail = host_list;
  }
  else {
    host_list_tail->next = new_host;
    host_list_tail = new_host;
  }

  return (new_host);
}

hostsmember* add_parent_host_to_host(host* hst, char const* host_name) {
  hostsmember* new_hostsmember = NULL;

  /* make sure we have the data we need */
  if (hst == NULL || host_name == NULL || !strcmp(host_name, "")) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Host is NULL or parent host name is NULL\n");
    return (NULL);
  }

  /* a host cannot be a parent/child of itself */
  if (!strcmp(host_name, hst->name)) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Host '%s' cannot be a child/parent of itself\n",
          hst->name);
    return (NULL);
  }

  /* allocate memory */
  new_hostsmember = new hostsmember;
  memset(new_hostsmember, 0, sizeof(*new_hostsmember));

  /* duplicate string vars */
  new_hostsmember->host_name = my_strdup(host_name);

  /* add the parent host entry to the host definition */
  new_hostsmember->next = hst->parent_hosts;
  hst->parent_hosts = new_hostsmember;

  return (new_hostsmember);
}

hostsmember* add_child_link_to_host(host* hst, host* child_ptr) {
  hostsmember* new_hostsmember = NULL;

  /* make sure we have the data we need */
  if (hst == NULL || child_ptr == NULL)
    return (NULL);

  /* allocate memory */
  new_hostsmember = new hostsmember;

  /* initialize values */
  new_hostsmember->host_name = NULL;
  new_hostsmember->host_ptr = child_ptr;

  /* add the child entry to the host definition */
  new_hostsmember->next = hst->child_hosts;
  hst->child_hosts = new_hostsmember;

  return (new_hostsmember);
}

servicesmember* add_service_link_to_host(host* hst, service* service_ptr) {
  servicesmember* new_servicesmember = NULL;

  /* make sure we have the data we need */
  if (hst == NULL || service_ptr == NULL)
    return (NULL);

  /* allocate memory */
  new_servicesmember = new servicesmember;
  memset(new_servicesmember, 0, sizeof(*new_servicesmember));

  /* initialize values */
  new_servicesmember->service_ptr = service_ptr;

  /* add the child entry to the host definition */
  new_servicesmember->next = hst->services;
  hst->services = new_servicesmember;

  return (new_servicesmember);
}

/* add a new contactgroup to a host */
contactgroupsmember* add_contactgroup_to_host(host* hst, char const* group_name) {
  contactgroupsmember* new_contactgroupsmember = NULL;

  if (hst == NULL || (group_name == NULL || !strcmp(group_name, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Host or contactgroup member is NULL\n");
    return (NULL);
  }

  /* allocate memory for a new member */
  new_contactgroupsmember = new contactgroupsmember;
  memset(new_contactgroupsmember, 0, sizeof(*new_contactgroupsmember));

  /* duplicate string vars */
  new_contactgroupsmember->group_name = my_strdup(group_name);

  /* add the new member to the head of the member list */
  new_contactgroupsmember->next = hst->contact_groups;
  hst->contact_groups = new_contactgroupsmember;;

  return (new_contactgroupsmember);
}

/* adds a contact to a host */
contactsmember* add_contact_to_host(host* hst, char const* contact_name) {
  return (add_contact_to_object(&hst->contacts, contact_name));
}

/* adds a custom variable to a host */
customvariablesmember* add_custom_variable_to_host(host* hst, char const* varname, char const* varvalue) {
  return (add_custom_variable_to_object(&hst->custom_variables, varname, varvalue));
}

/* add a new host group to the list in memory */
hostgroup* add_hostgroup(char const* name,
			 char const* alias,
			 char const* notes,
                         char const* notes_url,
			 char const* action_url) {
  hostgroup* new_hostgroup = NULL;
  int result = OK;

  /* make sure we have the data we need */
  if (name == NULL || !strcmp(name, "")) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Hostgroup name is NULL\n");
    return (NULL);
  }

  /* allocate memory */
  new_hostgroup = new hostgroup;
  memset(new_hostgroup, 0, sizeof(*new_hostgroup));

  /* duplicate vars */
  new_hostgroup->group_name = my_strdup(name);
  new_hostgroup->alias = my_strdup((alias == NULL) ? name : alias);
  if (notes)
    new_hostgroup->notes = my_strdup(notes);
  if (notes_url)
    new_hostgroup->notes_url = my_strdup(notes_url);
  if (action_url)
    new_hostgroup->action_url = my_strdup(action_url);

  /* add new host group to skiplist */
  result = skiplist_insert(object_skiplists[HOSTGROUP_SKIPLIST], (void*)new_hostgroup);
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Hostgroup '%s' has already been defined\n", name);
    result = ERROR;
    break;

  case SKIPLIST_OK:
    result = OK;
    break;

  default:
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Could not add hostgroup '%s' to skiplist\n", name);
    result = ERROR;
    break;
  }

  /* handle errors */
  if (result == ERROR) {
    delete[] new_hostgroup->alias;
    delete[] new_hostgroup->group_name;
    delete new_hostgroup;
    return (NULL);
  }

  /* hostgroups are sorted alphabetically, so add new items to tail of list */
  if (hostgroup_list == NULL) {
    hostgroup_list = new_hostgroup;
    hostgroup_list_tail = hostgroup_list;
  }
  else {
    hostgroup_list_tail->next = new_hostgroup;
    hostgroup_list_tail = new_hostgroup;
  }

  return (new_hostgroup);
}

/* add a new host to a host group */
hostsmember* add_host_to_hostgroup(hostgroup* temp_hostgroup, char const* host_name) {
  hostsmember* new_member = NULL;
  hostsmember* last_member = NULL;
  hostsmember* temp_member = NULL;

  /* make sure we have the data we need */
  if (temp_hostgroup == NULL || (host_name == NULL || !strcmp(host_name, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Hostgroup or group member is NULL\n");
    return (NULL);
  }

  /* allocate memory for a new member */
  new_member = new hostsmember;
  memset(new_member, 0, sizeof(*new_member));

  /* duplicate vars */
  new_member->host_name = my_strdup(host_name);

  /* add the new member to the member list, sorted by host name */
  last_member = temp_hostgroup->members;
  for (temp_member = temp_hostgroup->members;
       temp_member != NULL;
       temp_member = temp_member->next) {
    if (strcmp(new_member->host_name, temp_member->host_name) < 0) {
      new_member->next = temp_member;
      if (temp_member == temp_hostgroup->members)
        temp_hostgroup->members = new_member;
      else
        last_member->next = new_member;
      break;
    }
    else
      last_member = temp_member;
  }
  if (temp_hostgroup->members == NULL) {
    new_member->next = NULL;
    temp_hostgroup->members = new_member;
  }
  else if (temp_member == NULL) {
    new_member->next = NULL;
    last_member->next = new_member;
  }

  return (new_member);
}

/* add a new service group to the list in memory */
servicegroup* add_servicegroup(char const* name,
			       char const* alias,
			       char const* notes,
                               char const* notes_url,
			       char const* action_url) {
  servicegroup* new_servicegroup = NULL;
  int result = OK;

  /* make sure we have the data we need */
  if (name == NULL || !strcmp(name, "")) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Servicegroup name is NULL\n");
    return (NULL);
  }

  /* allocate memory */
  new_servicegroup = new servicegroup;
  memset(new_servicegroup, 0, sizeof(*new_servicegroup));

  /* duplicate vars */
  new_servicegroup->group_name = my_strdup(name);
  new_servicegroup->alias = my_strdup((alias == NULL) ? name : alias);
  if (notes) {
    new_servicegroup->notes = my_strdup(notes);
  }
  if (notes_url) {
    new_servicegroup->notes_url = my_strdup(notes_url);
  }
  if (action_url) {
    new_servicegroup->action_url = my_strdup(action_url);
  }

  /* add new service group to skiplist */
  result = skiplist_insert(object_skiplists[SERVICEGROUP_SKIPLIST], (void*)new_servicegroup);
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Servicegroup '%s' has already been defined\n", name);
    result = ERROR;
    break;

  case SKIPLIST_OK:
    result = OK;
    break;

  default:
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Could not add servicegroup '%s' to skiplist\n", name);
    result = ERROR;
    break;
  }

  /* handle errors */
  if (result == ERROR) {
    delete[] new_servicegroup->alias;
    delete[] new_servicegroup->group_name;
    delete new_servicegroup;
    return (NULL);
  }

  /* servicegroups are sorted alphabetically, so add new items to tail of list */
  if (servicegroup_list == NULL) {
    servicegroup_list = new_servicegroup;
    servicegroup_list_tail = servicegroup_list;
  }
  else {
    servicegroup_list_tail->next = new_servicegroup;
    servicegroup_list_tail = new_servicegroup;
  }

  return (new_servicegroup);
}

/* add a new service to a service group */
servicesmember* add_service_to_servicegroup(servicegroup* temp_servicegroup,
                                            char const* host_name,
                                            char const* svc_description) {
  servicesmember* new_member = NULL;
  servicesmember* last_member = NULL;
  servicesmember* temp_member = NULL;

  /* make sure we have the data we need */
  if (temp_servicegroup == NULL
      || (host_name == NULL || !strcmp(host_name, ""))
      || (svc_description == NULL || !strcmp(svc_description, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Servicegroup or group member is NULL\n");
    return (NULL);
  }

  /* allocate memory for a new member */
  new_member = new servicesmember;
  memset(new_member, 0, sizeof(*new_member));

  /* duplicate vars */
  new_member->host_name = my_strdup(host_name);
  new_member->service_description = my_strdup(svc_description);

  /* add new member to member list, sorted by host name then service description */
  last_member = temp_servicegroup->members;
  for (temp_member = temp_servicegroup->members;
       temp_member != NULL;
       temp_member = temp_member->next) {

    if (strcmp(new_member->host_name, temp_member->host_name) < 0) {
      new_member->next = temp_member;
      if (temp_member == temp_servicegroup->members)
        temp_servicegroup->members = new_member;
      else
        last_member->next = new_member;
      break;
    }
    else if (strcmp(new_member->host_name, temp_member->host_name) == 0
             && strcmp(new_member->service_description, temp_member->service_description) < 0) {
      new_member->next = temp_member;
      if (temp_member == temp_servicegroup->members)
        temp_servicegroup->members = new_member;
      else
        last_member->next = new_member;
      break;
    }
    else
      last_member = temp_member;
  }
  if (temp_servicegroup->members == NULL) {
    new_member->next = NULL;
    temp_servicegroup->members = new_member;
  }
  else if (temp_member == NULL) {
    new_member->next = NULL;
    last_member->next = new_member;
  }

  return (new_member);
}

/* add a new contact to the list in memory */
contact* add_contact(char const* name,
		     char const* alias,
		     char const* email,
		     char const* pager,
                     char const* const* addresses,
		     char const* svc_notification_period,
                     char const* host_notification_period,
                     int notify_service_ok,
		     int notify_service_critical,
                     int notify_service_warning,
                     int notify_service_unknown,
                     int notify_service_flapping,
                     int notify_service_downtime,
		     int notify_host_up,
                     int notify_host_down,
		     int notify_host_unreachable,
                     int notify_host_flapping,
		     int notify_host_downtime,
                     int host_notifications_enabled,
                     int service_notifications_enabled,
                     int can_submit_commands,
                     int retain_status_information,
                     int retain_nonstatus_information) {
  contact* new_contact = NULL;
  unsigned int x = 0;
  int result = OK;

  /* make sure we have the data we need */
  if (name == NULL || !strcmp(name, "")) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Contact name is NULL\n");
    return (NULL);
  }

  /* allocate memory for a new contact */
  new_contact = new contact;
  memset(new_contact, 0, sizeof(*new_contact));

  /* duplicate vars */
  new_contact->name = my_strdup(name);
  new_contact->alias = my_strdup((alias == NULL) ? name : alias);
  if (email)
    new_contact->email = my_strdup(email);
  if (pager)
    new_contact->pager = my_strdup(pager);
  if (svc_notification_period)
    new_contact->service_notification_period = my_strdup(svc_notification_period);
  if (host_notification_period)
    new_contact->host_notification_period = my_strdup(host_notification_period);
  if (addresses) {
    for (x = 0; x < MAX_CONTACT_ADDRESSES; x++) {
      if (addresses[x])
        new_contact->address[x] = my_strdup(addresses[x]);
    }
  }

  new_contact->notify_on_service_recovery = (notify_service_ok > 0) ? TRUE : FALSE;
  new_contact->notify_on_service_critical = (notify_service_critical > 0) ? TRUE : FALSE;
  new_contact->notify_on_service_warning = (notify_service_warning > 0) ? TRUE : FALSE;
  new_contact->notify_on_service_unknown = (notify_service_unknown > 0) ? TRUE : FALSE;
  new_contact->notify_on_service_flapping = (notify_service_flapping > 0) ? TRUE : FALSE;
  new_contact->notify_on_service_downtime = (notify_service_downtime > 0) ? TRUE : FALSE;
  new_contact->notify_on_host_recovery = (notify_host_up > 0) ? TRUE : FALSE;
  new_contact->notify_on_host_down = (notify_host_down > 0) ? TRUE : FALSE;
  new_contact->notify_on_host_unreachable = (notify_host_unreachable > 0) ? TRUE : FALSE;
  new_contact->notify_on_host_flapping = (notify_host_flapping > 0) ? TRUE : FALSE;
  new_contact->notify_on_host_downtime = (notify_host_downtime > 0) ? TRUE : FALSE;
  new_contact->host_notifications_enabled = (host_notifications_enabled > 0) ? TRUE : FALSE;
  new_contact->service_notifications_enabled = (service_notifications_enabled > 0) ? TRUE : FALSE;
  new_contact->can_submit_commands = (can_submit_commands > 0) ? TRUE : FALSE;
  new_contact->retain_status_information = (retain_status_information > 0) ? TRUE : FALSE;
  new_contact->retain_nonstatus_information = (retain_nonstatus_information > 0) ? TRUE : FALSE;
  new_contact->last_host_notification = (time_t)0L;
  new_contact->last_service_notification = (time_t)0L;
  new_contact->modified_attributes = MODATTR_NONE;
  new_contact->modified_host_attributes = MODATTR_NONE;
  new_contact->modified_service_attributes = MODATTR_NONE;

  new_contact->host_notification_period_ptr = NULL;
  new_contact->service_notification_period_ptr = NULL;
  new_contact->contactgroups_ptr = NULL;

  /* add new contact to skiplist */
  if (result == OK) {
    result = skiplist_insert(object_skiplists[CONTACT_SKIPLIST], (void*)new_contact);
    switch (result) {
    case SKIPLIST_ERROR_DUPLICATE:
      logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Contact '%s' has already been defined\n", name);
      result = ERROR;
      break;

    case SKIPLIST_OK:
      result = OK;
      break;

    default:
      logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Could not add contact '%s' to skiplist\n", name);
      result = ERROR;
      break;
    }
  }

  /* handle errors */
  if (result == ERROR) {
    for (x = 0; x < MAX_CONTACT_ADDRESSES; x++)
      delete[] new_contact->address[x];
    delete[] new_contact->name;
    delete[] new_contact->alias;
    delete[] new_contact->email;
    delete[] new_contact->pager;
    delete[] new_contact->service_notification_period;
    delete[] new_contact->host_notification_period;
    delete new_contact;
    return (NULL);
  }

  /* contacts are sorted alphabetically, so add new items to tail of list */
  if (contact_list == NULL) {
    contact_list = new_contact;
    contact_list_tail = contact_list;
  }
  else {
    contact_list_tail->next = new_contact;
    contact_list_tail = new_contact;
  }

  return (new_contact);
}

/* adds a host notification command to a contact definition */
commandsmember* add_host_notification_command_to_contact(contact* cntct, char const* command_name) {
  commandsmember* new_commandsmember = NULL;

  /* make sure we have the data we need */
  if (cntct == NULL || (command_name == NULL || !strcmp(command_name, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Contact or host notification command is NULL\n");
    return (NULL);
  }

  /* allocate memory */
  new_commandsmember = new commandsmember;
  memset(new_commandsmember, 0, sizeof(*new_commandsmember));

  /* duplicate vars */
  new_commandsmember->cmd = my_strdup(command_name);

  /* add the notification command */
  new_commandsmember->next = cntct->host_notification_commands;
  cntct->host_notification_commands = new_commandsmember;

  return (new_commandsmember);
}

/* adds a service notification command to a contact definition */
commandsmember* add_service_notification_command_to_contact(contact*  cntct, char const* command_name) {
  commandsmember* new_commandsmember = NULL;

  /* make sure we have the data we need */
  if (cntct == NULL || (command_name == NULL || !strcmp(command_name, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Contact or service notification command is NULL\n");
    return (NULL);
  }

  /* allocate memory */
  new_commandsmember = new commandsmember;
  memset(new_commandsmember, 0, sizeof(*new_commandsmember));

  /* duplicate vars */
  new_commandsmember->cmd = my_strdup(command_name);

  /* add the notification command */
  new_commandsmember->next = cntct->service_notification_commands;
  cntct->service_notification_commands = new_commandsmember;

  return (new_commandsmember);
}

/* adds a custom variable to a contact */
customvariablesmember* add_custom_variable_to_contact(contact* cntct, char const* varname, char const* varvalue) {
  return (add_custom_variable_to_object(&cntct->custom_variables, varname, varvalue));
}

/* add a new contact group to the list in memory */
contactgroup* add_contactgroup(char const* name, char const* alias) {
  contactgroup* new_contactgroup = NULL;
  int result = OK;

  /* make sure we have the data we need */
  if (name == NULL || !strcmp(name, "")) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Contactgroup name is NULL\n");
    return (NULL);
  }

  /* allocate memory for a new contactgroup entry */
  new_contactgroup = new contactgroup;
  memset(new_contactgroup, 0, sizeof(*new_contactgroup));

  /* duplicate vars */
  new_contactgroup->group_name = my_strdup(name);
  new_contactgroup->alias = my_strdup((alias == NULL) ? name : alias);

  /* add new contact group to skiplist */
  result = skiplist_insert(object_skiplists[CONTACTGROUP_SKIPLIST], (void*)new_contactgroup);
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Contactgroup '%s' has already been defined\n", name);
    result = ERROR;
    break;

  case SKIPLIST_OK:
    result = OK;
    break;

  default:
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Could not add contactgroup '%s' to skiplist\n", name);
    result = ERROR;
    break;
  }

  /* handle errors */
  if (result == ERROR) {
    delete[] new_contactgroup->alias;
    delete[] new_contactgroup->group_name;
    delete new_contactgroup;
    return (NULL);
  }

  /* contactgroups are sorted alphabetically, so add new items to tail of list */
  if (contactgroup_list == NULL) {
    contactgroup_list = new_contactgroup;
    contactgroup_list_tail = contactgroup_list;
  }
  else {
    contactgroup_list_tail->next = new_contactgroup;
    contactgroup_list_tail = new_contactgroup;
  }

  return (new_contactgroup);
}

/* add a new member to a contact group */
contactsmember* add_contact_to_contactgroup(contactgroup* grp, char const* contact_name) {
  contactsmember* new_contactsmember = NULL;

  /* make sure we have the data we need */
  if (grp == NULL || (contact_name == NULL || !strcmp(contact_name, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Contactgroup or contact name is NULL\n");
    return (NULL);
  }

  /* allocate memory for a new member */
  new_contactsmember = new contactsmember;
  memset(new_contactsmember, 0, sizeof(*new_contactsmember));

  /* duplicate vars */
  new_contactsmember->contact_name = my_strdup(contact_name);

  /* add the new member to the head of the member list */
  new_contactsmember->next = grp->members;
  grp->members = new_contactsmember;

  return (new_contactsmember);
}

/* add a new service to the list in memory */
service* add_service(char const* host_name,
		     char const* description,
                     char const* display_name,
		     char const* check_period,
                     int initial_state,
		     int max_attempts,
                     int parallelize,
		     int accept_passive_checks,
                     double check_interval,
		     double retry_interval,
                     double notification_interval,
                     double first_notification_delay,
                     char const* notification_period,
		     int notify_recovery,
                     int notify_unknown,
		     int notify_warning,
                     int notify_critical,
		     int notify_flapping,
                     int notify_downtime,
		     int notifications_enabled,
                     int is_volatile,
		     char const* event_handler,
                     int event_handler_enabled,
		     char const* check_command,
                     int checks_enabled,
		     int flap_detection_enabled,
                     double low_flap_threshold,
                     double high_flap_threshold,
                     int flap_detection_on_ok,
                     int flap_detection_on_warning,
                     int flap_detection_on_unknown,
                     int flap_detection_on_critical,
		     int stalk_on_ok,
                     int stalk_on_warning,
		     int stalk_on_unknown,
                     int stalk_on_critical,
		     int process_perfdata,
                     int failure_prediction_enabled,
                     char const* failure_prediction_options,
                     int check_freshness,
		     int freshness_threshold,
                     char const* notes,
		     char const* notes_url,
		     char const* action_url,
                     char const* icon_image,
		     char const* icon_image_alt,
                     int retain_status_information,
                     int retain_nonstatus_information,
                     int obsess_over_service) {
  service* new_service = NULL;
  int result = OK;
  unsigned int x = 0;

  /* make sure we have everything we need */
  if (!description || !description[0]) {
    logger(log_config_error, basic)
      << "error: service description is not set";
    return (NULL);
  }
  else if (!host_name || !host_name[0]) {
    logger(log_config_error, basic) << "error: host name of service '"
      << description << "' is not set";
    return (NULL);
  }
  else if (!check_command || !check_command[0]) {
    logger(log_config_error, basic) << "error: check command of service '"
      << description << "' on host '" << host_name << "' is not set";
    return (NULL);
  }

  /* check values */
  if (max_attempts <= 0
      || check_interval < 0
      || retry_interval <= 0
      || notification_interval < 0) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Invalid max_attempts, check_interval, retry_interval, or notification_interval value for service '%s' on host '%s'\n",
          description, host_name);
    return (NULL);
  }

  if (first_notification_delay < 0) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Invalid first_notification_delay value for service '%s' on host '%s'\n",
          description, host_name);
    return (NULL);
  }

  /* allocate memory */
  new_service = new service;
  memset(new_service, 0, sizeof(*new_service));

  /* duplicate vars */
  new_service->host_name = my_strdup(host_name);
  new_service->description = my_strdup(description);
  new_service->display_name = my_strdup((display_name == NULL) ? description : display_name);
  new_service->service_check_command = my_strdup(check_command);
  if (event_handler)
    new_service->event_handler = my_strdup(event_handler);
  if (notification_period)
    new_service->notification_period = my_strdup(notification_period);
  if (check_period)
    new_service->check_period = my_strdup(check_period);
  if (failure_prediction_options)
    new_service->failure_prediction_options = my_strdup(failure_prediction_options);
  if (notes)
    new_service->notes = my_strdup(notes);
  if (notes_url)
    new_service->notes_url = my_strdup(notes_url);
  if (action_url)
    new_service->action_url = my_strdup(action_url);
  if (icon_image)
    new_service->icon_image = my_strdup(icon_image);
  if (icon_image_alt)
    new_service->icon_image_alt = my_strdup(icon_image_alt);

  new_service->check_interval = check_interval;
  new_service->retry_interval = retry_interval;
  new_service->max_attempts = max_attempts;
  new_service->parallelize = (parallelize > 0) ? TRUE : FALSE;
  new_service->notification_interval = notification_interval;
  new_service->first_notification_delay = first_notification_delay;
  new_service->notify_on_unknown = (notify_unknown > 0) ? TRUE : FALSE;
  new_service->notify_on_warning = (notify_warning > 0) ? TRUE : FALSE;
  new_service->notify_on_critical = (notify_critical > 0) ? TRUE : FALSE;
  new_service->notify_on_recovery = (notify_recovery > 0) ? TRUE : FALSE;
  new_service->notify_on_flapping = (notify_flapping > 0) ? TRUE : FALSE;
  new_service->notify_on_downtime = (notify_downtime > 0) ? TRUE : FALSE;
  new_service->is_volatile = (is_volatile > 0) ? TRUE : FALSE;
  new_service->flap_detection_enabled = (flap_detection_enabled > 0) ? TRUE : FALSE;
  new_service->low_flap_threshold = low_flap_threshold;
  new_service->high_flap_threshold = high_flap_threshold;
  new_service->flap_detection_on_ok = (flap_detection_on_ok > 0) ? TRUE : FALSE;
  new_service->flap_detection_on_warning = (flap_detection_on_warning > 0) ? TRUE : FALSE;
  new_service->flap_detection_on_unknown = (flap_detection_on_unknown > 0) ? TRUE : FALSE;
  new_service->flap_detection_on_critical = (flap_detection_on_critical > 0) ? TRUE : FALSE;
  new_service->stalk_on_ok = (stalk_on_ok > 0) ? TRUE : FALSE;
  new_service->stalk_on_warning = (stalk_on_warning > 0) ? TRUE : FALSE;
  new_service->stalk_on_unknown = (stalk_on_unknown > 0) ? TRUE : FALSE;
  new_service->stalk_on_critical = (stalk_on_critical > 0) ? TRUE : FALSE;
  new_service->process_performance_data = (process_perfdata > 0) ? TRUE : FALSE;
  new_service->check_freshness = (check_freshness > 0) ? TRUE : FALSE;
  new_service->freshness_threshold = freshness_threshold;
  new_service->accept_passive_service_checks = (accept_passive_checks > 0) ? TRUE : FALSE;
  new_service->event_handler_enabled = (event_handler_enabled > 0) ? TRUE : FALSE;
  new_service->checks_enabled = (checks_enabled > 0) ? TRUE : FALSE;
  new_service->retain_status_information = (retain_status_information > 0) ? TRUE : FALSE;
  new_service->retain_nonstatus_information = (retain_nonstatus_information > 0) ? TRUE : FALSE;
  new_service->notifications_enabled = (notifications_enabled > 0) ? TRUE : FALSE;
  new_service->obsess_over_service = (obsess_over_service > 0) ? TRUE : FALSE;
  new_service->failure_prediction_enabled = (failure_prediction_enabled > 0) ? TRUE : FALSE;
  new_service->problem_has_been_acknowledged = FALSE;
  new_service->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
  new_service->check_type = SERVICE_CHECK_ACTIVE;
  new_service->current_attempt = (initial_state == STATE_OK) ? 1 : max_attempts;
  new_service->current_state = initial_state;
  new_service->current_event_id = 0L;
  new_service->last_event_id = 0L;
  new_service->current_problem_id = 0L;
  new_service->last_problem_id = 0L;
  new_service->last_state = initial_state;
  new_service->last_hard_state = initial_state;
  new_service->state_type = HARD_STATE;
  new_service->host_problem_at_last_check = FALSE;
  new_service->check_flapping_recovery_notification = FALSE;
  new_service->next_check = (time_t)0;
  new_service->should_be_scheduled = TRUE;
  new_service->last_check = (time_t)0;
  new_service->last_notification = (time_t)0;
  new_service->next_notification = (time_t)0;
  new_service->no_more_notifications = FALSE;
  new_service->last_state_change = (time_t)0;
  new_service->last_hard_state_change = (time_t)0;
  new_service->last_time_ok = (time_t)0;
  new_service->last_time_warning = (time_t)0;
  new_service->last_time_unknown = (time_t)0;
  new_service->last_time_critical = (time_t)0;
  new_service->has_been_checked = FALSE;
  new_service->is_being_freshened = FALSE;
  new_service->notified_on_unknown = FALSE;
  new_service->notified_on_warning = FALSE;
  new_service->notified_on_critical = FALSE;
  new_service->current_notification_number = 0;
  new_service->current_notification_id = 0L;
  new_service->latency = 0.0;
  new_service->execution_time = 0.0;
  new_service->is_executing = FALSE;
  new_service->check_options = CHECK_OPTION_NONE;
  new_service->scheduled_downtime_depth = 0;
  new_service->pending_flex_downtime = 0;
  for (x = 0; x < MAX_STATE_HISTORY_ENTRIES; x++)
    new_service->state_history[x] = STATE_OK;
  new_service->state_history_index = 0;
  new_service->is_flapping = FALSE;
  new_service->flapping_comment_id = 0;
  new_service->percent_state_change = 0.0;
  new_service->modified_attributes = MODATTR_NONE;

  /* add new service to skiplist */
  result = skiplist_insert(object_skiplists[SERVICE_SKIPLIST], (void*)new_service);
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Service '%s' on host '%s' has already been defined\n",
          description, host_name);
    result = ERROR;
    break;

  case SKIPLIST_OK:
    result = OK;
    break;

  default:
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Could not add service '%s' on host '%s' to skiplist\n",
          description, host_name);
    result = ERROR;
    break;
  }

  /* handle errors */
  if (result == ERROR) {
    delete[] new_service->perf_data;
    delete[] new_service->plugin_output;
    delete[] new_service->long_plugin_output;
    delete[] new_service->failure_prediction_options;
    delete[] new_service->notification_period;
    delete[] new_service->event_handler;
    delete[] new_service->service_check_command;
    delete[] new_service->display_name;
    delete[] new_service->description;
    delete[] new_service->host_name;
    delete new_service;
    return (NULL);
  }

  /* services are sorted alphabetically, so add new items to tail of list */
  if (service_list == NULL) {
    service_list = new_service;
    service_list_tail = service_list;
  }
  else {
    service_list_tail->next = new_service;
    service_list_tail = new_service;
  }

  return (new_service);
}

/* adds a contact group to a service */
contactgroupsmember* add_contactgroup_to_service(service* svc, char const* group_name) {
  contactgroupsmember* new_contactgroupsmember = NULL;

  /* bail out if we weren't given the data we need */
  if (svc == NULL || (group_name == NULL || !strcmp(group_name, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Service or contactgroup name is NULL\n");
    return (NULL);
  }

  /* allocate memory for the contactgroups member */
  new_contactgroupsmember = new contactgroupsmember;
  memset(new_contactgroupsmember, 0, sizeof(*new_contactgroupsmember));

  /* duplicate vars */
  new_contactgroupsmember->group_name = my_strdup(group_name);

  /* add this contactgroup to the service */
  new_contactgroupsmember->next = svc->contact_groups;
  svc->contact_groups = new_contactgroupsmember;

  return (new_contactgroupsmember);
}

/* adds a contact to a service */
contactsmember* add_contact_to_service(service* svc, char const* contact_name) {
  return (add_contact_to_object(&svc->contacts, contact_name));
}

/* adds a custom variable to a service */
customvariablesmember* add_custom_variable_to_service(service* svc, char const* varname, char const* varvalue) {
  return (add_custom_variable_to_object(&svc->custom_variables, varname, varvalue));
}

/* add a new command to the list in memory */
command* add_command(char const* name, char const* value) {
  command* new_command = NULL;
  int result = OK;

  /* make sure we have the data we need */
  if ((name == NULL || !strcmp(name, ""))
      || (value == NULL || !strcmp(value, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Command name of command line is NULL\n");
    return (NULL);
  }

  /* allocate memory for the new command */
  new_command = new command;
  memset(new_command, 0, sizeof(*new_command));

  /* duplicate vars */
  new_command->name = my_strdup(name);
  new_command->command_line = my_strdup(value);

  /* add new command to skiplist */
  result = skiplist_insert(object_skiplists[COMMAND_SKIPLIST], (void*)new_command);
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Command '%s' has already been defined\n", name);
    result = ERROR;
    break;

  case SKIPLIST_OK:
    result = OK;
    break;

  default:
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Could not add command '%s' to skiplist\n", name);
    result = ERROR;
    break;
  }

  /* handle errors */
  if (result == ERROR) {
    delete[] new_command->command_line;
    delete[] new_command->name;
    delete new_command;
    return (NULL);
  }

  /* commands are sorted alphabetically, so add new items to tail of list */
  if (command_list == NULL) {
    command_list = new_command;
    command_list_tail = command_list;
  }
  else {
    command_list_tail->next = new_command;
    command_list_tail = new_command;
  }

  return (new_command);
}

/* add a new service escalation to the list in memory */
serviceescalation* add_serviceescalation(char const* host_name,
                                         char const* description,
                                         int first_notification,
                                         int last_notification,
                                         double notification_interval,
                                         char const* escalation_period,
                                         int escalate_on_warning,
                                         int escalate_on_unknown,
                                         int escalate_on_critical,
                                         int escalate_on_recovery) {
  serviceescalation* new_serviceescalation = NULL;
  int result = OK;

  /* make sure we have the data we need */
  if ((host_name == NULL || !strcmp(host_name, ""))
      || (description == NULL || !strcmp(description, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Service escalation host name or description is NULL\n");
    return (NULL);
  }

#ifdef TEST
  printf("NEW SVC ESCALATION: %s/%s = %d/%d/%.3f\n", host_name,
         description, first_notification, last_notification,
         notification_interval);
#endif

  /* allocate memory for a new service escalation entry */
  new_serviceescalation = new serviceescalation;
  memset(new_serviceescalation, 0, sizeof(*new_serviceescalation));

  /* duplicate vars */
  new_serviceescalation->host_name = my_strdup(host_name);
  new_serviceescalation->description = my_strdup(description);
  if (escalation_period)
    new_serviceescalation->escalation_period = my_strdup(escalation_period);

  new_serviceescalation->first_notification = first_notification;
  new_serviceescalation->last_notification = last_notification;
  new_serviceescalation->notification_interval = (notification_interval <= 0) ? 0 : notification_interval;
  new_serviceescalation->escalate_on_recovery = (escalate_on_recovery > 0) ? TRUE : FALSE;
  new_serviceescalation->escalate_on_warning = (escalate_on_warning > 0) ? TRUE : FALSE;
  new_serviceescalation->escalate_on_unknown = (escalate_on_unknown > 0) ? TRUE : FALSE;
  new_serviceescalation->escalate_on_critical = (escalate_on_critical > 0) ? TRUE : FALSE;

  /* add new serviceescalation to skiplist */
  result = skiplist_insert(object_skiplists[SERVICEESCALATION_SKIPLIST], (void*)new_serviceescalation);
  switch (result) {
  case SKIPLIST_OK:
    result = OK;
    break;

  default:
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: Could not add escalation for service '%s' on host '%s' to skiplist\n",
          description, host_name);
    result = ERROR;
    break;
  }

  /* handle errors */
  if (result == ERROR) {
    delete[] new_serviceescalation->host_name;
    delete[] new_serviceescalation->description;
    delete[] new_serviceescalation->escalation_period;
    delete new_serviceescalation;
    return (NULL);
  }

  /* service escalations are sorted alphabetically, so add new items to tail of list */
  if (serviceescalation_list == NULL) {
    serviceescalation_list = new_serviceescalation;
    serviceescalation_list_tail = serviceescalation_list;
  }
  else {
    serviceescalation_list_tail->next = new_serviceescalation;
    serviceescalation_list_tail = new_serviceescalation;
  }

  return (new_serviceescalation);
}

/* adds a contact group to a service escalation */
contactgroupsmember* add_contactgroup_to_serviceescalation(serviceescalation* se, char const* group_name) {
  contactgroupsmember* new_contactgroupsmember = NULL;

  /* bail out if we weren't given the data we need */
  if (se == NULL || (group_name == NULL || !strcmp(group_name, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Service escalation or contactgroup name is NULL\n");
    return (NULL);
  }

  /* allocate memory for the contactgroups member */
  new_contactgroupsmember = new contactgroupsmember;
  memset(new_contactgroupsmember, 0, sizeof(*new_contactgroupsmember));

  /* duplicate vars */
  new_contactgroupsmember->group_name = my_strdup(group_name);

  /* add this contactgroup to the service escalation */
  new_contactgroupsmember->next = se->contact_groups;
  se->contact_groups = new_contactgroupsmember;

  return (new_contactgroupsmember);
}

/* adds a contact to a service escalation */
contactsmember* add_contact_to_serviceescalation(serviceescalation* se, char const* contact_name) {
  return (add_contact_to_object(&se->contacts, contact_name));
}

/* adds a service dependency definition */
servicedependency* add_service_dependency(char const* dependent_host_name,
                                          char const* dependent_service_description,
                                          char const* host_name,
                                          char const* service_description,
                                          int dependency_type,
                                          int inherits_parent,
                                          int fail_on_ok,
                                          int fail_on_warning,
                                          int fail_on_unknown,
                                          int fail_on_critical,
                                          int fail_on_pending,
                                          char const* dependency_period) {
  servicedependency* new_servicedependency = NULL;
  int result = OK;

  /* make sure we have what we need */
  if ((host_name == NULL || !strcmp(host_name, ""))
      || (service_description == NULL
          || !strcmp(service_description, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: NULL master service description/host name in service dependency definition\n");
    return (NULL);
  }
  if ((dependent_host_name == NULL || !strcmp(dependent_host_name, ""))
      || (dependent_service_description == NULL
          || !strcmp(dependent_service_description, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE,
          "Error: NULL dependent service description/host name in service dependency definition\n");
    return (NULL);
  }

  /* allocate memory for a new service dependency entry */
  new_servicedependency = new servicedependency;
  memset(new_servicedependency, 0, sizeof(*new_servicedependency));

  /* duplicate vars */
  new_servicedependency->dependent_host_name = my_strdup(dependent_host_name);
  new_servicedependency->dependent_service_description = my_strdup(dependent_service_description);
  new_servicedependency->host_name = my_strdup(host_name);
  new_servicedependency->service_description = my_strdup(service_description);
  if (dependency_period)
    new_servicedependency->dependency_period = my_strdup(dependency_period);

  new_servicedependency->dependency_type = (dependency_type == EXECUTION_DEPENDENCY) ? EXECUTION_DEPENDENCY : NOTIFICATION_DEPENDENCY;
  new_servicedependency->inherits_parent = (inherits_parent > 0) ? TRUE : FALSE;
  new_servicedependency->fail_on_ok = (fail_on_ok == 1) ? TRUE : FALSE;
  new_servicedependency->fail_on_warning = (fail_on_warning == 1) ? TRUE : FALSE;
  new_servicedependency->fail_on_unknown = (fail_on_unknown == 1) ? TRUE : FALSE;
  new_servicedependency->fail_on_critical = (fail_on_critical == 1) ? TRUE : FALSE;
  new_servicedependency->fail_on_pending = (fail_on_pending == 1) ? TRUE : FALSE;
  new_servicedependency->circular_path_checked = FALSE;
  new_servicedependency->contains_circular_path = FALSE;

  /* add new service dependency to skiplist */
  result = skiplist_insert(object_skiplists[SERVICEDEPENDENCY_SKIPLIST], (void*)new_servicedependency);
  switch (result) {
  case SKIPLIST_OK:
    result = OK;
    break;

  default:
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Could not add service dependency to skiplist\n");
    result = ERROR;
    break;
  }

  /* handle errors */
  if (result == ERROR) {
    delete[] new_servicedependency->host_name;
    delete[] new_servicedependency->service_description;
    delete[] new_servicedependency->dependent_host_name;
    delete[] new_servicedependency->dependent_service_description;
    delete new_servicedependency;
    return (NULL);
  }

  /* service dependencies are sorted alphabetically, so add new items to tail of list */
  if (servicedependency_list == NULL) {
    servicedependency_list = new_servicedependency;
    servicedependency_list_tail = servicedependency_list;
  }
  else {
    servicedependency_list_tail->next = new_servicedependency;
    servicedependency_list_tail = new_servicedependency;
  }

  return (new_servicedependency);
}

/* adds a host dependency definition */
hostdependency* add_host_dependency(char const* dependent_host_name,
                                    char const* host_name,
                                    int dependency_type,
                                    int inherits_parent,
				    int fail_on_up,
                                    int fail_on_down,
                                    int fail_on_unreachable,
                                    int fail_on_pending,
                                    char const* dependency_period) {
  hostdependency* new_hostdependency = NULL;
  int result = OK;

  /* make sure we have what we need */
  if ((dependent_host_name == NULL || !strcmp(dependent_host_name, ""))
      || (host_name == NULL || !strcmp(host_name, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: NULL host name in host dependency definition\n");
    return (NULL);
  }

  /* allocate memory for a new host dependency entry */
  new_hostdependency = new hostdependency;
  memset(new_hostdependency, 0, sizeof(*new_hostdependency));

  /* duplicate vars */
  new_hostdependency->dependent_host_name = my_strdup(dependent_host_name);
  new_hostdependency->host_name = my_strdup(host_name);
  if (dependency_period)
    new_hostdependency->dependency_period = my_strdup(dependency_period);

  new_hostdependency->dependency_type = (dependency_type == EXECUTION_DEPENDENCY)
    ? EXECUTION_DEPENDENCY : NOTIFICATION_DEPENDENCY;
  new_hostdependency->inherits_parent = (inherits_parent > 0) ? TRUE : FALSE;
  new_hostdependency->fail_on_up = (fail_on_up == 1) ? TRUE : FALSE;
  new_hostdependency->fail_on_down = (fail_on_down == 1) ? TRUE : FALSE;
  new_hostdependency->fail_on_unreachable = (fail_on_unreachable == 1) ? TRUE : FALSE;
  new_hostdependency->fail_on_pending = (fail_on_pending == 1) ? TRUE : FALSE;
  new_hostdependency->circular_path_checked = FALSE;
  new_hostdependency->contains_circular_path = FALSE;

  /* add new host dependency to skiplist */
  result = skiplist_insert(object_skiplists[HOSTDEPENDENCY_SKIPLIST], (void*)new_hostdependency);
  switch (result) {
  case SKIPLIST_OK:
    result = OK;
    break;

  default:
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Could not add host dependency to skiplist\n");
    result = ERROR;
    break;
  }

  /* handle errors */
  if (result == ERROR) {
    delete[] new_hostdependency->host_name;
    delete[] new_hostdependency->dependent_host_name;
    delete new_hostdependency;
    return (NULL);
  }

  /* host dependencies are sorted alphabetically, so add new items to tail of list */
  if (hostdependency_list == NULL) {
    hostdependency_list = new_hostdependency;
    hostdependency_list_tail = hostdependency_list;
  }
  else {
    hostdependency_list_tail->next = new_hostdependency;
    hostdependency_list_tail = new_hostdependency;
  }
  return (new_hostdependency);
}

/* add a new host escalation to the list in memory */
hostescalation* add_hostescalation(char const* host_name,
                                   int first_notification,
                                   int last_notification,
                                   double notification_interval,
                                   char const* escalation_period,
                                   int escalate_on_down,
                                   int escalate_on_unreachable,
                                   int escalate_on_recovery) {
  hostescalation* new_hostescalation = NULL;
  int result = OK;

  /* make sure we have the data we need */
  if (host_name == NULL || !strcmp(host_name, "")) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Host escalation host name is NULL\n");
    return (NULL);
  }

#ifdef TEST
  printf("NEW HST ESCALATION: %s = %d/%d/%.3f\n", host_name,
         first_notification, last_notification, notification_interval);
#endif

  /* allocate memory for a new host escalation entry */
  new_hostescalation = new hostescalation;
  memset(new_hostescalation, 0, sizeof(*new_hostescalation));

  /* duplicate vars */
  new_hostescalation->host_name = my_strdup(host_name);
  if (escalation_period)
    new_hostescalation->escalation_period = my_strdup(escalation_period);

  new_hostescalation->first_notification = first_notification;
  new_hostescalation->last_notification = last_notification;
  new_hostescalation->notification_interval = (notification_interval <= 0) ? 0 : notification_interval;
  new_hostescalation->escalate_on_recovery = (escalate_on_recovery > 0) ? TRUE : FALSE;
  new_hostescalation->escalate_on_down = (escalate_on_down > 0) ? TRUE : FALSE;
  new_hostescalation->escalate_on_unreachable = (escalate_on_unreachable > 0) ? TRUE : FALSE;

  /* add new hostescalation to skiplist */
  result = skiplist_insert(object_skiplists[HOSTESCALATION_SKIPLIST], (void*)new_hostescalation);
  switch (result) {
  case SKIPLIST_OK:
    result = OK;
    break;

  default:
    logit(NSLOG_CONFIG_ERROR, TRUE,
	  "Error: Could not add hostescalation '%s' to skiplist\n",
          host_name);
    result = ERROR;
    break;
  }

  /* handle errors */
  if (result == ERROR) {
    delete[] new_hostescalation->host_name;
    delete[] new_hostescalation->escalation_period;
    delete new_hostescalation;
    return (NULL);
  }

  /* host escalations are sorted alphabetically, so add new items to tail of list */
  if (hostescalation_list == NULL) {
    hostescalation_list = new_hostescalation;
    hostescalation_list_tail = hostescalation_list;
  }
  else {
    hostescalation_list_tail->next = new_hostescalation;
    hostescalation_list_tail = new_hostescalation;
  }

  return (new_hostescalation);
}

/* adds a contact group to a host escalation */
contactgroupsmember* add_contactgroup_to_hostescalation(hostescalation* he, char const* group_name) {
  contactgroupsmember* new_contactgroupsmember = NULL;

  /* bail out if we weren't given the data we need */
  if (he == NULL || (group_name == NULL || !strcmp(group_name, ""))) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Host escalation or contactgroup name is NULL\n");
    return (NULL);
  }

  /* allocate memory for the contactgroups member */
  new_contactgroupsmember = new contactgroupsmember;
  memset(new_contactgroupsmember, 0, sizeof(*new_contactgroupsmember));

  /* duplicate vars */
  new_contactgroupsmember->group_name = my_strdup(group_name);

  /* add this contactgroup to the host escalation */
  new_contactgroupsmember->next = he->contact_groups;
  he->contact_groups = new_contactgroupsmember;

  return (new_contactgroupsmember);
}

/* adds a contact to a host escalation */
contactsmember* add_contact_to_hostescalation(hostescalation* he, char const* contact_name) {
  return (add_contact_to_object(&he->contacts, contact_name));
}

/* adds a contact to an object */
contactsmember* add_contact_to_object(contactsmember** object_ptr, char const* contactname) {
  contactsmember* new_contactsmember = NULL;

  /* make sure we have the data we need */
  if (object_ptr == NULL) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Contact object is NULL\n");
    return (NULL);
  }

  if (contactname == NULL || !strcmp(contactname, "")) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Contact name is NULL\n");
    return (NULL);
  }

  /* allocate memory for a new member */
  new_contactsmember = new contactsmember;
  new_contactsmember->contact_name = my_strdup(contactname);

  /* set initial values */
  new_contactsmember->contact_ptr = NULL;

  /* add the new contact to the head of the contact list */
  new_contactsmember->next = *object_ptr;
  *object_ptr = new_contactsmember;

  return (new_contactsmember);
}

/* adds a custom variable to an object */
customvariablesmember* add_custom_variable_to_object(customvariablesmember** object_ptr,
						     char const* varname,
						     char const* varvalue) {
  customvariablesmember* new_customvariablesmember = NULL;

  /* make sure we have the data we need */
  if (object_ptr == NULL) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Custom variable object is NULL\n");
    return (NULL);
  }

  if (varname == NULL || !strcmp(varname, "")) {
    logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Custom variable name is NULL\n");
    return (NULL);
  }

  /* allocate memory for a new member */
  new_customvariablesmember = new customvariablesmember;
  new_customvariablesmember->variable_name = my_strdup(varname);

  if (varvalue)
    new_customvariablesmember->variable_value = my_strdup(varvalue);
  else
    new_customvariablesmember->variable_value = NULL;

  /* set initial values */
  new_customvariablesmember->has_been_modified = FALSE;

  /* add the new member to the head of the member list */
  new_customvariablesmember->next = *object_ptr;
  *object_ptr = new_customvariablesmember;

  return (new_customvariablesmember);
}

/******************************************************************/
/******************** OBJECT SEARCH FUNCTIONS *********************/
/******************************************************************/

/* given a timeperiod name and a starting point, find a timeperiod from the list in memory */
timeperiod* find_timeperiod(char const* name) {
  timeperiod temp_timeperiod;

  if (name == NULL)
    return (NULL);

  temp_timeperiod.name = const_cast<char*>(name);
  return ((timeperiod*)skiplist_find_first(object_skiplists[TIMEPERIOD_SKIPLIST],
					   &temp_timeperiod,
					   NULL));
}

/* given a host name, find it in the list in memory */
host* find_host(char const* name) {
  host temp_host;

  if (name == NULL)
    return (NULL);

  temp_host.name = const_cast<char*>(name);
  return ((host*)skiplist_find_first(object_skiplists[HOST_SKIPLIST],
				     &temp_host,
				     NULL));
}

/* find a hostgroup from the list in memory */
hostgroup* find_hostgroup(char const* name) {
  hostgroup temp_hostgroup;

  if (name == NULL)
    return (NULL);

  temp_hostgroup.group_name = const_cast<char*>(name);
  return ((hostgroup*)skiplist_find_first(object_skiplists[HOSTGROUP_SKIPLIST],
					  &temp_hostgroup,
					  NULL));
}

/* find a servicegroup from the list in memory */
servicegroup* find_servicegroup(char const* name) {
  servicegroup temp_servicegroup;

  if (name == NULL)
    return (NULL);

  temp_servicegroup.group_name = const_cast<char*>(name);
  return ((servicegroup*)skiplist_find_first(object_skiplists[SERVICEGROUP_SKIPLIST],
					     &temp_servicegroup,
					     NULL));
}

/* find a contact from the list in memory */
contact* find_contact(char const* name) {
  contact temp_contact;

  if (name == NULL)
    return (NULL);

  temp_contact.name = const_cast<char*>(name);
  return ((contact*)skiplist_find_first(object_skiplists[CONTACT_SKIPLIST],
					&temp_contact,
					NULL));
}

/* find a contact group from the list in memory */
contactgroup* find_contactgroup(char const* name) {
  contactgroup temp_contactgroup;

  if (name == NULL)
    return (NULL);

  temp_contactgroup.group_name = const_cast<char*>(name);
  return ((contactgroup*)skiplist_find_first(object_skiplists[CONTACTGROUP_SKIPLIST],
					     &temp_contactgroup,
					     NULL));
}

/* given a command name, find a command from the list in memory */
command* find_command(char const* name) {
  command temp_command;

  if (name == NULL)
    return (NULL);

  temp_command.name = const_cast<char*>(name);
  return ((command*)skiplist_find_first(object_skiplists[COMMAND_SKIPLIST],
					&temp_command,
					NULL));
}

/* given a host/service name, find the service in the list in memory */
service* find_service(char const* host_name, char const* svc_desc) {
  service temp_service;

  if (host_name == NULL || svc_desc == NULL)
    return (NULL);

  temp_service.host_name = const_cast<char*>(host_name);
  temp_service.description = const_cast<char*>(svc_desc);
  return ((service*)skiplist_find_first(object_skiplists[SERVICE_SKIPLIST],
					&temp_service,
					NULL));
}

/******************************************************************/
/******************* OBJECT TRAVERSAL FUNCTIONS *******************/
/******************************************************************/

hostescalation* get_first_hostescalation_by_host(char const* host_name, void** ptr) {
  hostescalation temp_hostescalation;

  if (host_name == NULL)
    return (NULL);

  temp_hostescalation.host_name = const_cast<char*>(host_name);
  return ((hostescalation*)skiplist_find_first(object_skiplists[HOSTESCALATION_SKIPLIST],
					       &temp_hostescalation,
					       ptr));
}

hostescalation* get_next_hostescalation_by_host(char const* host_name, void** ptr) {
  hostescalation temp_hostescalation;

  if (host_name == NULL)
    return (NULL);

  temp_hostescalation.host_name = const_cast<char*>(host_name);
  return ((hostescalation*)skiplist_find_next(object_skiplists[HOSTESCALATION_SKIPLIST],
					      &temp_hostescalation,
					      ptr));
}

serviceescalation* get_first_serviceescalation_by_service(char const* host_name,
                                                          char const* svc_description,
                                                          void** ptr) {
  serviceescalation temp_serviceescalation;

  if (host_name == NULL || svc_description == NULL)
    return (NULL);

  temp_serviceescalation.host_name = const_cast<char*>(host_name);
  temp_serviceescalation.description = const_cast<char*>(svc_description);
  return ((serviceescalation*)skiplist_find_first(object_skiplists[SERVICEESCALATION_SKIPLIST],
						  &temp_serviceescalation,
						  ptr));
}

serviceescalation* get_next_serviceescalation_by_service(char const* host_name,
                                                         char const* svc_description,
                                                         void** ptr) {
  serviceescalation temp_serviceescalation;

  if (host_name == NULL || svc_description == NULL)
    return (NULL);

  temp_serviceescalation.host_name = const_cast<char*>(host_name);
  temp_serviceescalation.description = const_cast<char*>(svc_description);
  return ((serviceescalation*)skiplist_find_next(object_skiplists[SERVICEESCALATION_SKIPLIST],
						 &temp_serviceescalation,
						 ptr));
}

hostdependency* get_first_hostdependency_by_dependent_host(char const* host_name, void** ptr) {
  hostdependency temp_hostdependency;

  if (host_name == NULL)
    return (NULL);

  temp_hostdependency.dependent_host_name = const_cast<char*>(host_name);
  return ((hostdependency*)skiplist_find_first(object_skiplists[HOSTDEPENDENCY_SKIPLIST],
					       &temp_hostdependency,
					       ptr));
}

hostdependency* get_next_hostdependency_by_dependent_host(char const* host_name, void** ptr) {
  hostdependency temp_hostdependency;

  if (host_name == NULL || ptr == NULL)
    return (NULL);

  temp_hostdependency.dependent_host_name = const_cast<char*>(host_name);
  return ((hostdependency*)skiplist_find_next(object_skiplists[HOSTDEPENDENCY_SKIPLIST],
					      &temp_hostdependency,
					      ptr));
}

servicedependency* get_first_servicedependency_by_dependent_service(char const* host_name,
                                                                    char const* svc_description,
                                                                    void** ptr) {
  servicedependency temp_servicedependency;

  if (host_name == NULL || svc_description == NULL)
    return (NULL);

  temp_servicedependency.dependent_host_name = const_cast<char*>(host_name);
  temp_servicedependency.dependent_service_description = const_cast<char*>(svc_description);
  return ((servicedependency*)skiplist_find_first(object_skiplists[SERVICEDEPENDENCY_SKIPLIST],
						  &temp_servicedependency,
						  ptr));
}

servicedependency* get_next_servicedependency_by_dependent_service(char const* host_name,
                                                                   char const* svc_description,
                                                                   void** ptr) {
  servicedependency temp_servicedependency;

  if (host_name == NULL || svc_description == NULL || ptr == NULL)
    return (NULL);

  temp_servicedependency.dependent_host_name = const_cast<char*>(host_name);
  temp_servicedependency.dependent_service_description = const_cast<char*>(svc_description);
  return ((servicedependency*)skiplist_find_next(object_skiplists[SERVICEDEPENDENCY_SKIPLIST],
						 &temp_servicedependency,
						 ptr));
}

/* adds a object to a list of objects */
int add_object_to_objectlist(objectlist** list, void* object_ptr) {
  objectlist* temp_item = NULL;
  objectlist* new_item = NULL;

  if (list == NULL || object_ptr == NULL)
    return (ERROR);

  /* skip this object if its already in the list */
  for (temp_item = *list; temp_item; temp_item = temp_item->next) {
    if (temp_item->object_ptr == object_ptr)
      break;
  }
  if (temp_item)
    return (OK);

  /* allocate memory for a new list item */
  new_item = new objectlist;

  /* initialize vars */
  new_item->object_ptr = object_ptr;

  /* add new item to head of list */
  new_item->next = *list;
  *list = new_item;

  return (OK);
}

/* remove a object to a list of objects */
int remove_object_to_objectlist(objectlist** list, void* object_ptr) {
  if (list == NULL)
    return (ERROR);

  for (objectlist* obj = *list, *prev = NULL; obj != NULL; prev = obj, obj = obj->next) {
    if (obj == object_ptr) {
      if (prev == NULL)
	*list = obj->next;
      else
	prev->next = obj->next;
      delete obj;
      return (OK);
    }
  }
  return (ERROR);
}

/* frees memory allocated to a temporary object list */
int free_objectlist(objectlist** temp_list) {
  objectlist* this_objectlist = NULL;
  objectlist* next_objectlist = NULL;

  if (temp_list == NULL)
    return (ERROR);

  /* free memory allocated to object list */
  for (this_objectlist = *temp_list;
       this_objectlist != NULL;
       this_objectlist = next_objectlist) {
    next_objectlist = this_objectlist->next;
    delete this_objectlist;
  }

  *temp_list = NULL;
  return (OK);
}

/******************************************************************/
/********************* OBJECT QUERY FUNCTIONS *********************/
/******************************************************************/

/* determines whether or not a specific host is an immediate child of another host */
int is_host_immediate_child_of_host(host* parent_host, host* child_host) {
  hostsmember* temp_hostsmember = NULL;

  /* not enough data */
  if (child_host == NULL)
    return (FALSE);

  /* root/top-level hosts */
  if (parent_host == NULL) {
    if (child_host->parent_hosts == NULL)
      return (TRUE);
  }
  /* mid-level/bottom hosts */
  else {
    for (temp_hostsmember = child_host->parent_hosts;
         temp_hostsmember != NULL;
         temp_hostsmember = temp_hostsmember->next) {
      if (temp_hostsmember->host_ptr == parent_host)
        return (TRUE);
    }
  }

  return (FALSE);
}

/* determines whether or not a specific host is an immediate parent of another host */
int is_host_immediate_parent_of_host(host* child_host, host* parent_host) {
  if (is_host_immediate_child_of_host(parent_host, child_host) == TRUE)
    return (TRUE);
  return (FALSE);
}

/* returns a count of the immediate children for a given host */
/* NOTE: This function is only used by the CGIS */
int number_of_immediate_child_hosts(host* hst) {
  int children = 0;
  host* temp_host = NULL;

  for (temp_host = host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {
    if (is_host_immediate_child_of_host(hst, temp_host) == TRUE)
      children++;
  }
  return (children);
}

/* returns a count of the total children for a given host */
/* NOTE: This function is only used by the CGIS */
int number_of_total_child_hosts(host* hst) {
  int children = 0;
  host* temp_host = NULL;

  for (temp_host = host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {
    if (is_host_immediate_child_of_host(hst, temp_host) == TRUE)
      children += number_of_total_child_hosts(temp_host) + 1;
  }
  return (children);
}

/* get the number of immediate parent hosts for a given host */
/* NOTE: This function is only used by the CGIS */
int number_of_immediate_parent_hosts(host* hst) {
  int parents = 0;
  host* temp_host = NULL;

  for (temp_host = host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {
    if (is_host_immediate_parent_of_host(hst, temp_host) == TRUE)
      parents++;
  }
  return (parents);
}

/* get the total number of parent hosts for a given host */
/* NOTE: This function is only used by the CGIS */
int number_of_total_parent_hosts(host* hst) {
  int parents = 0;
  host* temp_host = NULL;

  for (temp_host = host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {
    if (is_host_immediate_parent_of_host(hst, temp_host) == TRUE)
      parents += number_of_total_parent_hosts(temp_host) + 1;
  }
  return (parents);
}

/*  tests whether a host is a member of a particular hostgroup */
/* NOTE: This function is only used by the CGIS */
int is_host_member_of_hostgroup(hostgroup* group, host* hst) {
  hostsmember* temp_hostsmember = NULL;

  if (group == NULL || hst == NULL)
    return (FALSE);

  for (temp_hostsmember = group->members;
       temp_hostsmember != NULL;
       temp_hostsmember = temp_hostsmember->next) {
    if (temp_hostsmember->host_ptr == hst)
      return (TRUE);
  }
  return (FALSE);
}

/*  tests whether a host is a member of a particular servicegroup */
/* NOTE: This function is only used by the CGIS */
int is_host_member_of_servicegroup(servicegroup* group, host* hst) {
  servicesmember* temp_servicesmember = NULL;

  if (group == NULL || hst == NULL)
    return (FALSE);

  for (temp_servicesmember = group->members;
       temp_servicesmember != NULL;
       temp_servicesmember = temp_servicesmember->next) {
    if (temp_servicesmember->service_ptr != NULL
        && temp_servicesmember->service_ptr->host_ptr == hst)
      return (TRUE);
  }
  return (FALSE);
}

/*  tests whether a service is a member of a particular servicegroup */
/* NOTE: This function is only used by the CGIS */
int is_service_member_of_servicegroup(servicegroup* group, service* svc) {
  servicesmember* temp_servicesmember = NULL;

  if (group == NULL || svc == NULL)
    return (FALSE);

  for (temp_servicesmember = group->members;
       temp_servicesmember != NULL;
       temp_servicesmember = temp_servicesmember->next) {
    if (temp_servicesmember->service_ptr == svc)
      return (TRUE);
  }
  return (FALSE);
}

/*
 * tests whether a contact is a member of a particular contactgroup.
 * The mk-livestatus eventbroker module uses this, so it must hang
 * around until 4.0 to prevent api breakage.
 * The cgi's stopped using it quite long ago though, so we need only
 * compile it if we're building the core
 */
int is_contact_member_of_contactgroup(contactgroup* group, contact* cntct) {
  contactsmember* member;
  contact* temp_contact = NULL;

  if (!group || !cntct)
    return (FALSE);

  /* search all contacts in this contact group */
  for (member = group->members; member; member = member->next) {
    temp_contact = member->contact_ptr;
    if (temp_contact == NULL)
      continue;
    if (temp_contact == cntct)
      return (TRUE);
  }
  return (FALSE);
}

/*  tests whether a contact is a contact for a particular host */
int is_contact_for_host(host* hst, contact* cntct) {
  contactsmember* temp_contactsmember = NULL;
  contact* temp_contact = NULL;
  contactgroupsmember* temp_contactgroupsmember = NULL;
  contactgroup* temp_contactgroup = NULL;

  if (hst == NULL || cntct == NULL)
    return (FALSE);

  /* search all individual contacts of this host */
  for (temp_contactsmember = hst->contacts;
       temp_contactsmember != NULL;
       temp_contactsmember = temp_contactsmember->next) {
    temp_contact = temp_contactsmember->contact_ptr;
    if (temp_contact == NULL)
      continue;
    if (temp_contact == cntct)
      return (TRUE);
  }

  /* search all contactgroups of this host */
  for (temp_contactgroupsmember = hst->contact_groups;
       temp_contactgroupsmember != NULL;
       temp_contactgroupsmember = temp_contactgroupsmember->next) {
    temp_contactgroup = temp_contactgroupsmember->group_ptr;
    if (temp_contactgroup == NULL)
      continue;
    if (is_contact_member_of_contactgroup(temp_contactgroup, cntct))
      return (TRUE);
  }

  return (FALSE);
}

/* tests whether or not a contact is an escalated contact for a particular host */
int is_escalated_contact_for_host(host* hst, contact* cntct) {
  contactsmember* temp_contactsmember = NULL;
  contact* temp_contact = NULL;
  hostescalation* temp_hostescalation = NULL;
  contactgroupsmember* temp_contactgroupsmember = NULL;
  contactgroup* temp_contactgroup = NULL;
  void* ptr = NULL;

  /* search all host escalations */
  for (temp_hostescalation = get_first_hostescalation_by_host(hst->name, &ptr);
       temp_hostescalation != NULL;
       temp_hostescalation = get_next_hostescalation_by_host(hst->name, &ptr)) {

    /* search all contacts of this host escalation */
    for (temp_contactsmember = temp_hostescalation->contacts;
         temp_contactsmember != NULL;
         temp_contactsmember = temp_contactsmember->next) {
      temp_contact = temp_contactsmember->contact_ptr;
      if (temp_contact == NULL)
        continue;
      if (temp_contact == cntct)
        return (TRUE);
    }

    /* search all contactgroups of this host escalation */
    for (temp_contactgroupsmember = temp_hostescalation->contact_groups;
         temp_contactgroupsmember != NULL;
         temp_contactgroupsmember = temp_contactgroupsmember->next) {
      temp_contactgroup = temp_contactgroupsmember->group_ptr;
      if (temp_contactgroup == NULL)
        continue;
      if (is_contact_member_of_contactgroup(temp_contactgroup, cntct))
        return (TRUE);

    }
  }

  return (FALSE);
}

/*  tests whether a contact is a contact for a particular service */
int is_contact_for_service(service* svc, contact* cntct) {
  contactsmember* temp_contactsmember = NULL;
  contact* temp_contact = NULL;
  contactgroupsmember* temp_contactgroupsmember = NULL;
  contactgroup* temp_contactgroup = NULL;

  if (svc == NULL || cntct == NULL)
    return (FALSE);

  /* search all individual contacts of this service */
  for (temp_contactsmember = svc->contacts;
       temp_contactsmember != NULL;
       temp_contactsmember = temp_contactsmember->next) {
    temp_contact = temp_contactsmember->contact_ptr;

    if (temp_contact == cntct)
      return (TRUE);
  }

  /* search all contactgroups of this service */
  for (temp_contactgroupsmember = svc->contact_groups;
       temp_contactgroupsmember != NULL;
       temp_contactgroupsmember = temp_contactgroupsmember->next) {
    temp_contactgroup = temp_contactgroupsmember->group_ptr;
    if (temp_contactgroup == NULL)
      continue;
    if (is_contact_member_of_contactgroup(temp_contactgroup, cntct))
      return (TRUE);

  }

  return (FALSE);
}

/* tests whether or not a contact is an escalated contact for a particular service */
int is_escalated_contact_for_service(service* svc, contact* cntct) {
  serviceescalation* temp_serviceescalation = NULL;
  contactsmember* temp_contactsmember = NULL;
  contact* temp_contact = NULL;
  contactgroupsmember* temp_contactgroupsmember = NULL;
  contactgroup* temp_contactgroup = NULL;
  void* ptr = NULL;

  /* search all the service escalations */
  for (temp_serviceescalation = get_first_serviceescalation_by_service(svc->host_name, svc->description, &ptr);
       temp_serviceescalation != NULL;
       temp_serviceescalation = get_next_serviceescalation_by_service(svc->host_name, svc->description, &ptr)) {

    /* search all contacts of this service escalation */
    for (temp_contactsmember = temp_serviceescalation->contacts;
         temp_contactsmember != NULL;
         temp_contactsmember = temp_contactsmember->next) {
      temp_contact = temp_contactsmember->contact_ptr;
      if (temp_contact == NULL)
        continue;
      if (temp_contact == cntct)
        return (TRUE);
    }

    /* search all contactgroups of this service escalation */
    for (temp_contactgroupsmember = temp_serviceescalation->contact_groups;
         temp_contactgroupsmember != NULL;
         temp_contactgroupsmember = temp_contactgroupsmember->next) {
      temp_contactgroup = temp_contactgroupsmember->group_ptr;
      if (temp_contactgroup == NULL)
        continue;
      if (is_contact_member_of_contactgroup(temp_contactgroup, cntct))
        return (TRUE);
    }
  }

  return (FALSE);
}

/* checks to see if there exists a circular dependency for a service */
int check_for_circular_servicedependency_path(servicedependency* root_dep,
                                              servicedependency* dep,
                                              int dependency_type) {
  servicedependency* temp_sd = NULL;

  if (root_dep == NULL || dep == NULL)
    return (FALSE);

  /* this is not the proper dependency type */
  if (root_dep->dependency_type != dependency_type
      || dep->dependency_type != dependency_type)
    return (FALSE);

  /* don't go into a loop, don't bother checking anymore if we know this dependency already has a loop */
  if (root_dep->contains_circular_path == TRUE)
    return (TRUE);

  /* dependency has already been checked - there is a path somewhere, but it may not be for this particular dep... */
  /* this should speed up detection for some loops */
  if (dep->circular_path_checked == TRUE)
    return (FALSE);

  /* set the check flag so we don't get into an infinite loop */
  dep->circular_path_checked = TRUE;

  /* is this service dependent on the root service? */
  if (dep != root_dep) {
    if (root_dep->dependent_service_ptr == dep->master_service_ptr) {
      root_dep->contains_circular_path = TRUE;
      dep->contains_circular_path = TRUE;
      return (TRUE);
    }
  }

  /* notification dependencies are ok at this point as long as they don't inherit */
  if (dependency_type == NOTIFICATION_DEPENDENCY
      && dep->inherits_parent == FALSE)
    return (FALSE);

  /* check all parent dependencies */
  for (temp_sd = servicedependency_list;
       temp_sd != NULL;
       temp_sd = temp_sd->next) {

    /* only check parent dependencies */
    if (dep->master_service_ptr != temp_sd->dependent_service_ptr)
      continue;

    if (check_for_circular_servicedependency_path
        (root_dep, temp_sd, dependency_type) == TRUE)
      return (TRUE);
  }

  return (FALSE);
}

/* checks to see if there exists a circular dependency for a host */
int check_for_circular_hostdependency_path(hostdependency* root_dep,
                                           hostdependency* dep,
                                           int dependency_type) {
  hostdependency* temp_hd = NULL;

  if (root_dep == NULL || dep == NULL)
    return (FALSE);

  /* this is not the proper dependency type */
  if (root_dep->dependency_type != dependency_type
      || dep->dependency_type != dependency_type)
    return (FALSE);

  /* don't go into a loop, don't bother checking anymore if we know this dependency already has a loop */
  if (root_dep->contains_circular_path == TRUE)
    return (TRUE);

  /* dependency has already been checked - there is a path somewhere, but it may not be for this particular dep... */
  /* this should speed up detection for some loops */
  if (dep->circular_path_checked == TRUE)
    return (FALSE);

  /* set the check flag so we don't get into an infinite loop */
  dep->circular_path_checked = TRUE;

  /* is this host dependent on the root host? */
  if (dep != root_dep) {
    if (root_dep->dependent_host_ptr == dep->master_host_ptr) {
      root_dep->contains_circular_path = TRUE;
      dep->contains_circular_path = TRUE;
      return (TRUE);
    }
  }

  /* notification dependencies are ok at this point as long as they don't inherit */
  if (dependency_type == NOTIFICATION_DEPENDENCY
      && dep->inherits_parent == FALSE)
    return (FALSE);

  /* check all parent dependencies */
  for (temp_hd = hostdependency_list;
       temp_hd != NULL;
       temp_hd = temp_hd->next) {

    /* only check parent dependencies */
    if (dep->master_host_ptr != temp_hd->dependent_host_ptr)
      continue;

    if (check_for_circular_hostdependency_path
        (root_dep, temp_hd, dependency_type) == TRUE)
      return (TRUE);
  }

  return (FALSE);
}

/******************************************************************/
/******************* OBJECT DELETION FUNCTIONS ********************/
/******************************************************************/

/* free all allocated memory for objects */
int free_object_data(void) {
  timeperiod* this_timeperiod = NULL;
  timeperiod* next_timeperiod = NULL;
  daterange* this_daterange = NULL;
  daterange* next_daterange = NULL;
  timerange* this_timerange = NULL;
  timerange* next_timerange = NULL;
  timeperiodexclusion* this_timeperiodexclusion = NULL;
  timeperiodexclusion* next_timeperiodexclusion = NULL;
  host* this_host = NULL;
  host* next_host = NULL;
  hostsmember* this_hostsmember = NULL;
  hostsmember* next_hostsmember = NULL;
  hostgroup* this_hostgroup = NULL;
  hostgroup* next_hostgroup = NULL;
  servicegroup* this_servicegroup = NULL;
  servicegroup* next_servicegroup = NULL;
  servicesmember* this_servicesmember = NULL;
  servicesmember* next_servicesmember = NULL;
  contact* this_contact = NULL;
  contact* next_contact = NULL;
  contactgroup* this_contactgroup = NULL;
  contactgroup* next_contactgroup = NULL;
  contactsmember* this_contactsmember = NULL;
  contactsmember* next_contactsmember = NULL;
  contactgroupsmember* this_contactgroupsmember = NULL;
  contactgroupsmember* next_contactgroupsmember = NULL;
  customvariablesmember* this_customvariablesmember = NULL;
  customvariablesmember* next_customvariablesmember = NULL;
  service* this_service = NULL;
  service* next_service = NULL;
  command* this_command = NULL;
  command* next_command = NULL;
  commandsmember* this_commandsmember = NULL;
  commandsmember* next_commandsmember = NULL;
  serviceescalation* this_serviceescalation = NULL;
  serviceescalation* next_serviceescalation = NULL;
  servicedependency* this_servicedependency = NULL;
  servicedependency* next_servicedependency = NULL;
  hostdependency* this_hostdependency = NULL;
  hostdependency* next_hostdependency = NULL;
  hostescalation* this_hostescalation = NULL;
  hostescalation* next_hostescalation = NULL;
  int x = 0;
  unsigned int i = 0;

  /**** free memory for the timeperiod list ****/
  this_timeperiod = timeperiod_list;
  while (this_timeperiod != NULL) {
    /* free the exception time ranges contained in this timeperiod */
    for (x = 0; x < DATERANGE_TYPES; x++) {
      for (this_daterange = this_timeperiod->exceptions[x];
           this_daterange != NULL;
	   this_daterange = next_daterange) {
        next_daterange = this_daterange->next;
        for (this_timerange = this_daterange->times;
             this_timerange != NULL;
	     this_timerange = next_timerange) {
          next_timerange = this_timerange->next;
          delete this_timerange;
        }
        delete this_daterange;
      }
    }

    /* free the day time ranges contained in this timeperiod */
    for (x = 0; x < 7; x++) {
      for (this_timerange = this_timeperiod->days[x];
           this_timerange != NULL;
	   this_timerange = next_timerange) {
        next_timerange = this_timerange->next;
        delete this_timerange;
      }
    }

    /* free exclusions */
    for (this_timeperiodexclusion = this_timeperiod->exclusions;
         this_timeperiodexclusion != NULL;
         this_timeperiodexclusion = next_timeperiodexclusion) {
      next_timeperiodexclusion = this_timeperiodexclusion->next;
      delete[] this_timeperiodexclusion->timeperiod_name;
      delete this_timeperiodexclusion;
    }

    next_timeperiod = this_timeperiod->next;
    delete[] this_timeperiod->name;
    delete[] this_timeperiod->alias;
    delete this_timeperiod;
    this_timeperiod = next_timeperiod;
  }

  /* reset pointers */
  timeperiod_list = NULL;

  /**** free memory for the host list ****/
  this_host = host_list;
  while (this_host != NULL) {
    next_host = this_host->next;

    /* free memory for parent hosts */
    this_hostsmember = this_host->parent_hosts;
    while (this_hostsmember != NULL) {
      next_hostsmember = this_hostsmember->next;
      delete[] this_hostsmember->host_name;
      delete this_hostsmember;
      this_hostsmember = next_hostsmember;
    }

    /* free memory for child host links */
    this_hostsmember = this_host->child_hosts;
    while (this_hostsmember != NULL) {
      next_hostsmember = this_hostsmember->next;
      delete[] this_hostsmember->host_name;
      delete this_hostsmember;
      this_hostsmember = next_hostsmember;
    }

    /* free memory for service links */
    this_servicesmember = this_host->services;
    while (this_servicesmember != NULL) {
      next_servicesmember = this_servicesmember->next;
      delete[] this_servicesmember->host_name;
      delete[] this_servicesmember->service_description;
      delete this_servicesmember;
      this_servicesmember = next_servicesmember;
    }

    /* free memory for contact groups */
    this_contactgroupsmember = this_host->contact_groups;
    while (this_contactgroupsmember != NULL) {
      next_contactgroupsmember = this_contactgroupsmember->next;
      delete[] this_contactgroupsmember->group_name;
      delete this_contactgroupsmember;
      this_contactgroupsmember = next_contactgroupsmember;
    }

    /* free memory for contacts */
    this_contactsmember = this_host->contacts;
    while (this_contactsmember != NULL) {
      next_contactsmember = this_contactsmember->next;
      delete[] this_contactsmember->contact_name;
      delete this_contactsmember;
      this_contactsmember = next_contactsmember;
    }

    /* free memory for custom variables */
    this_customvariablesmember = this_host->custom_variables;
    while (this_customvariablesmember != NULL) {
      next_customvariablesmember = this_customvariablesmember->next;
      delete[] this_customvariablesmember->variable_name;
      delete[] this_customvariablesmember->variable_value;
      delete this_customvariablesmember;
      this_customvariablesmember = next_customvariablesmember;
    }

    delete[] this_host->name;
    delete[] this_host->display_name;
    delete[] this_host->alias;
    delete[] this_host->address;
    delete[] this_host->plugin_output;
    delete[] this_host->long_plugin_output;
    delete[] this_host->perf_data;

    free_objectlist(&this_host->hostgroups_ptr);
    delete[] this_host->check_period;
    delete[] this_host->host_check_command;
    delete[] this_host->event_handler;
    delete[] this_host->failure_prediction_options;
    delete[] this_host->notification_period;
    delete[] this_host->notes;
    delete[] this_host->notes_url;
    delete[] this_host->action_url;
    delete[] this_host->icon_image;
    delete[] this_host->icon_image_alt;
    delete[] this_host->vrml_image;
    delete[] this_host->statusmap_image;
    delete this_host;
    this_host = next_host;
  }

  /* reset pointers */
  host_list = NULL;

  /**** free memory for the host group list ****/
  this_hostgroup = hostgroup_list;
  while (this_hostgroup != NULL) {
    /* free memory for the group members */
    this_hostsmember = this_hostgroup->members;
    while (this_hostsmember != NULL) {
      next_hostsmember = this_hostsmember->next;
      delete[] this_hostsmember->host_name;
      delete this_hostsmember;
      this_hostsmember = next_hostsmember;
    }

    next_hostgroup = this_hostgroup->next;
    delete[] this_hostgroup->group_name;
    delete[] this_hostgroup->alias;
    delete[] this_hostgroup->notes;
    delete[] this_hostgroup->notes_url;
    delete[] this_hostgroup->action_url;
    delete this_hostgroup;
    this_hostgroup = next_hostgroup;
  }

  /* reset pointers */
  hostgroup_list = NULL;

  /**** free memory for the service group list ****/
  this_servicegroup = servicegroup_list;
  while (this_servicegroup != NULL) {
    /* free memory for the group members */
    this_servicesmember = this_servicegroup->members;
    while (this_servicesmember != NULL) {
      next_servicesmember = this_servicesmember->next;
      delete[] this_servicesmember->host_name;
      delete[] this_servicesmember->service_description;
      delete this_servicesmember;
      this_servicesmember = next_servicesmember;
    }

    next_servicegroup = this_servicegroup->next;
    delete[] this_servicegroup->group_name;
    delete[] this_servicegroup->alias;
    delete[] this_servicegroup->notes;
    delete[] this_servicegroup->notes_url;
    delete[] this_servicegroup->action_url;
    delete this_servicegroup;
    this_servicegroup = next_servicegroup;
  }

  /* reset pointers */
  servicegroup_list = NULL;

  /**** free memory for the contact list ****/
  this_contact = contact_list;
  while (this_contact != NULL) {
    /* free memory for the host notification commands */
    this_commandsmember = this_contact->host_notification_commands;
    while (this_commandsmember != NULL) {
      next_commandsmember = this_commandsmember->next;
      if (this_commandsmember->cmd != NULL)
        delete[] this_commandsmember->cmd;
      delete this_commandsmember;
      this_commandsmember = next_commandsmember;
    }

    /* free memory for the service notification commands */
    this_commandsmember = this_contact->service_notification_commands;
    while (this_commandsmember != NULL) {
      next_commandsmember = this_commandsmember->next;
      if (this_commandsmember->cmd != NULL)
        delete[] this_commandsmember->cmd;
      delete this_commandsmember;
      this_commandsmember = next_commandsmember;
    }

    /* free memory for custom variables */
    this_customvariablesmember = this_contact->custom_variables;
    while (this_customvariablesmember != NULL) {
      next_customvariablesmember = this_customvariablesmember->next;
      delete[] this_customvariablesmember->variable_name;
      delete[] this_customvariablesmember->variable_value;
      delete this_customvariablesmember;
      this_customvariablesmember = next_customvariablesmember;
    }

    next_contact = this_contact->next;
    delete[] this_contact->name;
    delete[] this_contact->alias;
    delete[] this_contact->email;
    delete[] this_contact->pager;
    for (i = 0; i < MAX_CONTACT_ADDRESSES; i++)
      delete[] this_contact->address[i];
    delete[] this_contact->host_notification_period;
    delete[] this_contact->service_notification_period;

    free_objectlist(&this_contact->contactgroups_ptr);

    delete this_contact;
    this_contact = next_contact;
  }

  /* reset pointers */
  contact_list = NULL;

  /**** free memory for the contact group list ****/
  this_contactgroup = contactgroup_list;
  while (this_contactgroup != NULL) {
    /* free memory for the group members */
    this_contactsmember = this_contactgroup->members;
    while (this_contactsmember != NULL) {
      next_contactsmember = this_contactsmember->next;
      delete[] this_contactsmember->contact_name;
      delete this_contactsmember;
      this_contactsmember = next_contactsmember;
    }

    next_contactgroup = this_contactgroup->next;
    delete[] this_contactgroup->group_name;
    delete[] this_contactgroup->alias;
    delete this_contactgroup;
    this_contactgroup = next_contactgroup;
  }

  /* reset pointers */
  contactgroup_list = NULL;

  /**** free memory for the service list ****/
  this_service = service_list;
  while (this_service != NULL) {
    next_service = this_service->next;

    /* free memory for contact groups */
    this_contactgroupsmember = this_service->contact_groups;
    while (this_contactgroupsmember != NULL) {
      next_contactgroupsmember = this_contactgroupsmember->next;
      delete[] this_contactgroupsmember->group_name;
      delete this_contactgroupsmember;
      this_contactgroupsmember = next_contactgroupsmember;
    }

    /* free memory for contacts */
    this_contactsmember = this_service->contacts;
    while (this_contactsmember != NULL) {
      next_contactsmember = this_contactsmember->next;
      delete[] this_contactsmember->contact_name;
      delete this_contactsmember;
      this_contactsmember = next_contactsmember;
    }

    /* free memory for custom variables */
    this_customvariablesmember = this_service->custom_variables;
    while (this_customvariablesmember != NULL) {
      next_customvariablesmember = this_customvariablesmember->next;
      delete[] this_customvariablesmember->variable_name;
      delete[] this_customvariablesmember->variable_value;
      delete this_customvariablesmember;
      this_customvariablesmember = next_customvariablesmember;
    }

    delete[] this_service->host_name;
    delete[] this_service->description;
    delete[] this_service->display_name;
    delete[] this_service->service_check_command;
    delete[] this_service->plugin_output;
    delete[] this_service->long_plugin_output;
    delete[] this_service->perf_data;

    delete[] this_service->event_handler_args;
    delete[] this_service->check_command_args;

    free_objectlist(&this_service->servicegroups_ptr);
    delete[] this_service->notification_period;
    delete[] this_service->check_period;
    delete[] this_service->event_handler;
    delete[] this_service->failure_prediction_options;
    delete[] this_service->notes;
    delete[] this_service->notes_url;
    delete[] this_service->action_url;
    delete[] this_service->icon_image;
    delete[] this_service->icon_image_alt;
    delete this_service;
    this_service = next_service;
  }

  /* reset pointers */
  service_list = NULL;

  /**** free memory for the command list ****/
  this_command = command_list;
  while (this_command != NULL) {
    next_command = this_command->next;
    delete[] this_command->name;
    delete[] this_command->command_line;
    delete this_command;
    this_command = next_command;
  }

  /* reset pointers */
  command_list = NULL;

  /**** free memory for the service escalation list ****/
  this_serviceescalation = serviceescalation_list;
  while (this_serviceescalation != NULL) {
    /* free memory for the contact group members */
    this_contactgroupsmember = this_serviceescalation->contact_groups;
    while (this_contactgroupsmember != NULL) {
      next_contactgroupsmember = this_contactgroupsmember->next;
      delete[] this_contactgroupsmember->group_name;
      delete this_contactgroupsmember;
      this_contactgroupsmember = next_contactgroupsmember;
    }

    /* free memory for contacts */
    this_contactsmember = this_serviceescalation->contacts;
    while (this_contactsmember != NULL) {
      next_contactsmember = this_contactsmember->next;
      delete[] this_contactsmember->contact_name;
      delete this_contactsmember;
      this_contactsmember = next_contactsmember;
    }

    next_serviceescalation = this_serviceescalation->next;
    delete[] this_serviceescalation->host_name;
    delete[] this_serviceescalation->description;
    delete[] this_serviceescalation->escalation_period;
    delete this_serviceescalation;
    this_serviceescalation = next_serviceescalation;
  }

  /* reset pointers */
  serviceescalation_list = NULL;

  /**** free memory for the service dependency list ****/
  this_servicedependency = servicedependency_list;
  while (this_servicedependency != NULL) {
    next_servicedependency = this_servicedependency->next;
    delete[] this_servicedependency->dependency_period;
    delete[] this_servicedependency->dependent_host_name;
    delete[] this_servicedependency->dependent_service_description;
    delete[] this_servicedependency->host_name;
    delete[] this_servicedependency->service_description;
    delete this_servicedependency;
    this_servicedependency = next_servicedependency;
  }

  /* reset pointers */
  servicedependency_list = NULL;

  /**** free memory for the host dependency list ****/
  this_hostdependency = hostdependency_list;
  while (this_hostdependency != NULL) {
    next_hostdependency = this_hostdependency->next;
    delete[] this_hostdependency->dependency_period;
    delete[] this_hostdependency->dependent_host_name;
    delete[] this_hostdependency->host_name;
    delete this_hostdependency;
    this_hostdependency = next_hostdependency;
  }

  /* reset pointers */
  hostdependency_list = NULL;

  /**** free memory for the host escalation list ****/
  this_hostescalation = hostescalation_list;
  while (this_hostescalation != NULL) {
    /* free memory for the contact group members */
    this_contactgroupsmember = this_hostescalation->contact_groups;
    while (this_contactgroupsmember != NULL) {
      next_contactgroupsmember = this_contactgroupsmember->next;
      delete[] this_contactgroupsmember->group_name;
      delete this_contactgroupsmember;
      this_contactgroupsmember = next_contactgroupsmember;
    }

    /* free memory for contacts */
    this_contactsmember = this_hostescalation->contacts;
    while (this_contactsmember != NULL) {
      next_contactsmember = this_contactsmember->next;
      delete[] this_contactsmember->contact_name;
      delete this_contactsmember;
      this_contactsmember = next_contactsmember;
    }

    next_hostescalation = this_hostescalation->next;
    delete[] this_hostescalation->host_name;
    delete[] this_hostescalation->escalation_period;
    delete this_hostescalation;
    this_hostescalation = next_hostescalation;
  }

  /* reset pointers */
  hostescalation_list = NULL;

  /* free object skiplists */
  free_object_skiplists();

  return (OK);
}

static int remove_contact_to_contactsmember(contactsmember** cntctsmember,
					    contact* cntct) {
  contactsmember* this_item = *cntctsmember;
  contactsmember* prev_item = NULL;
  while (this_item != NULL && this_item->contact_ptr != cntct) {
    prev_item = this_item;
    this_item = this_item->next;
  }

  // check we have find a contacts member.
  if (this_item == NULL)
    return (0);

  // update list.
  if (prev_item == NULL)
    *cntctsmember = this_item->next;
  else
    prev_item->next = this_item->next;
  delete[] this_item->contact_name;
  delete this_item;
  return (1);
}

static int remove_contactgroup_to_contactgroupsmember(contactgroupsmember** groupsmember,
						      contactgroup* group) {
  contactgroupsmember* this_item = *groupsmember;
  contactgroupsmember* prev_item = NULL;
  while (this_item != NULL && this_item->group_ptr != group) {
    prev_item = this_item;
    this_item = this_item->next;
  }

  // check we have find a contact groups member.
  if (this_item == NULL)
    return (0);

  // update list.
  if (prev_item == NULL)
    *groupsmember = this_item->next;
  else
    prev_item->next = this_item->next;
  delete[] this_item->group_name;
  delete this_item;
  return (1);
}

static int remove_service(service* this_service) {
  // check we have find a service.
  if (this_service == NULL)
    return (0);

  // update host service list.
  host* hst = find_host(this_service->host_name);
  if (hst != NULL) {
    for (servicesmember* svcmbr = hst->services, *prev = NULL;
	 svcmbr != NULL;
	 svcmbr = svcmbr->next) {
      if (svcmbr->service_ptr == this_service) {
	if (prev == NULL)
	  hst->services = svcmbr->next;
	else
	  prev->next = svcmbr->next;
	delete[] svcmbr->host_name;
	delete[] svcmbr->service_description;
	delete svcmbr;
	break;
      }
      prev = svcmbr;
    }
  }

 // update the event list low.
  for (timed_event* temp_event = event_list_low;
    temp_event != NULL;
    temp_event = temp_event->next) {
    if (temp_event->event_data == this_service) {
      remove_event(temp_event, &event_list_low, &event_list_low_tail);
      delete temp_event;
      break;
    }
  }

  // update the event list high.
  for (timed_event* temp_event = event_list_high;
       temp_event != NULL;
       temp_event = temp_event->next) {
    if (temp_event->event_data == this_service) {
      remove_event(temp_event, &event_list_high, &event_list_high_tail);
      delete temp_event;
      break;
    }
  }

  // update the servicedependency list.
  for (servicedependency* dep = servicedependency_list, *prev = NULL; dep != NULL; dep = dep->next) {
    if (dep->master_service_ptr == this_service || dep->dependent_service_ptr == this_service) {
      skiplist_delete(object_skiplists[SERVICEDEPENDENCY_SKIPLIST], dep);
      if (prev == NULL)
	servicedependency_list = dep->next;
      else
	prev->next = dep->next;
      if (dep->next == NULL)
	servicedependency_list_tail = prev;
      delete[] dep->dependent_host_name;
      delete[] dep->dependent_service_description;
      delete[] dep->host_name;
      delete[] dep->service_description;
      delete[] dep->dependency_period;
      delete dep;
      break;
    }
    prev = dep;
  }

  // update the serviceescalation list.
  for (serviceescalation* escalation = serviceescalation_list, *prev = NULL;
       escalation != NULL;
       prev = escalation, escalation = escalation->next) {
      skiplist_delete(object_skiplists[SERVICEESCALATION_SKIPLIST], escalation);
    if (escalation->service_ptr == this_service) {
      if (prev == NULL)
	serviceescalation_list = escalation->next;
      else
	prev->next = escalation->next;
      if (escalation->next == NULL)
	serviceescalation_list_tail = prev;
      delete[] escalation->host_name;
      delete[] escalation->description;
      delete[] escalation->escalation_period;
      delete escalation;
      break;
    }
  }

  // update the service skiplist.
  skiplist_delete_all(object_skiplists[SERVICE_SKIPLIST], (void*)this_service);

  /* free memory for contact groups */
  contactgroupsmember* this_contactgroupsmember = this_service->contact_groups;
  while (this_contactgroupsmember != NULL) {
    contactgroupsmember* next_contactgroupsmember = this_contactgroupsmember->next;
    delete[] this_contactgroupsmember->group_name;
    delete this_contactgroupsmember;
    this_contactgroupsmember = next_contactgroupsmember;
  }

  /* free memory for contacts */
  contactsmember* this_contactsmember = this_service->contacts;
  while (this_contactsmember != NULL) {
    contactsmember* next_contactsmember = this_contactsmember->next;
    delete[] this_contactsmember->contact_name;
    delete this_contactsmember;
    this_contactsmember = next_contactsmember;
  }

  /* free memory for custom variables */
  customvariablesmember* this_customvariablesmember = this_service->custom_variables;
  while (this_customvariablesmember != NULL) {
    customvariablesmember* next_customvariablesmember = this_customvariablesmember->next;
    delete[] this_customvariablesmember->variable_name;
    delete[] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
    this_customvariablesmember = next_customvariablesmember;
  }

  // cleanup memory.
  delete[] this_service->host_name;
  delete[] this_service->description;
  delete[] this_service->display_name;
  delete[] this_service->service_check_command;
  delete[] this_service->plugin_output;
  delete[] this_service->long_plugin_output;
  delete[] this_service->perf_data;

  delete[] this_service->event_handler_args;
  delete[] this_service->check_command_args;

  free_objectlist(&this_service->servicegroups_ptr);
  delete[] this_service->notification_period;
  delete[] this_service->check_period;
  delete[] this_service->event_handler;
  delete[] this_service->failure_prediction_options;
  delete[] this_service->notes;
  delete[] this_service->notes_url;
  delete[] this_service->action_url;
  delete[] this_service->icon_image;
  delete[] this_service->icon_image_alt;
  delete this_service;

  return (1);
}

int remove_service_by_id(char const* host_name, char const* description) {
  if (host_name == NULL || description == NULL)
    return (0);

  service* this_service = service_list;
  service* prev_service = NULL;
  while (this_service != NULL) {
    if (!strcmp(this_service->host_name, host_name)
	&& !strcmp(this_service->description, description))
      break;
    prev_service = this_service;
    this_service = this_service->next;
  }

  // check we have find a service.
  if (this_service == NULL)
    return (0);

  // update the service list.
  if (prev_service == NULL)
    service_list = this_service->next;
  else
    prev_service->next = this_service->next;
  if (this_service->next == NULL)
    service_list_tail = prev_service;
  return (remove_service(this_service));
}

static int remove_servicegroup(servicegroup* this_servicegroup) {
  // update service list.
  for (service* svc = service_list; svc != NULL; svc = svc->next)
    remove_object_to_objectlist(&svc->servicegroups_ptr, this_servicegroup);

  servicesmember* this_servicesmember = this_servicegroup->members;
  while (this_servicesmember != NULL) {
    servicesmember* next_servicesmember = this_servicesmember->next;
    delete[] this_servicesmember->host_name;
    delete[] this_servicesmember->service_description;
    delete this_servicesmember;
    this_servicesmember = next_servicesmember;
  }

  // update the servicegroup skiplist.
  skiplist_delete_all(object_skiplists[SERVICEGROUP_SKIPLIST], (void*)this_servicegroup);

  delete[] this_servicegroup->group_name;
  delete[] this_servicegroup->alias;
  delete[] this_servicegroup->notes;
  delete[] this_servicegroup->notes_url;
  delete[] this_servicegroup->action_url;
  delete this_servicegroup;

  return (1);
}

int remove_servicegroup_by_id(char const* name) {
  if (name == NULL)
    return (0);

  servicegroup* this_servicegroup = servicegroup_list;
  servicegroup* prev_servicegroup = NULL;
  while (this_servicegroup != NULL && strcmp(this_servicegroup->group_name, name)) {
    prev_servicegroup = this_servicegroup;
    this_servicegroup = this_servicegroup->next;
  }

  // check we have find a service group.
  if (this_servicegroup == NULL)
    return (0);

  // update the servicegroup list.
  if (prev_servicegroup == NULL)
    servicegroup_list = this_servicegroup->next;
  else
    prev_servicegroup->next = this_servicegroup->next;
  if (this_servicegroup->next == NULL)
    servicegroup_list_tail = prev_servicegroup;
  return (remove_servicegroup(this_servicegroup));
}

static int remove_host(host* this_host) {
  // check we have find a host.
  if (this_host == NULL)
    return (0);

  // update the event list low.
  for (timed_event* temp_event = event_list_low;
    temp_event != NULL;
    temp_event = temp_event->next) {
    if (temp_event->event_data == this_host) {
      remove_event(temp_event, &event_list_low, &event_list_low_tail);
      delete temp_event;
      break;
    }
  }

  // update the event list high.
  for (timed_event* temp_event = event_list_high;
       temp_event != NULL;
       temp_event = temp_event->next) {
    if (temp_event->event_data == this_host) {
      remove_event(temp_event, &event_list_high, &event_list_high_tail);
      delete temp_event;
      break;
    }
  }

  // update the hostdependency list.
  for (hostdependency* dep = hostdependency_list, *prev = NULL; dep != NULL; dep = dep->next) {
    if (dep->master_host_ptr == this_host || dep->dependent_host_ptr == this_host) {
      skiplist_delete(object_skiplists[HOSTDEPENDENCY_SKIPLIST], dep);
      if (prev == NULL)
	hostdependency_list = dep->next;
      else
	prev->next = dep->next;
      if (dep->next == NULL)
	hostdependency_list_tail = prev;
      delete[] dep->dependent_host_name;
      delete[] dep->host_name;
      delete[] dep->dependency_period;
      delete dep;
      break;
    }
    prev = dep;
  }

  // update the hostescalation list.
  for (hostescalation* escalation = hostescalation_list, *prev = NULL;
       escalation != NULL;
       prev = escalation, escalation = escalation->next) {
    skiplist_delete(object_skiplists[HOSTESCALATION_SKIPLIST], escalation);
    if (escalation->host_ptr == this_host) {
      if (prev == NULL)
	hostescalation_list = escalation->next;
      else
	prev->next = escalation->next;
      if (escalation->next == NULL)
	hostescalation_list_tail = prev;
      delete[] escalation->host_name;
      delete[] escalation->escalation_period;
      delete escalation;
      break;
    }
  }

  // update the host skiplist.
  skiplist_delete_all(object_skiplists[HOST_SKIPLIST], (void*)this_host);

  /* free memory for parent hosts */
  hostsmember* this_hostsmember = this_host->parent_hosts;
  while (this_hostsmember != NULL) {
    hostsmember* next_hostsmember = this_hostsmember->next;
    delete[] this_hostsmember->host_name;
    delete this_hostsmember;
    this_hostsmember = next_hostsmember;
  }

  /* free memory for child host links */
  this_hostsmember = this_host->child_hosts;
  while (this_hostsmember != NULL) {
    hostsmember* next_hostsmember = this_hostsmember->next;
    delete[] this_hostsmember->host_name;
    delete this_hostsmember;
    this_hostsmember = next_hostsmember;
  }

  /* free memory for service links */
  servicesmember* this_servicesmember = this_host->services;
  while (this_servicesmember != NULL) {
    servicesmember* next_servicesmember = this_servicesmember->next;
    service* svc = this_servicesmember->service_ptr;
    delete[] this_servicesmember->host_name;
    delete[] this_servicesmember->service_description;
    delete this_servicesmember;
    remove_service_by_id(svc->host_name, svc->description);
    this_servicesmember = next_servicesmember;
  }

  /* free memory for contact groups */
  contactgroupsmember* this_contactgroupsmember = this_host->contact_groups;
  while (this_contactgroupsmember != NULL) {
    contactgroupsmember* next_contactgroupsmember = this_contactgroupsmember->next;
    delete[] this_contactgroupsmember->group_name;
    delete this_contactgroupsmember;
    this_contactgroupsmember = next_contactgroupsmember;
  }

  /* free memory for contacts */
  contactsmember* this_contactsmember = this_host->contacts;
  while (this_contactsmember != NULL) {
    contactsmember* next_contactsmember = this_contactsmember->next;
    delete[] this_contactsmember->contact_name;
    delete this_contactsmember;
    this_contactsmember = next_contactsmember;
  }

  /* free memory for custom variables */
  customvariablesmember* this_customvariablesmember = this_host->custom_variables;
  while (this_customvariablesmember != NULL) {
    customvariablesmember* next_customvariablesmember = this_customvariablesmember->next;
    delete[] this_customvariablesmember->variable_name;
    delete[] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
    this_customvariablesmember = next_customvariablesmember;
  }

  delete[] this_host->name;
  delete[] this_host->display_name;
  delete[] this_host->alias;
  delete[] this_host->address;
  delete[] this_host->plugin_output;
  delete[] this_host->long_plugin_output;
  delete[] this_host->perf_data;

  free_objectlist(&this_host->hostgroups_ptr);
  delete[] this_host->check_period;
  delete[] this_host->host_check_command;
  delete[] this_host->event_handler;
  delete[] this_host->failure_prediction_options;
  delete[] this_host->notification_period;
  delete[] this_host->notes;
  delete[] this_host->notes_url;
  delete[] this_host->action_url;
  delete[] this_host->icon_image;
  delete[] this_host->icon_image_alt;
  delete[] this_host->vrml_image;
  delete[] this_host->statusmap_image;
  delete this_host;

  return (1);
}

int remove_host_by_id(char const* name) {
  if (name == NULL)
    return (0);

  host* this_host = host_list;
  host* prev_host = NULL;
  while (this_host != NULL && strcmp(this_host->name, name)) {
    prev_host = this_host;
    this_host = this_host->next;
  }

  // check we have find a host.
  if (this_host == NULL)
    return (0);

  // update the host list.
  if (prev_host == NULL)
    host_list = this_host->next;
  else
    prev_host->next = this_host->next;
  if (this_host->next == NULL)
    host_list_tail = prev_host;
  return (remove_host(this_host));
}

static int remove_hostgroup(hostgroup* this_hostgroup) {
  // update host list.
  for (host* hst = host_list; hst != NULL; hst = hst->next)
    remove_object_to_objectlist(&hst->hostgroups_ptr, this_hostgroup);

  hostsmember* this_hostsmember = this_hostgroup->members;
  while (this_hostsmember != NULL) {
    hostsmember* next_hostsmember = this_hostsmember->next;
    delete[] this_hostsmember->host_name;
    delete this_hostsmember;
    this_hostsmember = next_hostsmember;
  }

  // update the hostgroup skiplist.
  skiplist_delete_all(object_skiplists[HOSTGROUP_SKIPLIST], (void*)this_hostgroup);

  delete[] this_hostgroup->group_name;
  delete[] this_hostgroup->alias;
  delete[] this_hostgroup->notes;
  delete[] this_hostgroup->notes_url;
  delete[] this_hostgroup->action_url;
  delete this_hostgroup;

  return (1);
}

int remove_hostgroup_by_id(char const* name) {
  if (name == NULL)
    return (0);

  hostgroup* this_hostgroup = hostgroup_list;
  hostgroup* prev_hostgroup = NULL;
  while (this_hostgroup != NULL && strcmp(this_hostgroup->group_name, name)) {
    prev_hostgroup = this_hostgroup;
    this_hostgroup = this_hostgroup->next;
  }

  // check we have find a host group.
  if (this_hostgroup == NULL)
    return (0);

  // update the hostgroup list.
  if (prev_hostgroup == NULL)
    hostgroup_list = this_hostgroup->next;
  else
    prev_hostgroup->next = this_hostgroup->next;
  if (this_hostgroup->next == NULL)
    hostgroup_list_tail = prev_hostgroup;
  return (remove_hostgroup(this_hostgroup));
}

static int remove_contact(contact* this_contact) {
  // remove contact from contactgroup.
  contactgroup* this_contactgroup = contactgroup_list;
  while (this_contactgroup != NULL) {
    remove_contact_to_contactsmember(&this_contactgroup->members, this_contact);
    this_contactgroup = this_contactgroup->next;
  }

  // remove contact from host.
  host* this_host = host_list;
  while (this_host != NULL) {
    remove_contact_to_contactsmember(&this_host->contacts, this_contact);
    this_host = this_host->next;
  }

  // remove contact from service.
  service* this_service = service_list;
  while (this_service != NULL) {
    remove_contact_to_contactsmember(&this_service->contacts, this_contact);
    this_service = this_service->next;
  }

  // remove contact from hostescalation.
  hostescalation* this_hostescalation = hostescalation_list;
  while (this_hostescalation != NULL) {
    remove_contact_to_contactsmember(&this_hostescalation->contacts, this_contact);
    this_hostescalation = this_hostescalation->next;
  }

  // remove contact from serviceescalation.
  serviceescalation* this_serviceescalation = serviceescalation_list;
  while (this_serviceescalation != NULL) {
    remove_contact_to_contactsmember(&this_serviceescalation->contacts, this_contact);
    this_serviceescalation = this_serviceescalation->next;
  }

  /* free memory for the host notification commands */
  commandsmember* this_commandsmember = this_contact->host_notification_commands;
  while (this_commandsmember != NULL) {
    commandsmember* next_commandsmember = this_commandsmember->next;
    if (this_commandsmember->cmd != NULL)
      delete[] this_commandsmember->cmd;
    delete this_commandsmember;
    this_commandsmember = next_commandsmember;
  }

  /* free memory for the service notification commands */
  this_commandsmember = this_contact->service_notification_commands;
  while (this_commandsmember != NULL) {
    commandsmember* next_commandsmember = this_commandsmember->next;
    if (this_commandsmember->cmd != NULL)
      delete[] this_commandsmember->cmd;
    delete this_commandsmember;
    this_commandsmember = next_commandsmember;
  }

  /* free memory for custom variables */
  customvariablesmember* this_customvariablesmember = this_contact->custom_variables;
  while (this_customvariablesmember != NULL) {
    customvariablesmember* next_customvariablesmember = this_customvariablesmember->next;
    delete[] this_customvariablesmember->variable_name;
    delete[] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
    this_customvariablesmember = next_customvariablesmember;
  }

  // update the contact skiplist.
  skiplist_delete_all(object_skiplists[CONTACT_SKIPLIST], (void*)this_contact);

  // update the object list.
  free_objectlist(&this_contact->contactgroups_ptr);

  for (unsigned int i = 0; i < MAX_CONTACT_ADDRESSES; ++i)
    delete[] this_contact->address[i];
  delete[] this_contact->name;
  delete[] this_contact->alias;
  delete[] this_contact->email;
  delete[] this_contact->pager;
  delete[] this_contact->host_notification_period;
  delete[] this_contact->service_notification_period;
  delete this_contact;

  return (1);
}

int remove_contact_by_id(char const* name) {
  if (name == NULL)
    return (0);

  contact* this_contact = contact_list;
  contact* prev_contact = NULL;
  while (this_contact != NULL && strcmp(this_contact->name, name)) {
    prev_contact = this_contact;
    this_contact = this_contact->next;
  }

  // check we have find a contact.
  if (this_contact == NULL)
    return (0);

  // update the contact list.
  if (prev_contact == NULL)
    contact_list = this_contact->next;
  else
    prev_contact->next = this_contact->next;
  if (this_contact->next == NULL)
    contact_list_tail = prev_contact;
  return (remove_contact(this_contact));
}

static int remove_contactgroup(contactgroup* this_contactgroup) {
  // check we have find a contact group.
  if (this_contactgroup == NULL)
    return (0);

  // remove contactgroup from host.
  host* this_host = host_list;
  while (this_host != NULL) {
    remove_contactgroup_to_contactgroupsmember(&this_host->contact_groups, this_contactgroup);
    this_host = this_host->next;
  }

  // remove contactgroup from service.
  service* this_service = service_list;
  while (this_service != NULL) {
    remove_contactgroup_to_contactgroupsmember(&this_service->contact_groups, this_contactgroup);
    this_service = this_service->next;
  }

  // remove contactgroup from hostescalation.
  hostescalation* this_hostescalation = hostescalation_list;
  while (this_hostescalation != NULL) {
    remove_contactgroup_to_contactgroupsmember(&this_hostescalation->contact_groups,
					       this_contactgroup);
    this_hostescalation = this_hostescalation->next;
  }

  // remove contactgroup from serviceescalation.
  serviceescalation* this_serviceescalation = serviceescalation_list;
  while (this_serviceescalation != NULL) {
    remove_contactgroup_to_contactgroupsmember(&this_serviceescalation->contact_groups,
					       this_contactgroup);
    this_serviceescalation = this_serviceescalation->next;
  }

  // update the contactgroup skiplist.
  skiplist_delete_all(object_skiplists[CONTACTGROUP_SKIPLIST], (void*)this_contactgroup);

  delete[] this_contactgroup->group_name;
  delete[] this_contactgroup->alias;
  delete this_contactgroup;

  return (1);
}

int remove_contactgroup_by_id(char const* name) {
  if (name == NULL)
    return (0);

  contactgroup* this_contactgroup = contactgroup_list;
  contactgroup* prev_contactgroup = NULL;
  while (this_contactgroup != NULL
	 && strcmp(this_contactgroup->group_name, name)) {
    prev_contactgroup = this_contactgroup;
    this_contactgroup = this_contactgroup->next;
  }

  // check we have find a contact group.
  if (this_contactgroup == NULL)
    return (0);

  // update the contactgroup list.
  if (prev_contactgroup == NULL)
    contactgroup_list = this_contactgroup->next;
  else
    prev_contactgroup->next = this_contactgroup->next;
  if (this_contactgroup->next == NULL)
    contactgroup_list_tail = prev_contactgroup;
  return (remove_contactgroup(this_contactgroup));
}

static int remove_command(command* this_command) {
  // update the command skiplist.
  skiplist_delete_all(object_skiplists[COMMAND_SKIPLIST], (void*)this_command);

  delete[] this_command->name;
  delete[] this_command->command_line;
  delete this_command;

  return (1);
}


int remove_command_by_id(char const* name) {
  if (name == NULL)
    return (0);

  command* this_command = command_list;
  command* prev_command = NULL;
  while (this_command != NULL
	 && strcmp(this_command->name, name)) {
    prev_command = this_command;
    this_command = this_command->next;
  }

  // check we have find a command.
  if (this_command == NULL)
    return (0);

  // check if command are use by a host.
  for (host* hst = host_list; hst != NULL; hst = hst->next) {
    if (hst->event_handler_ptr == this_command
	|| hst->check_command_ptr == this_command) {
      return (2);
    }
  }

  // check if command are use by a service.
  for (service* svc = service_list; svc != NULL; svc = svc->next) {
    if (svc->event_handler_ptr == this_command
	|| svc->check_command_ptr == this_command) {
      return (2);
    }
  }

  // check if command are use by a contact.
  for (contact* cntct = contact_list; cntct != NULL; cntct = cntct->next) {
    for (commandsmember* cntctsmember = cntct->host_notification_commands;
	 cntctsmember != NULL;
	 cntctsmember = cntctsmember->next) {
      if (cntctsmember->command_ptr == this_command) {
	return (2);
      }
    }
    for (commandsmember* cntctsmember = cntct->service_notification_commands;
	 cntctsmember != NULL;
	 cntctsmember = cntctsmember->next) {
      if (cntctsmember->command_ptr == this_command) {
	return (2);
      }
    }
  }

  // update the command list.
  if (prev_command == NULL)
    command_list = this_command->next;
  else
    prev_command->next = this_command->next;
  if (this_command->next == NULL)
    command_list_tail = prev_command;
  return (remove_command(this_command));
}

int remove_serviceescalation_by_id(char const* host_name,
				   char const* service_description) {
  if (host_name == NULL || service_description == NULL)
    return (0);

  serviceescalation* this_serviceescalation = serviceescalation_list;
  serviceescalation* prev_serviceescalation = NULL;
  while (this_serviceescalation != NULL
	 && strcmp(this_serviceescalation->host_name, host_name)
	 && strcmp(this_serviceescalation->description, service_description)) {
    prev_serviceescalation = this_serviceescalation;
    this_serviceescalation = this_serviceescalation->next;
  }

  // check we have find a serviceescalation.
  if (this_serviceescalation == NULL)
    return (0);

  // update the serviceescalation list.
  if (prev_serviceescalation == NULL)
    serviceescalation_list = this_serviceescalation->next;
  else
    prev_serviceescalation->next = this_serviceescalation->next;
  if (this_serviceescalation->next == NULL)
    serviceescalation_list_tail = prev_serviceescalation;

  contactgroupsmember* this_contactgroupsmembers = this_serviceescalation->contact_groups;
  while (this_contactgroupsmembers != NULL) {
    contactgroupsmember* tmp = this_contactgroupsmembers->next;

    delete[] this_contactgroupsmembers->group_name;
    delete this_contactgroupsmembers;
    this_contactgroupsmembers = tmp;
  }

  contactsmember* this_contactsmember = this_serviceescalation->contacts;
  while (this_contactsmember != NULL) {
    contactsmember* tmp = this_contactsmember->next;

    delete[] this_contactsmember->contact_name;
    delete this_contactsmember;
    this_contactsmember = tmp;
  }

  // update the serviceescalation skiplist.
  skiplist_delete_all(object_skiplists[SERVICEESCALATION_SKIPLIST], (void*)this_serviceescalation);

  delete[] this_serviceescalation->host_name;
  delete[] this_serviceescalation->description;
  delete[] this_serviceescalation->escalation_period;
  delete this_serviceescalation;
  return (1);
}

int remove_servicedependency_by_id(char const* host_name,
				   char const* service_description,
				   char const* dependent_host_name,
				   char const* dependent_service_description) {
  if (host_name == NULL
      || service_description == NULL
      || dependent_host_name == NULL
      || dependent_service_description == NULL)
    return (0);

  servicedependency* this_servicedependency = servicedependency_list;
  servicedependency* prev_servicedependency = NULL;
  while (this_servicedependency != NULL
	 && strcmp(this_servicedependency->host_name, host_name)
	 && strcmp(this_servicedependency->service_description, service_description)
	 && strcmp(this_servicedependency->dependent_host_name, dependent_host_name)
	 && strcmp(this_servicedependency->dependent_service_description,
		   dependent_service_description)) {
    prev_servicedependency = this_servicedependency;
    this_servicedependency = this_servicedependency->next;
  }

  // check we have find a servicedependency.
  if (this_servicedependency == NULL)
    return (0);

  // update the servicedependency list.
  if (prev_servicedependency == NULL)
    servicedependency_list = this_servicedependency->next;
  else
    prev_servicedependency->next = this_servicedependency->next;
  if (this_servicedependency->next == NULL)
    servicedependency_list_tail = prev_servicedependency;

  // update the servicedependency skiplist.
  skiplist_delete_all(object_skiplists[SERVICEDEPENDENCY_SKIPLIST], (void*)this_servicedependency);

  delete[] this_servicedependency->dependent_host_name;
  delete[] this_servicedependency->dependent_service_description;
  delete[] this_servicedependency->host_name;
  delete[] this_servicedependency->service_description;
  delete[] this_servicedependency->dependency_period;
  delete this_servicedependency;
  return (1);
}

int remove_hostescalation_by_id(char const* host_name) {
  if (host_name == NULL)
    return (0);

  hostescalation* this_hostescalation = hostescalation_list;
  hostescalation* prev_hostescalation = NULL;
  while (this_hostescalation != NULL
	 && strcmp(this_hostescalation->host_name, host_name)) {
    prev_hostescalation = this_hostescalation;
    this_hostescalation = this_hostescalation->next;
  }

  // check we have find a hostescalation.
  if (this_hostescalation == NULL)
    return (0);

  // update the hostescalation list.
  if (prev_hostescalation == NULL)
    hostescalation_list = this_hostescalation->next;
  else
    prev_hostescalation->next = this_hostescalation->next;
  if (this_hostescalation->next == NULL)
    hostescalation_list_tail = prev_hostescalation;

  contactgroupsmember* this_contactgroupsmembers = this_hostescalation->contact_groups;
  while (this_contactgroupsmembers != NULL) {
    contactgroupsmember* tmp = this_contactgroupsmembers->next;

    delete[] this_contactgroupsmembers->group_name;
    delete this_contactgroupsmembers;
    this_contactgroupsmembers = tmp;
  }

  contactsmember* this_contactsmember = this_hostescalation->contacts;
  while (this_contactsmember != NULL) {
    contactsmember* tmp = this_contactsmember->next;

    delete[] this_contactsmember->contact_name;
    delete this_contactsmember;
    this_contactsmember = tmp;
  }

  // update the hostescalation skiplist.
  skiplist_delete_all(object_skiplists[HOSTESCALATION_SKIPLIST], (void*)this_hostescalation);

  delete[] this_hostescalation->host_name;
  delete[] this_hostescalation->escalation_period;
  delete this_hostescalation;
  return (1);
}

int remove_hostdependency_by_id(char const* host_name,
				char const* dependent_host_name) {
  if (host_name == NULL || dependent_host_name == NULL)
     return (0);

  hostdependency* this_hostdependency = hostdependency_list;
  hostdependency* prev_hostdependency = NULL;
  while (this_hostdependency != NULL
	 && strcmp(this_hostdependency->host_name, host_name)
	 && strcmp(this_hostdependency->dependent_host_name, dependent_host_name)) {
    prev_hostdependency = this_hostdependency;
    this_hostdependency = this_hostdependency->next;
  }

  // check we have find a hostdependency.
  if (this_hostdependency == NULL)
    return (0);

  // update the hostdependency list.
  if (prev_hostdependency == NULL)
    hostdependency_list = this_hostdependency->next;
  else
    prev_hostdependency->next = this_hostdependency->next;
  if (this_hostdependency->next == NULL)
    hostdependency_list_tail = prev_hostdependency;

  // update the hostdependency skiplist.
  skiplist_delete_all(object_skiplists[HOSTDEPENDENCY_SKIPLIST], (void*)this_hostdependency);

  delete[] this_hostdependency->dependent_host_name;
  delete[] this_hostdependency->host_name;
  delete[] this_hostdependency->dependency_period;
  delete this_hostdependency;
  return (1);
}

static int remove_timerange(timerange* this_timerange) {
  while (this_timerange != NULL) {
    timerange* tmp = this_timerange->next;
    delete this_timerange;
    this_timerange = tmp;
  }
  return (1);
}

static int remove_daterange(daterange* this_daterange) {
  while (this_daterange != NULL) {
    daterange* tmp = this_daterange->next;
    remove_timerange(this_daterange->times);
    delete this_daterange;
    this_daterange = tmp;
  }
  return (1);
}

static int remove_timeperiodexclusions(timeperiodexclusion* this_timeperiodexclusion) {
  while (this_timeperiodexclusion != NULL) {
    timeperiodexclusion* tmp = this_timeperiodexclusion->next;
    delete[] this_timeperiodexclusion->timeperiod_name;
    delete this_timeperiodexclusion;
    this_timeperiodexclusion = tmp;
  }
  return (1);
}

static int remove_timeperiod(timeperiod* this_timeperiod) {
  if (this_timeperiod == NULL)
    return (0);

  // remove all timerange.
  for (unsigned int i = 0;
       i < sizeof(this_timeperiod->days) / sizeof(*this_timeperiod->days);
       ++i) {
    remove_timerange(this_timeperiod->days[i]);
  }

  // remove all exceptions.
  for (unsigned int i = 0;
       i < sizeof(this_timeperiod->exceptions) / sizeof(*this_timeperiod->exceptions);
       ++i) {
    remove_daterange(this_timeperiod->exceptions[i]);
  }

  // remove all exclusions.
  remove_timeperiodexclusions(this_timeperiod->exclusions);

  // remove all timeperiod used by contacts.
  for (contact* cntct = contact_list; cntct != NULL; cntct = cntct->next) {
    if (cntct->host_notification_period_ptr == this_timeperiod)
      cntct->host_notification_period_ptr = NULL;
    if (cntct->service_notification_period_ptr == this_timeperiod)
      cntct->service_notification_period_ptr = NULL;
  }

  // remove all timeperiod used by hosts.
  for (host* hst = host_list; hst != NULL; hst = hst->next) {
    if (hst->check_period_ptr == this_timeperiod)
      hst->check_period_ptr = NULL;
    if (hst->notification_period_ptr == this_timeperiod)
      hst->notification_period_ptr = NULL;
  }

  // remove all timeperiod used by services.
  for (service* svc = service_list; svc != NULL; svc = svc->next) {
    if (svc->check_period_ptr == this_timeperiod)
      svc->check_period_ptr = NULL;
    if (svc->notification_period_ptr == this_timeperiod)
      svc->notification_period_ptr = NULL;
  }

  // remove all timeperiod used by serviceescalations.
  for (serviceescalation* se = serviceescalation_list; se != NULL; se = se->next) {
    if (se->escalation_period_ptr == this_timeperiod)
      se->escalation_period_ptr = NULL;
  }

  // remove all timeperiod used by servicedependencies.
  for (servicedependency* sd = servicedependency_list; sd != NULL; sd = sd->next) {
    if (sd->dependency_period_ptr == this_timeperiod)
      sd->dependency_period_ptr = NULL;
  }

  // remove all timeperiod used by hostescalations.
  for (hostescalation* he = hostescalation_list; he != NULL; he = he->next) {
    if (he->escalation_period_ptr == this_timeperiod)
      he->escalation_period_ptr = NULL;
  }

  // remove all timeperiod used by hostdependencies.
  for (hostdependency* hd = hostdependency_list; hd != NULL; hd = hd->next) {
    if (hd->dependency_period_ptr == this_timeperiod)
      hd->dependency_period_ptr = NULL;
  }

  // update the timeperiod skiplist.
  skiplist_delete_all(object_skiplists[TIMEPERIOD_SKIPLIST], (void*)this_timeperiod);

  delete[] this_timeperiod->name;
  delete[] this_timeperiod->alias;
  delete this_timeperiod;

  return (1);
}

int remove_timeperiod_by_id(char const* name) {
  if (name == NULL)
    return (0);

  timeperiod* this_timeperiod = timeperiod_list;
  timeperiod* prev_timeperiod = NULL;
  while (this_timeperiod != NULL
	 && strcmp(this_timeperiod->name, name)) {
    prev_timeperiod = this_timeperiod;
    this_timeperiod = this_timeperiod->next;
  }

  // check we have find a timeperiod.
  if (this_timeperiod == NULL)
    return (0);

  // update the timeperiod list.
  if (prev_timeperiod == NULL)
    timeperiod_list = this_timeperiod->next;
  else
    prev_timeperiod->next = this_timeperiod->next;
  if (this_timeperiod->next == NULL)
    timeperiod_list_tail = prev_timeperiod;
  return (remove_timeperiod(this_timeperiod));
}
