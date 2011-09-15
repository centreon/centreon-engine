/*
** Copyright 2011 Merethis
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

#include <QDebug>
#include <exception>
#include "error.hh"
#include "objects.hh"
#include "utils.hh"
#include "macros.hh"
#include "globals.hh"

using namespace com::centreon::engine;

/**
 *  Check if remove timeperiod works with some timeperiods.
 */
static void remove_all_timeperiod() {
  init_object_skiplists();

  add_timeperiod("timeperiod_name_1", "timeperiod_alias");
  add_timeperiod("timeperiod_name_2", "timeperiod_alias");
  add_timeperiod("timeperiod_name_3", "timeperiod_alias");

  if (remove_timeperiod_by_id("timeperiod_name_2") != 1
      || remove_timeperiod_by_id("timeperiod_name_1") != 1
      || remove_timeperiod_by_id("timeperiod_name_3") != 1
      || timeperiod_list != NULL
      || timeperiod_list_tail != NULL)
    throw (engine_error() << "remove all timeperiod failed.");

  free_object_skiplists();
}

/**
 *  Check if remove timeperiod works with invalid call.
 */
static void remove_timeperiod_failed() {
  init_object_skiplists();

  if (remove_timeperiod_by_id("") == 1)
    throw (engine_error() << "timeperiod remove but dosen't exist.");
  if (remove_timeperiod_by_id(NULL) == 1)
    throw (engine_error() << "timeperiod remove but pointer is NULL.");

  free_object_skiplists();
}

/**
 *  Check if remove timeperiod works with some timetranges.
 */
static void remove_timeperiod_with_timeranges() {
  init_object_skiplists();

  timeperiod* t = add_timeperiod("timeperiod_name", "timeperiod_alias");
  for (unsigned int i = 0; i < sizeof(t->days) / sizeof(*t->days); ++i) {
    add_timerange_to_timeperiod(t, i, 86400 + i * 10, 86400 + i * 20);
    add_timerange_to_timeperiod(t, i, 86400 + i * 10, 86400 + i * 20);
    add_timerange_to_timeperiod(t, i, 86400 + i * 10, 86400 + i * 20);
  }

  if (remove_timeperiod_by_id("timeperiod_name") != 1
      || timeperiod_list != NULL
      || timeperiod_list_tail != NULL)
    throw (engine_error() << "remove timeperiod with timerange failed.");

  free_object_skiplists();
}

/**
 *  Check if remove timeperiod works with some exclusions.
 */
static void remove_timeperiod_with_exclusions() {
  init_object_skiplists();

  timeperiod* t = add_timeperiod("timeperiod_name", "timeperiod_alias");
  for (unsigned int i = 0; i < 10; ++i)
    add_exclusion_to_timeperiod(t, "timeperiodexclusion_name");

  if (remove_timeperiod_by_id("timeperiod_name") != 1
      || timeperiod_list != NULL
      || timeperiod_list_tail != NULL)
    throw (engine_error() << "remove timeperiod with exclusions failed.");

  free_object_skiplists();
}

/**
 *  Check if remove timeperiod works with some exceptions.
 */
