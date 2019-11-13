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

#include <exception>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/nebstructs.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  callback used by check event user function.
 *
 *  @param[in] data The current timed event.
 */
static void user_function(void* data) {
  reinterpret_cast<timed_event*>(data)->event_args = NULL;
}

/**
 *  callback used by broker.
 *
 *  @param[in] callback_type The callback type send by the broker.
 *  @param[in] data          The nebstruct external command send by the broker.
 *
 *  @return the last callback_type recve by the callback.
 */
static int broker_callback(int callback_type, void* data) {
  static int last_callback_type = -1;
  int ret = last_callback_type;

  nebstruct_external_command_data* neb_data =
      static_cast<nebstruct_external_command_data*>(data);
  if (callback_type != -1)
    last_callback_type = neb_data->type;
  else
    last_callback_type = -1;
  return (ret);
}

/**
 *  Check the event service check.
 */
static void check_event_service_check() {
  // create fake service.
  service svc;
  memset(&svc, 0, sizeof(svc));
  svc.should_be_scheduled = true;
  svc.host_name = const_cast<char*>("name");
  svc.description = const_cast<char*>("description");

  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_SERVICE_CHECK;
  event.event_data = static_cast<void*>(&svc);

  handle_timed_event(&event);

  // check if handle_timed_event call _exec_event_service_check.
  if (svc.next_check == 0)
    throw(engine_error() << __func__);
}

/**
 *  Check the event command check.
 */
static void check_event_command_check() {
  // register broker callback to catch event.
  config->event_broker_options(BROKER_EXTERNALCOMMAND_DATA);
  void* module_id = reinterpret_cast<void*>(0x4242);
  neb_register_callback(NEBCALLBACK_EXTERNAL_COMMAND_DATA, module_id, 0,
                        &broker_callback);

  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_COMMAND_CHECK;

  handle_timed_event(&event);

  if (broker_callback(-1, NULL) != NEBTYPE_EXTERNALCOMMAND_CHECK)
    throw(engine_error() << __func__);

  // release callback.
  neb_deregister_module_callbacks(module_id);
}

/**
 *  Check the event program shutdown.
 */
static void check_event_program_shutdown() {
  // set sigshutdown as known value.
  sigshutdown = false;

  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_PROGRAM_SHUTDOWN;

  handle_timed_event(&event);

  // check if handle_timed_event call _exec_event_program_shutdown.
  if (sigshutdown == false)
    throw(engine_error() << __func__);
}

/**
 *  Check the event program restart.
 */
static void check_event_program_restart() {
  // set sigrestart as known value.
  sigrestart = false;

  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_PROGRAM_RESTART;

  handle_timed_event(&event);

  // check if handle_timed_event call _exec_event_program_restart.
  if (sigrestart != false)
    throw(engine_error() << __func__);
}

/**
 *  Check the event check reaper.
 */
static void check_event_check_reaper() {
  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_CHECK_REAPER;

  handle_timed_event(&event);
}

/**
 *  Check the event orphan check.
 */
static void check_event_orphan_check() {
  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_ORPHAN_CHECK;

  handle_timed_event(&event);
}

/**
 *  Check the event retention save.
 */
static void check_event_retention_save() {
  // register broker callback to catch event.
  config->event_broker_options(BROKER_RETENTION_DATA);
  config->retain_state_information(true);
  void* module_id = reinterpret_cast<void*>(0x4242);
  neb_register_callback(NEBCALLBACK_RETENTION_DATA, module_id, 0,
                        &broker_callback);

  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_RETENTION_SAVE;

  handle_timed_event(&event);

  if (broker_callback(-1, NULL) != NEBTYPE_RETENTIONDATA_ENDSAVE)
    throw(engine_error() << __func__);

  // release callback.
  neb_deregister_module_callbacks(module_id);
}

/**
 *  Check the event status save.
 */
