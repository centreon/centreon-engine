/*
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

#include <cstring>
#include "chkdiff.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/timedevent.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/sched_info.hh"
#include "com/centreon/engine/events/timed_event.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros.hh"
#include "test/unittest.hh"
#include "xodtemplate.hh"

void init_timing_loop();

using namespace com::centreon;
using namespace com::centreon::engine;

struct global {
  sched_info scheduling_info;
  timed_event* events_high;
  timed_event* events_low;

  umap<std::string, std::shared_ptr<command> > save_commands;
  umap<std::string, std::shared_ptr<commands::connector> > save_connectors;
  umap<std::string, std::shared_ptr<contact> > save_contacts;
  umap<std::string, std::shared_ptr<contactgroup> > save_contactgroups;
  umap<std::string, std::shared_ptr<host> > save_hosts;
  umultimap<std::string, std::shared_ptr<hostdependency> >
      save_hostdependencies;
  umultimap<std::string, std::shared_ptr<hostescalation> > save_hostescalations;
  umap<std::string, std::shared_ptr<hostgroup> > save_hostgroups;
  umap<std::pair<std::string, std::string>, std::shared_ptr<service> >
      save_services;
  umultimap<std::pair<std::string, std::string>,
            std::shared_ptr<servicedependency> >
      save_servicedependencies;
  umultimap<std::pair<std::string, std::string>,
            std::shared_ptr<serviceescalation> >
      save_serviceescalations;
  umap<std::string, std::shared_ptr<servicegroup> > save_servicegroups;
  umap<std::string, std::shared_ptr<timeperiod> > save_timeperiods;
};

/**
 *  Clear all global list.
 */
static void clear(global& g) {
  command_list = NULL;
  contact_list = NULL;
  contactgroup_list = NULL;
  host_list = NULL;
  hostdependency_list = NULL;
  hostescalation_list = NULL;
  hostgroup_list = NULL;
  service_list = NULL;
  servicedependency_list = NULL;
  serviceescalation_list = NULL;
  servicegroup_list = NULL;
  timeperiod_list = NULL;

  configuration::applier::state& app_state(
      configuration::applier::state::instance());
  g.save_commands = app_state.commands();
  app_state.commands().clear();
  g.save_connectors = app_state.connectors();
  app_state.connectors().clear();
  g.save_contacts = app_state.contacts();
  app_state.contacts().clear();
  g.save_contactgroups = app_state.contactgroups();
  app_state.contactgroups().clear();
  g.save_hosts = app_state.hosts();
  app_state.hosts().clear();
  g.save_hostdependencies = app_state.hostdependencies();
  app_state.hostdependencies().clear();
  g.save_hostescalations = app_state.hostescalations();
  app_state.hostescalations().clear();
  g.save_hostgroups = app_state.hostgroups();
  app_state.hostgroups().clear();
  g.save_services = app_state.services();
  app_state.services().clear();
  g.save_servicedependencies = app_state.servicedependencies();
  app_state.servicedependencies().clear();
  g.save_serviceescalations = app_state.serviceescalations();
  app_state.serviceescalations().clear();
  g.save_servicegroups = app_state.servicegroups();
  app_state.servicegroups().clear();
  g.save_timeperiods = app_state.timeperiods();
  app_state.timeperiods().clear();
}

static void lise_run_time(timed_event* lst, time_t ref) {
  for (timed_event* evt(lst); evt; evt = evt->next)
    if (evt->run_time != ref)
      evt->run_time -= 1;
}

/**
 *  Check difference between global object.
 *
 *  @param[in] l1 The first struct.
 *  @param[in] l2 The second struct.
 *
 *  @return True if globals are equal, otherwise false.
 */