static void remove_timeperiod_with_exceptions() {
  init_object_skiplists();

  timeperiod* t = add_timeperiod("timeperiod_name", "timeperiod_alias");
  for (unsigned int i = 0; i < 10; ++i)
    add_exception_to_timeperiod(t, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  if (remove_timeperiod_by_id("timeperiod_name") != 1
      || timeperiod_list != NULL
      || timeperiod_list_tail != NULL)
    throw (engine_error() << "remove timeperiod with exceptions failed.");

  free_object_skiplists();
}

/**
 *  Check if remove timeperiod works with some contacts.
 */
static void remove_timeperiod_with_contacts() {
  init_object_skiplists();

  timeperiod* t = add_timeperiod("timeperiod_name", "timeperiod_alias");
  contact* cntct = add_contact("contact_name", NULL, NULL, NULL, NULL, NULL,
                               NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  cntct->host_notification_period_ptr = t;
  cntct->service_notification_period_ptr = t;

  if (remove_timeperiod_by_id("timeperiod_name") != 1
      || timeperiod_list != NULL
      || timeperiod_list_tail != NULL
      || cntct->host_notification_period_ptr != NULL
      || cntct->service_notification_period_ptr != NULL)
    throw (engine_error() << "remove timeperiod with contacts failed.");

  delete[] cntct->name;
  delete[] cntct->alias;
  delete cntct;

  contact_list = NULL;
  contact_list_tail = NULL;
  free_object_skiplists();
}

/**
 *  Check if remove timeperiod works with some hosts.
 */
static void remove_timeperiod_with_hosts() {
  init_object_skiplists();

  timeperiod* t = add_timeperiod("timeperiod_name", "timeperiod_alias");
  host* hst = add_host("host_name", "host_display_name", "host_alias",
		       "localhost", NULL, 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
                       0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0,
                       0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
                       NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0,
                       0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  hst->check_period_ptr = t;
  hst->notification_period_ptr = t;


  if (remove_timeperiod_by_id("timeperiod_name") != 1
      || timeperiod_list != NULL
      || timeperiod_list_tail != NULL
      || hst->check_period_ptr != NULL
      || hst->notification_period_ptr != NULL)
    throw (engine_error() << "remove timeperiod with hosts failed.");

  delete[] hst->name;
  delete[] hst->display_name;
  delete[] hst->alias;
  delete[] hst->address;
  delete hst;

  host_list = NULL;
  host_list_tail = NULL;
  free_object_skiplists();
}

/**
 *  Check if remove timeperiod works with some services.
 */
static void remove_timeperiod_with_services() {
  init_object_skiplists();

  timeperiod* t = add_timeperiod("timeperiod_name", "timeperiod_alias");
  service* svc = add_service("service_host_name", "service_host_description", NULL,
                             NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL, 0, 0, 0, 0,
                             0, 0, 0, 0, NULL, 0, "check_command", 0, 0, 0.0, 0.0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL,
                             NULL, NULL, 0, 0, 0);
  svc->check_period_ptr = t;
  svc->notification_period_ptr = t;

  if (remove_timeperiod_by_id("timeperiod_name") != 1
      || timeperiod_list != NULL
      || timeperiod_list_tail != NULL
      || svc->check_period_ptr != NULL
      || svc->notification_period_ptr != NULL)
    throw (engine_error() << "remove timeperiod with services failed.");

  delete[] svc->service_check_command;
  delete[] svc->display_name;
  delete[] svc->description;
  delete[] svc->host_name;
  delete svc;

  service_list = NULL;
  service_list_tail = NULL;
  free_object_skiplists();
}

/**
 *  Check if remove timeperiod works with some serviceescalations.
 */
static void remove_timeperiod_with_serviceescalations() {
  init_object_skiplists();

  timeperiod* t = add_timeperiod("timeperiod_name", "timeperiod_alias");
  serviceescalation* se = add_serviceescalation("host_name", "description",
						0, 0, 0.0, NULL, 0, 0, 0, 0);
  se->escalation_period_ptr = t;

  if (remove_timeperiod_by_id("timeperiod_name") != 1
      || timeperiod_list != NULL
      || timeperiod_list_tail != NULL
      || se->escalation_period_ptr != NULL)
    throw (engine_error() << "remove timeperiod with serviceescalations failed.");

  delete[] se->host_name;
  delete[] se->description;
  delete[] se->escalation_period;
  delete se;

  serviceescalation_list = NULL;
  serviceescalation_list_tail = NULL;
  free_object_skiplists();
}

/**
 *  Check if remove timeperiod works with some servicedependencies.
 */
static void remove_timeperiod_with_servicedependencies() {
  init_object_skiplists();

  timeperiod* t = add_timeperiod("timeperiod_name", "timeperiod_alias");
  servicedependency* sd = add_service_dependency("service_dependency_dependent_host_name",
                                                 "service_dependency_dependent_service_description",
                                                 "service_dependency_host_name",
                                                 "service_dependency_service_description",
                                                 0, 0, 0, 0, 0, 0, 0,
                                                 "service_dependency_dependency_period");
  sd->dependency_period_ptr = t;

  if (remove_timeperiod_by_id("timeperiod_name") != 1
      || timeperiod_list != NULL
      || timeperiod_list_tail != NULL
      || sd->dependency_period_ptr != NULL)
    throw (engine_error() << "remove timeperiod with servicedependencies failed.");

  delete[] sd->host_name;
  delete[] sd->service_description;
  delete[] sd->dependent_host_name;
  delete[] sd->dependent_service_description;
  delete[] sd->dependency_period;
  delete sd;

  servicedependency_list = NULL;
  servicedependency_list_tail = NULL;
  free_object_skiplists();
}

/**
 *  Check if remove timeperiod works with some hostescalations.
 */
static void remove_timeperiod_with_hostescalations() {
  init_object_skiplists();

  timeperiod* t = add_timeperiod("timeperiod_name", "timeperiod_alias");
  hostescalation* he = add_hostescalation("host_name", 0, 0, 0.0,
					  "escalation_period", 0, 0, 0);
  he->escalation_period_ptr = t;

  if (remove_timeperiod_by_id("timeperiod_name") != 1
      || timeperiod_list != NULL
      || timeperiod_list_tail != NULL
      || he->escalation_period_ptr != NULL)
    throw (engine_error() << "remove timeperiod with hostescalations failed.");

  delete[] he->host_name;
  delete[] he->escalation_period;
  delete he;

  hostescalation_list = NULL;
  hostescalation_list_tail = NULL;
  free_object_skiplists();
}

/**
 *  Check if remove timeperiod works with some hostdependencies.
 */
static void remove_timeperiod_with_hostdependencies() {
  init_object_skiplists();

  timeperiod* t = add_timeperiod("timeperiod_name", "timeperiod_alias");
  hostdependency* hd = add_host_dependency("dependent_host_name", "host_name",
					   0, 0, 0, 0, 0, 0, NULL);
  hd->dependency_period_ptr = t;

  if (remove_timeperiod_by_id("timeperiod_name") != 1
      || timeperiod_list != NULL
      || timeperiod_list_tail != NULL
      || hd->dependency_period_ptr != NULL)
    throw (engine_error() << "remove timeperiod with hostdependencies failed.");

  delete[] hd->host_name;
  delete[] hd->dependent_host_name;
  delete hd;

  hostdependency_list = NULL;
  hostdependency_list_tail = NULL;
  free_object_skiplists();
}


/**
 *  Check if remove command works.
 */
int main(void) {
  try {
    remove_all_timeperiod();
    remove_timeperiod_failed();
    remove_timeperiod_with_timeranges();
    remove_timeperiod_with_exclusions();
    remove_timeperiod_with_exceptions();
    remove_timeperiod_with_contacts();
    remove_timeperiod_with_hosts();
    remove_timeperiod_with_services();
    remove_timeperiod_with_serviceescalations();
    remove_timeperiod_with_servicedependencies();
    remove_timeperiod_with_hostescalations();
    remove_timeperiod_with_hostdependencies();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    free_memory(get_global_macros());
    return (1);
  }
  return (0);
}