static void check_event_status_save() {
  // register broker callback to catch event.
  config->event_broker_options(BROKER_STATUS_DATA);
  config->retain_state_information(true);
  void* module_id = reinterpret_cast<void*>(0x4242);
  neb_register_callback(NEBCALLBACK_AGGREGATED_STATUS_DATA, module_id, 0,
                        &broker_callback);

  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_STATUS_SAVE;

  handle_timed_event(&event);

  if (broker_callback(-1, NULL) != NEBTYPE_AGGREGATEDSTATUS_ENDDUMP)
    throw(engine_error() << __func__);

  // release callback.
  neb_deregister_module_callbacks(module_id);
}

/**
 *  Check the event scheduled downtime.
 */
static void check_event_scheduled_downtime() {
  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_SCHEDULED_DOWNTIME;
  event.event_data = static_cast<void*>(new unsigned long(42));

  handle_timed_event(&event);

  // check if handle_timed_event call _exec_event_scheduled_downtime.
  if (event.event_data != NULL)
    throw(engine_error() << __func__);
}

/**
 *  Check the event sfreshness check.
 */
static void check_event_sfreshness_check() {
  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_SFRESHNESS_CHECK;

  handle_timed_event(&event);
}

/**
 *  Check the event expire downtime.
 */
static void check_event_expire_downtime() {
  // create fake comment.
  unsigned long downtime_id(42);
  if (add_host_downtime("name", 0, const_cast<char*>("author"),
                        const_cast<char*>("comment"), 0, 0, 0, 0, 0,
                        downtime_id) != OK ||
      scheduled_downtime_list == NULL)
    throw(engine_error() << "add_new comment failed.");

  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_EXPIRE_DOWNTIME;

  handle_timed_event(&event);

  // check if handle_timed_event call _exec_event_expire_downtime.
  if (scheduled_downtime_list != NULL)
    throw(engine_error() << __func__);
}

/**
 *  Check the event host check.
 */
static void check_event_host_check() {
  host hst;
  memset(&hst, 0, sizeof(hst));
  hst.name = const_cast<char*>("name");
  hst.should_be_scheduled = true;

  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_HOST_CHECK;
  event.event_data = static_cast<void*>(&hst);

  handle_timed_event(&event);

  // check if handle_timed_event call _exec_event_host_check.
  if (hst.next_check == 0)
    throw(engine_error() << __func__);
}

/**
 *  Check the event hfreshness check.
 */
static void check_event_hfreshness_check() {
  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_HFRESHNESS_CHECK;

  handle_timed_event(&event);
}

/**
 *  Check the event reschedule checks.
 */
static void check_event_reschedule_checks() {
  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_RESCHEDULE_CHECKS;

  handle_timed_event(&event);
}

/**
 *  Check the event expire comment.
 */
static void check_event_expire_comment() {
  // create fake comment.
  unsigned long comment_id(42);
  if (add_comment(HOST_COMMENT, 0, "name", NULL, (time_t)0, "author",
                  const_cast<char*>("comment"), comment_id, 0, true, 0,
                  0) != OK ||
      comment_list == NULL)
    throw(engine_error() << "add_new comment failed.");

  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_EXPIRE_COMMENT;
  event.event_data = reinterpret_cast<void*>(comment_id);

  handle_timed_event(&event);

  // check if handle_timed_event call _exec_event_expire_comment.
  if (comment_list != NULL)
    throw(engine_error() << __func__);
}

/**
 *  Check the event user function.
 */
static void check_event_user_function() {
  union {
    void (*func)(void*);
    void* data;
  } user;
  user.func = user_function;

  // create fake event.
  timed_event event;
  memset(&event, 0, sizeof(event));
  event.event_type = EVENT_USER_FUNCTION;
  event.event_data = user.data;
  event.event_args = static_cast<void*>(&event);

  handle_timed_event(&event);

  if (event.event_args == &event)
    throw(engine_error() << __func__);
}

/**
 *  Check the handle timed event working.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  check_event_service_check();
  check_event_command_check();
  check_event_program_shutdown();
  check_event_program_restart();
  check_event_check_reaper();
  check_event_orphan_check();
  check_event_retention_save();
  check_event_status_save();
  check_event_scheduled_downtime();
  check_event_sfreshness_check();
  check_event_expire_downtime();
  check_event_host_check();
  check_event_hfreshness_check();
  check_event_reschedule_checks();
  check_event_expire_comment();
  check_event_user_function();

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