static bool chkdiff(global& g1, global& g2) {
  bool ret(true);
  if (g1.scheduling_info.first_service_check !=
      g2.scheduling_info.first_service_check)
    g1.scheduling_info.first_service_check += 1;
  if (g1.scheduling_info.last_service_check !=
      g2.scheduling_info.last_service_check)
    g1.scheduling_info.last_service_check += 1;
  if (g1.scheduling_info.first_host_check !=
      g2.scheduling_info.first_host_check)
    g1.scheduling_info.first_host_check += 1;
  if (g1.scheduling_info.last_host_check != g2.scheduling_info.last_host_check)
    g1.scheduling_info.last_host_check += 1;

  if (memcmp(&g1.scheduling_info, &g2.scheduling_info, sizeof(sched_info))) {
    ret = false;
    std::cerr << "difference detected" << std::endl;
    std::cerr << "old " << g1.scheduling_info << std::endl;
    std::cerr << "new " << g2.scheduling_info << std::endl;
  }

  lise_run_time(g1.events_high, g1.events_high->run_time);
  lise_run_time(g2.events_high, g1.events_high->run_time);
  if (!chkdiff(g1.events_high, g2.events_high))
    ret = false;

  lise_run_time(g1.events_low, g1.events_low->run_time);
  lise_run_time(g2.events_low, g1.events_low->run_time);
  if (!chkdiff(g1.events_low, g2.events_low))
    ret = false;
  return (ret);
}

/**
 *  Read configuration with new parser.
 *
 *  @parser[out] g        Fill global variable.
 *  @param[in]  filename  The file path to parse.
 *  @parse[in]  options   The options to use.
 *
 *  @return True on succes, otherwise false.
 */
static bool newparser_read_config(global& g,
                                  std::string const& filename,
                                  unsigned int options) {
  bool ret(false);
  try {
    init_macros();
    configuration::state config;

    // tricks to bypass create log file.
    config.log_file("");

    configuration::parser p(options);
    p.parse(filename, config);

    configuration::applier::state::instance().apply(config);

    g.scheduling_info = ::scheduling_info;
    memset(&::scheduling_info, 0, sizeof(::scheduling_info));
    g.events_high = ::event_list_high;
    ::event_list_high = NULL;
    g.events_low = ::event_list_low;
    ::event_list_low = NULL;

    clear(g);
    clear_volatile_macros_r(get_global_macros());
    free_macrox_names();
    ret = true;
  } catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
  }
  return (ret);
}

/**
 *  Read configuration with old parser.
 *
 *  @parser[out] g        Fill global variable.
 *  @param[in]   filename The file path to parse.
 *  @parse[in]   options  The options to use.
 *
 *  @return True on succes, otherwise false.
 */
static bool oldparser_read_config(global& g,
                                  std::string const& filename,
                                  unsigned int options) {
  clear_volatile_macros_r(get_global_macros());
  free_macrox_names();
  init_object_skiplists();
  init_macros();
  int ret(read_main_config_file(filename.c_str()));
  if (ret == OK)
    ret = xodtemplate_read_config_data(filename.c_str(), options, false, false);
  if (ret == OK) {
    ret = pre_flight_check();

    init_timing_loop();

    g.scheduling_info = ::scheduling_info;
    memset(&::scheduling_info, 0, sizeof(::scheduling_info));
    g.events_high = ::event_list_high;
    ::event_list_high = NULL;
    g.events_low = ::event_list_low;
    ::event_list_low = NULL;
  }
  clear(g);
  clear_volatile_macros_r(get_global_macros());
  free_macrox_names();
  free_object_skiplists();
  return (ret == OK);
}

/**
 *  Check the parsing argument.
 *
 *  @param[in] argc Unused.
 *  @param[in] argv Unused.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  if (argc != 2)
    throw(engine_error() << "usage: " << argv[0] << " file.cfg");

  unsigned int options(configuration::parser::read_all);

  global oldcfg;
  if (!oldparser_read_config(oldcfg, argv[1], options))
    throw(engine_error() << "old parser can't parse " << argv[1]);

  global newcfg;
  if (!newparser_read_config(newcfg, argv[1], options))
    throw(engine_error() << "new parser can't parse " << argv[1]);

  bool ret(chkdiff(oldcfg, newcfg));

  deleter::listmember(oldcfg.events_high, &deleter::timedevent);
  deleter::listmember(oldcfg.events_low, &deleter::timedevent);
  deleter::listmember(newcfg.events_high, &deleter::timedevent);
  deleter::listmember(newcfg.events_low, &deleter::timedevent);
  return (!ret);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
