/*
** Copyright 2011 Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <sstream>
#include <string>
#include <string.h>
#include <strings.h>
#include "centreonscheduler.nsmap" // gSOAP namespaces.
#include "soapH.h"
#include "objects.hh"
#include "downtime.hh"
#include "logging.hh"
#include "sretention.hh"
#include "nagios.hh"
#include "broker.hh"
#include "statusdata.hh"

extern int           sigrestart;
extern int           sigshutdown;
extern int           enable_event_handlers;
extern int           enable_failure_prediction;
extern int           enable_flap_detection;
extern int           execute_host_checks;
extern int           accept_passive_host_checks;
extern char*         global_host_event_handler;
extern int           check_host_freshness;
extern int           obsess_over_hosts;
extern int           enable_notifications;
extern int           process_performance_data;
extern int           execute_service_checks;
extern int           accept_passive_service_checks;
extern char*         global_service_event_handler;
extern int           check_service_freshness;
extern int           obsess_over_services;
extern int           enable_event_handlers;
extern char*         global_host_event_handler;
extern char*         global_service_event_handler;
extern command*      global_host_event_handler_ptr;
extern command*      global_service_event_handler_ptr;
extern unsigned long modified_host_process_attributes;
extern unsigned long modified_service_process_attributes;

static inline char* my_strdup(char const* str) {
  char* new_str = new char[strlen(str) + 1];
  return (strcpy(new_str, str));
}

/**
 *  Restart Scheduler.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__processRestart(soap* s,
				      centreonscheduler__processRestartResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);
  sigrestart = true;
  res.error->code = 0;
  logit(NSLOG_PROCESS_INFO, true, "Webservice: program restarting...\n");

  return (SOAP_OK);
}

/**
 *  Shutdown Scheduler.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__processShutdown(soap* s,
				       centreonscheduler__processShutdownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);
  sigshutdown = true;
  res.error->code = 0;
  logit(NSLOG_PROCESS_INFO, true, "Webservice: program shutting down...\n");

  return (SOAP_OK);
}

/**
 *  Read state information.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__stateInformationLoad(soap* s,
					    centreonscheduler__stateInformationLoadResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);
  read_initial_state_information();

  res.error->code = 0;
  return (SOAP_OK);
}

/**
 *  Save state information.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__stateInformationSave(soap* s,
					    centreonscheduler__stateInformationSaveResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);
  save_state_information(false);
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get whether or not the host is acknowledged.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetAcknowledgementIsSet(soap* s,
						   ns1__hostIDType* host_id,
						   centreonscheduler__hostGetAcknowledgementIsSetResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = (host->acknowledgement_type != ACKNOWLEDGEMENT_NONE);
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the current type of the acknowledgement on a host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetAcknowledgementType(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetAcknowledgementTypeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->acknowledgement_type;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the address of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetAddress(soap* s,
				      ns1__hostIDType* host_id,
				      centreonscheduler__hostGetAddressResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (host->address != NULL) {
    res.value = host->address;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the address of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  address Host's address.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetAddress(soap* s,
				      ns1__hostIDType* host_id,
				      std::string address,
				      centreonscheduler__hostSetAddressResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 host_id->name.c_str(),
		 address.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (address == "") {
    res.error->message = "Host `" + host_id->name + "' address is empty.";
  }

  delete[] host->address;
  host->address = my_strdup(address.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if active checks are enabled on the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckActiveEnabled(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__hostGetCheckActiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->checks_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host check command.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckCommand(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__hostGetCheckCommandResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (host->host_check_command != NULL) {
    res.value = host->host_check_command;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the current check attempt of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckCurrentAttempt(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetCheckCurrentAttemptResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->current_attempt;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the normal check interval.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckIntervalNormal(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetCheckIntervalNormalResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->check_interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the retry check interval.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckIntervalRetry(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__hostGetCheckIntervalRetryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->retry_interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the date when the last check was executed.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckLast(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__hostGetCheckLastResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->last_check;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the max check attempts of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckMaxAttempts(soap* s,
					       ns1__hostIDType* host_id,
					       centreonscheduler__hostGetCheckMaxAttemptsResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->max_attempts;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the time at which the next host_id check is scheduled to run.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckNext(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__hostGetCheckNextResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->next_check;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host check options.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckOptions(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__hostGetCheckOptionsResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->check_options;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if passive checks are enabled on this host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckPassiveEnabled(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetCheckPassiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->accept_passive_host_checks;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host check period.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckPeriod(soap* s,
					  ns1__hostIDType* host_id,
					  centreonscheduler__hostGetCheckPeriodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (host->check_period != NULL) {
    res.value->timeperiod = host->check_period;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if the host should be scheduled.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckShouldBeScheduled(soap* s,
						     ns1__hostIDType* host_id,
						     centreonscheduler__hostGetCheckShouldBeScheduledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->should_be_scheduled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the type of the host check.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCheckType(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__hostGetCheckTypeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->check_type;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable active checks on the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetCheckActiveEnabled(soap* s,
						 ns1__hostIDType* host_id,
						 bool enable,
						 centreonscheduler__hostSetCheckActiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->checks_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the host check command.
 *
 *  @param[in]  s        Unused.
 *  @param[in]  host_id  Host to set data.
 *  @param[in]  command  New check command.
 *  @param[out] res      Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetCheckCommand(soap* s,
					   ns1__hostIDType* host_id,
					   std::string command,
					   centreonscheduler__hostSetCheckCommandResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 host_id->name.c_str(),
		 command.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (command == "") {
    res.error->message = "Host `" + host_id->name + "' command is empty.";
  }

  delete[] host->host_check_command;
  host->host_check_command = my_strdup(command.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the normal check interval of the host.
 *
 *  @param[in]  s         Unused.
 *  @param[in]  host_id   Host to set data.
 *  @param[in]  interval  Check interval time.
 *  @param[out] res       Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetCheckIntervalNormal(soap* s,
						  ns1__hostIDType* host_id,
						  unsigned int interval,
						  centreonscheduler__hostSetCheckIntervalNormalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %u)\n",
		 __func__,
		 host_id->name.c_str(),
		 interval);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->check_interval = interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the retry check interval of the host.
 *
 *  @param[in]  s         Unused.
 *  @param[in]  host_id   Host to set data.
 *  @param[in]  interval  Check interval time.
 *  @param[out] res       Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetCheckIntervalRetry(soap* s,
						 ns1__hostIDType* host_id,
						 unsigned int interval,
						 centreonscheduler__hostSetCheckIntervalRetryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %u)\n",
		 __func__,
		 host_id->name.c_str(),
		 interval);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->retry_interval = interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the max check attempts of the host.
 *
 *  @param[in]  s         Unused.
 *  @param[in]  host_id   Host to set data.
 *  @param[in]  attempts  Max attempts.
 *  @param[out] res       Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetCheckMaxAttempts(soap* s,
					       ns1__hostIDType* host_id,
					       unsigned int attempts,
					       centreonscheduler__hostSetCheckMaxAttemptsResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %u)\n",
		 __func__,
		 host_id->name.c_str(),
		 attempts);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (attempts == 0) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' bad attempts value.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->max_attempts = attempts;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable host_id passive checks.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetCheckPassiveEnabled(soap* s,
						  ns1__hostIDType* host_id,
						  bool enable,
						  centreonscheduler__hostSetCheckPassiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->accept_passive_host_checks = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if the host has been checked for circular path.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCircularPathChecked(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetCircularPathCheckedResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->circular_path_checked;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if the host has circular path.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetCircularPathHas(soap* s,
					      ns1__hostIDType* host_id,
					      centreonscheduler__hostGetCircularPathHasResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->contains_circular_path;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host downtime depth.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetDowntimeDepth(soap* s,
					    ns1__hostIDType* host_id,
					    centreonscheduler__hostGetDowntimeDepthResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->scheduled_downtime_depth;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if host has a pending flexible downtime.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetDowntimeFlexPending(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetDowntimeFlexPendingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->pending_flex_downtime;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host event handler.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetEventHandler(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__hostGetEventHandlerResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (host->event_handler != NULL) {
    res.value = host->event_handler;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if the host event handler is enabled.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetEventHandlerEnabled(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetEventHandlerEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->event_handler_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the host event handler.
 *
 *  @param[in]  s             Unused.
 *  @param[in]  host_id       Host to set data.
 *  @param[in]  event_handler The event handler.
 *  @param[out] res           Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetEventHandler(soap* s,
					   ns1__hostIDType* host_id,
					   std::string event_handler,
					   centreonscheduler__hostSetEventHandlerResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 host_id->name.c_str(),
		 event_handler.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (event_handler == "") {
    res.error->message = "Host `" + host_id->name + "' event_handler is empty.";
  }

  delete[] host->event_handler;
  host->event_handler = my_strdup(event_handler.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable host_id event handler.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetEventHandlerEnabled(soap* s,
						  ns1__hostIDType* host_id,
						  bool enable,
						  centreonscheduler__hostSetEventHandlerEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->event_handler_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if failure prediction is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFailurePredictionEnabled(soap* s,
						       ns1__hostIDType* host_id,
						       centreonscheduler__hostGetFailurePredictionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->failure_prediction_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get host_id failure prediction options.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFailurePredictionOptions(soap* s,
						       ns1__hostIDType* host_id,
						       centreonscheduler__hostGetFailurePredictionOptionsResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (host->failure_prediction_options != NULL) {
    res.value = host->failure_prediction_options;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable failure prediction on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetFailurePredictionEnabled(soap* s,
						       ns1__hostIDType* host_id,
						       bool enable,
						       centreonscheduler__hostSetFailurePredictionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->failure_prediction_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the flap detection comment ID of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFlapDetectionCommentID(soap* s,
						     ns1__hostIDType* host_id,
						     centreonscheduler__hostGetFlapDetectionCommentIDResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->flapping_comment_id;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check whether flap detection is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFlapDetectionEnabled(soap* s,
						   ns1__hostIDType* host_id,
						   centreonscheduler__hostGetFlapDetectionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->flap_detection_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if host is flapping.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFlapDetectionIsFlapping(soap* s,
						      ns1__hostIDType* host_id,
						      centreonscheduler__hostGetFlapDetectionIsFlappingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->is_flapping;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on down state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFlapDetectionOnDown(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetFlapDetectionOnDownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->flap_detection_on_down;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on unreachable state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFlapDetectionOnUnreachable(soap* s,
							 ns1__hostIDType* host_id,
							 centreonscheduler__hostGetFlapDetectionOnUnreachableResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->flap_detection_on_unreachable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on up state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFlapDetectionOnUp(soap* s,
						ns1__hostIDType* host_id,
						centreonscheduler__hostGetFlapDetectionOnUpResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->flap_detection_on_up;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the flap detection state history was updated.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFlapDetectionStateHistoryLastUpdate(soap* s,
								  ns1__hostIDType* host_id,
								  centreonscheduler__hostGetFlapDetectionStateHistoryLastUpdateResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->last_state_history_update;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host high flap threshold.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFlapDetectionThresholdHigh(soap* s,
							 ns1__hostIDType* host_id,
							 centreonscheduler__hostGetFlapDetectionThresholdHighResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->high_flap_threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host low flap threshold.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFlapDetectionThresholdLow(soap* s,
							ns1__hostIDType* host_id,
							centreonscheduler__hostGetFlapDetectionThresholdLowResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->low_flap_threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetFlapDetectionEnabled(soap* s,
						   ns1__hostIDType* host_id,
						   bool enable,
						   centreonscheduler__hostSetFlapDetectionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->flap_detection_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on down state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetFlapDetectionOnDown(soap* s,
						  ns1__hostIDType* host_id,
						  bool enable,
						  centreonscheduler__hostSetFlapDetectionOnDownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->flap_detection_on_down = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on unreachable state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetFlapDetectionOnUnreachable(soap* s,
							 ns1__hostIDType* host_id,
							 bool enable,
							 centreonscheduler__hostSetFlapDetectionOnUnreachableResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->flap_detection_on_unreachable = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on up state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetFlapDetectionOnUp(soap* s,
						ns1__hostIDType* host_id,
						bool enable,
						centreonscheduler__hostSetFlapDetectionOnUpResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->flap_detection_on_up = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the high flap threshold of the host.
 *
 *  @param[in]  s          Unused.
 *  @param[in]  host_id    Host to set data.
 *  @param[in]  threshold  New threshold.
 *  @param[out] res        Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetFlapDetectionThresholdHigh(soap* s,
							 ns1__hostIDType* host_id,
							 double threshold,
							 centreonscheduler__hostSetFlapDetectionThresholdHighResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %f)\n",
		 __func__,
		 host_id->name.c_str(),
		 threshold);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->high_flap_threshold = threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the low flap threshold of the host.
 *
 *  @param[in]  s          Unused.
 *  @param[in]  host_id    Host to set data.
 *  @param[in]  threshold  New threshold.
 *  @param[out] res        Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetFlapDetectionThresholdLow(soap* s,
							ns1__hostIDType* host_id,
							double threshold,
							centreonscheduler__hostSetFlapDetectionThresholdLowResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %f)\n",
		 __func__,
		 host_id->name.c_str(),
		 threshold);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->low_flap_threshold = threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if freshness checks are enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFreshnessCheckEnabled(soap* s,
						    ns1__hostIDType* host_id,
						    centreonscheduler__hostGetFreshnessCheckEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->check_freshness;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if freshness check is active on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFreshnessIsActive(soap* s,
						ns1__hostIDType* host_id,
						centreonscheduler__hostGetFreshnessIsActiveResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->is_being_freshened;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the freshness threshold of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetFreshnessThreshold(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__hostGetFreshnessThresholdResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->freshness_threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable freshness checks on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetFreshnessCheckEnabled(soap* s,
						    ns1__hostIDType* host_id,
						    bool enable,
						    centreonscheduler__hostSetFreshnessCheckEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->check_freshness = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the host freshness threshold.
 *
 *  @param[in]  s          Unused.
 *  @param[in]  host_id    Host to set data.
 *  @param[in]  threshold  New threshold.
 *  @param[out] res        Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetFreshnessThreshold(soap* s,
						 ns1__hostIDType* host_id,
						 int threshold,
						 centreonscheduler__hostSetFreshnessThresholdResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 threshold);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->freshness_threshold = threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the modified attributes on the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetModifiedAttributes(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__hostGetModifiedAttributesResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->modified_attributes;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host alias.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNameAlias(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__hostGetNameAliasResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (host->alias != NULL) {
    res.value = host->alias;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host display name.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNameDisplay(soap* s,
					  ns1__hostIDType* host_id,
					  centreonscheduler__hostGetNameDisplayResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (host->display_name != NULL) {
    res.value = host->display_name;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the host alias.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  alias   Host's alias.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNameAlias(soap* s,
					ns1__hostIDType* host_id,
					std::string alias,
					centreonscheduler__hostSetNameAliasResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 host_id->name.c_str(),
		 alias.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (alias == "") {
    res.error->message = "Host `" + host_id->name + "' alias is empty.";
  }

  delete[] host->alias;
  host->alias = my_strdup(alias.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the display name of the host.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  host_id      Host to set data.
 *  @param[in]  displayname  Host's display name.
 *  @param[out] res          Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNameDisplay(soap* s,
					  ns1__hostIDType* host_id,
					  std::string displayname,
					  centreonscheduler__hostSetNameDisplayResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 host_id->name.c_str(),
		 displayname.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (displayname == "") {
    res.error->message = "Host `" + host_id->name + "' displayname is empty.";
  }

  delete[] host->display_name;
  host->display_name = my_strdup(displayname.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the ID of the current host_id notification.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsCurrentID(soap* s,
						     ns1__hostIDType* host_id,
						     centreonscheduler__hostGetNotificationsCurrentIDResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->current_notification_id;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the current notification number of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsCurrentNumber(soap* s,
							 ns1__hostIDType* host_id,
							 centreonscheduler__hostGetNotificationsCurrentNumberResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->current_notification_number;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsEnabled(soap* s,
						   ns1__hostIDType* host_id,
						   centreonscheduler__hostGetNotificationsEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->notifications_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the first notification delay of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsFirstDelay(soap* s,
						      ns1__hostIDType* host_id,
						      centreonscheduler__hostGetNotificationsFirstDelayResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->first_notification_delay;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the notification interval of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsInterval(soap* s,
						    ns1__hostIDType* host_id,
						    centreonscheduler__hostGetNotificationsIntervalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->notification_interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the time at which the last notification was sent.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsLast(soap* s,
						ns1__hostIDType* host_id,
						centreonscheduler__hostGetNotificationsLastResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->last_host_notification;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the time at which the next notification will be sent.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsNext(soap* s,
						ns1__hostIDType* host_id,
						centreonscheduler__hostGetNotificationsNextResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->next_host_notification;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is down.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsOnDown(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetNotificationsOnDownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->notify_on_down;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is on downtime.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsOnDowntime(soap* s,
						      ns1__hostIDType* host_id,
						      centreonscheduler__hostGetNotificationsOnDowntimeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->notify_on_downtime;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is flappy.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsOnFlapping(soap* s,
						      ns1__hostIDType* host_id,
						      centreonscheduler__hostGetNotificationsOnFlappingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->notify_on_flapping;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host recovers.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsOnRecovery(soap* s,
						      ns1__hostIDType* host_id,
						      centreonscheduler__hostGetNotificationsOnRecoveryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->notify_on_recovery;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is unreachable.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsOnUnreachable(soap* s,
							 ns1__hostIDType* host_id,
							 centreonscheduler__hostGetNotificationsOnUnreachableResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->notify_on_unreachable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the notification period of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetNotificationsPeriod(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetNotificationsPeriodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (host->notification_period != NULL) {
    res.value->timeperiod = host->notification_period;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNotificationsEnabled(soap* s,
						   ns1__hostIDType* host_id,
						   bool enable,
						   centreonscheduler__hostSetNotificationsEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->notifications_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the time after which the first host_id notification will be sent.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  delay   Delay of the first notification.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNotificationsFirstDelay(soap* s,
						      ns1__hostIDType* host_id,
						      unsigned int delay,
						      centreonscheduler__hostSetNotificationsFirstDelayResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %u)\n",
		 __func__,
		 host_id->name.c_str(),
		 delay);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->first_notification_delay = delay;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the notification interval of the host.
 *
 *  @param[in]  s         Unused.
 *  @param[in]  host_id   Host to set data.
 *  @param[in]  interval  Notification interval.
 *  @param[out] res       Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNotificationsInterval(soap* s,
						    ns1__hostIDType* host_id,
						    unsigned int interval,
						    centreonscheduler__hostSetNotificationsIntervalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %u)\n",
		 __func__,
		 host_id->name.c_str(),
		 interval);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->notification_interval = interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id is down.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNotificationsOnDown(soap* s,
						  ns1__hostIDType* host_id,
						  bool enable,
						  centreonscheduler__hostSetNotificationsOnDownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->notify_on_down = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id is in downtime.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNotificationsOnDowntime(soap* s,
						      ns1__hostIDType* host_id,
						      bool enable,
						      centreonscheduler__hostSetNotificationsOnDowntimeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->notify_on_downtime = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id is flappy.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNotificationsOnFlapping(soap* s,
						      ns1__hostIDType* host_id,
						      bool enable,
						      centreonscheduler__hostSetNotificationsOnFlappingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->notify_on_flapping = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id recovers.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNotificationsOnRecovery(soap* s,
						      ns1__hostIDType* host_id,
						      bool enable,
						      centreonscheduler__hostSetNotificationsOnRecoveryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->notify_on_recovery = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id is unreachable.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNotificationsOnUnreachable(soap* s,
							 ns1__hostIDType* host_id,
							 bool enable,
							 centreonscheduler__hostSetNotificationsOnUnreachableResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->notify_on_unreachable = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  >Check whether or not host_id is being obsessed over.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetObsessOver(soap* s,
					 ns1__hostIDType* host_id,
					 centreonscheduler__hostGetObsessOverResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->obsess_over_host;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable host_id obsession.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetObsessOver(soap* s,
					 ns1__hostIDType* host_id,
					 bool enable,
					 centreonscheduler__hostSetObsessOverResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->obsess_over_host = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if perfdata processing is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetPerfdataProcessingEnabled(soap* s,
							ns1__hostIDType* host_id,
							centreonscheduler__hostGetPerfdataProcessingEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->process_performance_data;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable perfdata processing.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetPerfdataProcessingEnabled(soap* s,
							ns1__hostIDType* host_id,
							bool enable,
							centreonscheduler__hostSetPerfdataProcessingEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->process_performance_data = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last execution time of the plugin.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetPluginExecutionTime(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetPluginExecutionTimeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->execution_time;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if host check if currently executing.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetPluginIsExecuting(soap* s,
						ns1__hostIDType* host_id,
						centreonscheduler__hostGetPluginIsExecutingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->is_executing;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host latency.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetPluginLatency(soap* s,
					    ns1__hostIDType* host_id,
					    centreonscheduler__hostGetPluginLatencyResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->latency;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the plugin output.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetPluginOutput(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__hostGetPluginOutputResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (host->plugin_output != NULL) {
    res.value = host->plugin_output;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the plugin perfdata.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetPluginPerfdata(soap* s,
					     ns1__hostIDType* host_id,
					     centreonscheduler__hostGetPluginPerfdataResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (host->perf_data != NULL) {
    res.value = host->perf_data;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if host status information are retained.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetRetainStatusInformation(soap* s,
						      ns1__hostIDType* host_id,
						      centreonscheduler__hostGetRetainStatusInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->retain_status_information;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if host non status information are retained.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetRetainNonStatusInformation(soap* s,
							 ns1__hostIDType* host_id,
							 centreonscheduler__hostGetRetainNonStatusInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->retain_nonstatus_information;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable status information retention on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetRetainStatusInformation(soap* s,
						      ns1__hostIDType* host_id,
						      bool enable,
						      centreonscheduler__hostSetRetainStatusInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->retain_status_information = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable non status information retention on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetRetainNonStatusInformation(soap* s,
							 ns1__hostIDType* host_id,
							 bool enable,
							 centreonscheduler__hostSetRetainNonStatusInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->retain_nonstatus_information = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the number of services on this host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetServicesCount(soap* s,
					    ns1__hostIDType* host_id,
					    centreonscheduler__hostGetServicesCountResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->total_services;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the total check interval on this host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetServicesTotalCheckInterval(soap* s,
							 ns1__hostIDType* host_id,
							 centreonscheduler__hostGetServicesTotalCheckIntervalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->total_service_check_interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if stalking on down is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStalkOnDown(soap* s,
					  ns1__hostIDType* host_id,
					  centreonscheduler__hostGetStalkOnDownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->stalk_on_down;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if stalking on unreachable is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStalkOnUnreachable(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__hostGetStalkOnUnreachableResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->stalk_on_unreachable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if stalking on up is enabled on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStalkOnUp(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__hostGetStalkOnUpResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->stalk_on_up;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on down.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetStalkOnDown(soap* s,
					  ns1__hostIDType* host_id,
					  bool enable,
					  centreonscheduler__hostSetStalkOnDownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->stalk_on_down = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on unreachable.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetStalkOnUnreachable(soap* s,
						 ns1__hostIDType* host_id,
						 bool enable,
						 centreonscheduler__hostSetStalkOnUnreachableResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->stalk_on_unreachable = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on up.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetStalkOnUp(soap* s,
					ns1__hostIDType* host_id,
					bool enable,
					centreonscheduler__hostSetStalkOnUpResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->stalk_on_up = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the current state of the state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStateCurrent(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__hostGetStateCurrentResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->current_state;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the initial state of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStateInitial(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__hostGetStateInitialResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->initial_state;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last host_id state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStateLast(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__hostGetStateLastResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->last_state;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the state changed.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStateLastChange(soap* s,
					      ns1__hostIDType* host_id,
					      centreonscheduler__hostGetStateLastChangeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->last_state_change;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the host was in a down state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStateLastDown(soap* s,
					    ns1__hostIDType* host_id,
					    centreonscheduler__hostGetStateLastDownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->last_time_down;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host last hard state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStateLastHard(soap* s,
					    ns1__hostIDType* host_id,
					    centreonscheduler__hostGetStateLastHardResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->last_hard_state;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time at which the hard state changed.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStateLastHardChange(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__hostGetStateLastHardChangeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->last_hard_state_change;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the host was in an unreachable state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStateLastUnreachable(soap* s,
						   ns1__hostIDType* host_id,
						   centreonscheduler__hostGetStateLastUnreachableResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->last_time_unreachable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the host was in an up state.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStateLastUp(soap* s,
					  ns1__hostIDType* host_id,
					  centreonscheduler__hostGetStateLastUpResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->last_time_up;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the percent state change of the host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStatePercentChange(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__hostGetStatePercentChangeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->percent_state_change;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get state type (hard or soft).
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostGetStateType(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__hostGetStateTypeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = host->state_type;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable all notifications beyond a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to get data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNotificationsBeyondEnabled(soap* s,
							 ns1__hostIDType* host_id,
							 bool enable,
							 centreonscheduler__hostSetNotificationsBeyondEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (enable == true) {
    enable_and_propagate_notifications(host, 0, false, true, true);
  }
  else {
    disable_and_propagate_notifications(host, 0, false, true, true);
  }

  res.error->code = 0;
  return (SOAP_OK);
}

/**
 *  Enable notifications of a host and its children.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNotificationsOnSelfAndChildrenEnabled(soap* s,
								    ns1__hostIDType* host_id,
								    bool enable,
								    centreonscheduler__hostSetNotificationsOnSelfAndChildrenEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (enable == true) {
    enable_and_propagate_notifications(host, 0, true, true, false);
  }
  else {
    disable_and_propagate_notifications(host, 0, true, true, false);
  }

  res.error->code = 0;
  return (SOAP_OK);
}

/**
 *  Set notification period of host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  timeperiod_id     Period information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetNotificationsPeriod(soap* s,
						  ns1__hostIDType* host_id,
						  ns1__timeperiodIDType* timeperiod_id,
						  centreonscheduler__hostSetNotificationsPeriodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 host_id->name.c_str(),
		 timeperiod_id->timeperiod.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] host->notification_period;
  host->notification_period = my_strdup(timeperiod_id->timeperiod.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get whether or not the service is acknowledged.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetAcknowledgementIsSet(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonscheduler__serviceGetAcknowledgementIsSetResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = (service->acknowledgement_type != ACKNOWLEDGEMENT_NONE);
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the current type of the acknowledgement on a service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetAcknowledgementType(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonscheduler__serviceGetAcknowledgementTypeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->acknowledgement_type;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if active checks are enabled on the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckActiveEnabled(soap* s,
						    ns1__serviceIDType* service_id,
						    centreonscheduler__serviceGetCheckActiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->checks_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the service check command.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckCommand(soap* s,
					      ns1__serviceIDType* service_id,
					      centreonscheduler__serviceGetCheckCommandResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (service->service_check_command != NULL) {
    res.value = service->service_check_command;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the current check attempt of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckCurrentAttempt(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonscheduler__serviceGetCheckCurrentAttemptResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->current_attempt;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the normal check interval.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckIntervalNormal(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonscheduler__serviceGetCheckIntervalNormalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->check_interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the retry check interval.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckIntervalRetry(soap* s,
						    ns1__serviceIDType* service_id,
						    centreonscheduler__serviceGetCheckIntervalRetryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->retry_interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the date when the last check was executed.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckLast(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonscheduler__serviceGetCheckLastResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->last_check;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the max check attempts of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckMaxAttempts(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonscheduler__serviceGetCheckMaxAttemptsResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->max_attempts;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the time at which the next service check is scheduled to run.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckNext(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonscheduler__serviceGetCheckNextResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->next_check;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the service check options.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckOptions(soap* s,
					      ns1__serviceIDType* service_id,
					      centreonscheduler__serviceGetCheckOptionsResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->check_options;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if passive checks are enabled on this service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckPassiveEnabled(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonscheduler__serviceGetCheckPassiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->accept_passive_service_checks;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the service check period.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckPeriod(soap* s,
					     ns1__serviceIDType* service_id,
					     centreonscheduler__serviceGetCheckPeriodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (service->check_period != NULL) {
    res.value->timeperiod = service->check_period;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if the service should be scheduled.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckShouldBeScheduled(soap* s,
							ns1__serviceIDType* service_id,
							centreonscheduler__serviceGetCheckShouldBeScheduledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->should_be_scheduled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the type of the service check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCheckType(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonscheduler__serviceGetCheckTypeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->check_type;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable active checks on the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetCheckActiveEnabled(soap* s,
						    ns1__serviceIDType* service_id,
						    bool enable,
						    centreonscheduler__serviceSetCheckActiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->checks_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the service check command.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  command           New check command.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetCheckCommand(soap* s,
					      ns1__serviceIDType* service_id,
					      std::string command,
					      centreonscheduler__serviceSetCheckCommandResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %s)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 command.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (command == "") {
    res.error->message = "Service `" + service_id->service + "' command is empty.";
  }

  delete[] service->service_check_command;
  service->service_check_command = my_strdup(command.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the normal check interval of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  interval          Check interval time.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetCheckIntervalNormal(soap* s,
						     ns1__serviceIDType* service_id,
						     unsigned int interval,
						     centreonscheduler__serviceSetCheckIntervalNormalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %u)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 interval);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->check_interval = interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the retry check interval of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  interval          Check interval time.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetCheckIntervalRetry(soap* s,
						    ns1__serviceIDType* service_id,
						    unsigned int interval,
						    centreonscheduler__serviceSetCheckIntervalRetryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %u)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 interval);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->retry_interval = interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the max check attempts of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  attempts          Max attempts.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetCheckMaxAttempts(soap* s,
						  ns1__serviceIDType* service_id,
						  unsigned int attempts,
						  centreonscheduler__serviceSetCheckMaxAttemptsResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %u)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 attempts);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (attempts == 0) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service + "' bad attempts value.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->max_attempts = attempts;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable host passive checks.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetCheckPassiveEnabled(soap* s,
						     ns1__serviceIDType* service_id,
						     bool enable,
						     centreonscheduler__serviceSetCheckPassiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->accept_passive_service_checks = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the value of a service custom variable.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[in]  variable          Custom variable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetCustomVariable(soap* s,
						ns1__serviceIDType* service_id,
						std::string variable,
						centreonscheduler__serviceGetCustomVariableResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %s)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 variable.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  for(customvariablesmember* tmp = service->custom_variables;
      tmp != NULL;
      tmp = tmp->next){
    if (!strcasecmp(tmp->variable_name, variable.c_str())) {
      if (tmp->variable_value != NULL) {
	res.value = tmp->variable_value;
      }
      res.error->code = 0;
      return (SOAP_OK);
    }
  }

  res.error->code = 1;
  res.error->message = "Service `" + service_id->service
    + "' variable `" + variable + "' not found.";


  log_debug_info(DEBUGL_COMMANDS, 2,
		 "Webservice: %s failed. %s\n",
		 __func__,
		 res.error->message.c_str());
  return (SOAP_OK);
}

/**
 *  Get the service downtime depth.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetDowntimeDepth(soap* s,
					       ns1__serviceIDType* service_id,
					       centreonscheduler__serviceGetDowntimeDepthResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->scheduled_downtime_depth;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if service has a pending flexible downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetDowntimeFlexPending(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonscheduler__serviceGetDowntimeFlexPendingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->pending_flex_downtime;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the service event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetEventHandler(soap* s,
					      ns1__serviceIDType* service_id,
					      centreonscheduler__serviceGetEventHandlerResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (service->event_handler != NULL) {
    res.value = service->event_handler;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if the service event handler is enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetEventHandlerEnabled(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonscheduler__serviceGetEventHandlerEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->event_handler_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the service event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  event_handler     The event handler.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetEventHandler(soap* s,
					      ns1__serviceIDType* service_id,
					      std::string event_handler,
					      centreonscheduler__serviceSetEventHandlerResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %s)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 event_handler.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (event_handler == "") {
    res.error->message = "Service `" + service_id->service + "' event_handler is empty.";
  }

  delete[] service->event_handler;
  service->event_handler = my_strdup(event_handler.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable service event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetEventHandlerEnabled(soap* s,
						     ns1__serviceIDType* service_id,
						     bool enable,
						     centreonscheduler__serviceSetEventHandlerEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->event_handler_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if failure prediction is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFailurePredictionEnabled(soap* s,
							  ns1__serviceIDType* service_id,
							  centreonscheduler__serviceGetFailurePredictionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = service->failure_prediction_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get service failure prediction options.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFailurePredictionOptions(soap* s,
							  ns1__serviceIDType* service_id,
							  centreonscheduler__serviceGetFailurePredictionOptionsResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (service->failure_prediction_options != NULL) {
    res.value = service->failure_prediction_options;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable failure prediction on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetFailurePredictionEnabled(soap* s,
							  ns1__serviceIDType* service_id,
							  bool enable,
							  centreonscheduler__serviceSetFailurePredictionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->failure_prediction_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the flap detection comment ID of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFlapDetectionCommentID(soap* s,
							ns1__serviceIDType* service_id,
							centreonscheduler__serviceGetFlapDetectionCommentIDResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->flapping_comment_id;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check whether flap detection is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFlapDetectionEnabled(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonscheduler__serviceGetFlapDetectionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->flap_detection_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if service is flapping.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFlapDetectionIsFlapping(soap* s,
							 ns1__serviceIDType* service_id,
							 centreonscheduler__serviceGetFlapDetectionIsFlappingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->is_flapping;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on critical state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFlapDetectionOnCritical(soap* s,
							 ns1__serviceIDType* service_id,
							 centreonscheduler__serviceGetFlapDetectionOnCriticalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->flap_detection_on_critical;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on ok state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFlapDetectionOnOk(soap* s,
						   ns1__serviceIDType* service_id,
						   centreonscheduler__serviceGetFlapDetectionOnOkResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->flap_detection_on_ok;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on unknown state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFlapDetectionOnUnknown(soap* s,
							ns1__serviceIDType* service_id,
							centreonscheduler__serviceGetFlapDetectionOnUnknownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->flap_detection_on_unknown;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on warning state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFlapDetectionOnWarning(soap* s,
							ns1__serviceIDType* service_id,
							centreonscheduler__serviceGetFlapDetectionOnWarningResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->flap_detection_on_warning;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the service high flap threshold.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFlapDetectionThresholdHigh(soap* s,
							    ns1__serviceIDType* service_id,
							    centreonscheduler__serviceGetFlapDetectionThresholdHighResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->high_flap_threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the service low flap threshold.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFlapDetectionThresholdLow(soap* s,
							   ns1__serviceIDType* service_id,
							   centreonscheduler__serviceGetFlapDetectionThresholdLowResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->low_flap_threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetFlapDetectionEnabled(soap* s,
						      ns1__serviceIDType* service_id,
						      bool enable,
						      centreonscheduler__serviceSetFlapDetectionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->flap_detection_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on critical state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetFlapDetectionOnCritical(soap* s,
							 ns1__serviceIDType* service_id,
							 bool enable,
							 centreonscheduler__serviceSetFlapDetectionOnCriticalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->flap_detection_on_critical = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on ok state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetFlapDetectionOnOk(soap* s,
						   ns1__serviceIDType* service_id,
						   bool enable,
						   centreonscheduler__serviceSetFlapDetectionOnOkResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->flap_detection_on_ok = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on unknown state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetFlapDetectionOnUnknown(soap* s,
							ns1__serviceIDType* service_id,
							bool enable,
							centreonscheduler__serviceSetFlapDetectionOnUnknownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->flap_detection_on_unknown = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on warning state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetFlapDetectionOnWarning(soap* s,
							ns1__serviceIDType* service_id,
							bool enable,
							centreonscheduler__serviceSetFlapDetectionOnWarningResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->flap_detection_on_warning = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the high flap threshold of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  threshold         New threshold.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetFlapDetectionThresholdHigh(soap* s,
							    ns1__serviceIDType* service_id,
							    double threshold,
							    centreonscheduler__serviceSetFlapDetectionThresholdHighResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %f)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 threshold);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->high_flap_threshold = threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the low flap threshold of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  threshold         New threshold.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetFlapDetectionThresholdLow(soap* s,
							   ns1__serviceIDType* service_id,
							   double threshold,
							   centreonscheduler__serviceSetFlapDetectionThresholdLowResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %f)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 threshold);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->low_flap_threshold = threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if freshness checks are enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFreshnessCheckEnabled(soap* s,
						       ns1__serviceIDType* service_id,
						       centreonscheduler__serviceGetFreshnessCheckEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->check_freshness;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if freshness check is active on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFreshnessIsActive(soap* s,
						   ns1__serviceIDType* service_id,
						   centreonscheduler__serviceGetFreshnessIsActiveResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->is_being_freshened;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the freshness threshold of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetFreshnessThreshold(soap* s,
						    ns1__serviceIDType* service_id,
						    centreonscheduler__serviceGetFreshnessThresholdResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->freshness_threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable freshness checks on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetFreshnessCheckEnabled(soap* s,
						       ns1__serviceIDType* service_id,
						       bool enable,
						       centreonscheduler__serviceSetFreshnessCheckEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->check_freshness = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the service freshness threshold.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  threshold         New threshold.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetFreshnessThreshold(soap* s,
						    ns1__serviceIDType* service_id,
						    int threshold,
						    centreonscheduler__serviceSetFreshnessThresholdResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 threshold);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->freshness_threshold = threshold;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the modified attributes on the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetModifiedAttributes(soap* s,
						    ns1__serviceIDType* service_id,
						    centreonscheduler__serviceGetModifiedAttributesResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->modified_attributes;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the service display name.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNameDisplay(soap* s,
					     ns1__serviceIDType* service_id,
					     centreonscheduler__serviceGetNameDisplayResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (service->display_name != NULL) {
    res.value = service->display_name;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the display name of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  name              Service's display name.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetNameDisplay(soap* s,
					     ns1__serviceIDType* service_id,
					     std::string name,
					     centreonscheduler__serviceSetNameDisplayResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %s)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (name == "") {
    res.error->message = "Service `" + service_id->service + "' name is empty.";
  }

  delete[] service->display_name;
  service->display_name = my_strdup(name.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the ID of the current service notification.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsCurrentID(soap* s,
							ns1__serviceIDType* service_id,
							centreonscheduler__serviceGetNotificationsCurrentIDResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->current_notification_id;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the current notification number of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsCurrentNumber(soap* s,
							    ns1__serviceIDType* service_id,
							    centreonscheduler__serviceGetNotificationsCurrentNumberResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->current_notification_number;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsEnabled(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonscheduler__serviceGetNotificationsEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->notifications_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the first notification delay of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsFirstDelay(soap* s,
							 ns1__serviceIDType* service_id,
							 centreonscheduler__serviceGetNotificationsFirstDelayResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->first_notification_delay;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the notification interval of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsInterval(soap* s,
						       ns1__serviceIDType* service_id,
						       centreonscheduler__serviceGetNotificationsIntervalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->notification_interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the time at which the last notification was sent.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsLast(soap* s,
						   ns1__serviceIDType* service_id,
						   centreonscheduler__serviceGetNotificationsLastResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->last_notification;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the time at which the next notification will be sent.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsNext(soap* s,
						   ns1__serviceIDType* service_id,
						   centreonscheduler__serviceGetNotificationsNextResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->next_notification;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is critical.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsOnCritical(soap* s,
							 ns1__serviceIDType* service_id,
							 centreonscheduler__serviceGetNotificationsOnCriticalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->notify_on_critical;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is on downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsOnDowntime(soap* s,
							 ns1__serviceIDType* service_id,
							 centreonscheduler__serviceGetNotificationsOnDowntimeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->notify_on_downtime;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is flappy.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsOnFlapping(soap* s,
							 ns1__serviceIDType* service_id,
							 centreonscheduler__serviceGetNotificationsOnFlappingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->notify_on_flapping;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service recovers.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsOnRecovery(soap* s,
							 ns1__serviceIDType* service_id,
							 centreonscheduler__serviceGetNotificationsOnRecoveryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->notify_on_recovery;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is unknown.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsOnUnknown(soap* s,
							ns1__serviceIDType* service_id,
							centreonscheduler__serviceGetNotificationsOnUnknownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->notify_on_unknown;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is warning.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsOnWarning(soap* s,
							ns1__serviceIDType* service_id,
							centreonscheduler__serviceGetNotificationsOnWarningResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->notify_on_warning;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the notification period of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetNotificationsPeriod(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonscheduler__serviceGetNotificationsPeriodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (service->notification_period != NULL) {
    res.value->timeperiod = service->notification_period;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetNotificationsEnabled(soap* s,
						      ns1__serviceIDType* service_id,
						      bool enable,
						      centreonscheduler__serviceSetNotificationsEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->notifications_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the time after which the first service notification will be sent.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  delay             Delay of the first notification.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetNotificationsFirstDelay(soap* s,
							 ns1__serviceIDType* service_id,
							 unsigned int delay,
							 centreonscheduler__serviceSetNotificationsFirstDelayResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 delay);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->first_notification_delay = delay;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the notification interval of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  interval          Notification interval.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetNotificationsInterval(soap* s,
						       ns1__serviceIDType* service_id,
						       unsigned int interval,
						       centreonscheduler__serviceSetNotificationsIntervalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 interval);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->notification_interval = interval;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is critical.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetNotificationsOnCritical(soap* s,
							 ns1__serviceIDType* service_id,
							 bool enable,
							 centreonscheduler__serviceSetNotificationsOnCriticalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->notify_on_critical = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is in downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetNotificationsOnDowntime(soap* s,
							 ns1__serviceIDType* service_id,
							 bool enable,
							 centreonscheduler__serviceSetNotificationsOnDowntimeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->notify_on_downtime = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is flappy.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetNotificationsOnFlapping(soap* s,
							 ns1__serviceIDType* service_id,
							 bool enable,
							 centreonscheduler__serviceSetNotificationsOnFlappingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->notify_on_flapping = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service recovers.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetNotificationsOnRecovery(soap* s,
							 ns1__serviceIDType* service_id,
							 bool enable,
							 centreonscheduler__serviceSetNotificationsOnRecoveryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->notify_on_recovery = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is unknown.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetNotificationsOnUnknown(soap* s,
							ns1__serviceIDType* service_id,
							bool enable,
							centreonscheduler__serviceSetNotificationsOnUnknownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->notify_on_unknown = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is warning.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetNotificationsOnWarning(soap* s,
							ns1__serviceIDType* service_id,
							bool enable,
							centreonscheduler__serviceSetNotificationsOnWarningResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->notify_on_warning = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check whether or not service is being obsessed over.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetObsessOver(soap* s,
					    ns1__serviceIDType* service_id,
					    centreonscheduler__serviceGetObsessOverResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->obsess_over_service;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable service obsession.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetObsessOver(soap* s,
					    ns1__serviceIDType* service_id,
					    bool enable,
					    centreonscheduler__serviceSetObsessOverResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->obsess_over_service = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if perfdata processing is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetPerfdataProcessingEnabled(soap* s,
							   ns1__serviceIDType* service_id,
							   centreonscheduler__serviceGetPerfdataProcessingEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->process_performance_data;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable perfdata processing.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetPerfdataProcessingEnabled(soap* s,
							   ns1__serviceIDType* service_id,
							   bool enable,
							   centreonscheduler__serviceSetPerfdataProcessingEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->process_performance_data = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last execution time of the plugin.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetPluginExecutionTime(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonscheduler__serviceGetPluginExecutionTimeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->execution_time;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if service check if currently executing.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetPluginIsExecuting(soap* s,
						   ns1__serviceIDType* service_id,
						   centreonscheduler__serviceGetPluginIsExecutingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->is_executing;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the service latency.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetPluginLatency(soap* s,
					       ns1__serviceIDType* service_id,
					       centreonscheduler__serviceGetPluginLatencyResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->latency;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the plugin output.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetPluginOutput(soap* s,
					      ns1__serviceIDType* service_id,
					      centreonscheduler__serviceGetPluginOutputResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (service->plugin_output != NULL)
    res.value = service->plugin_output;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the plugin perfdata.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetPluginPerfdata(soap* s,
						ns1__serviceIDType* service_id,
						centreonscheduler__serviceGetPluginPerfdataResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (service->perf_data != NULL) {
    res.value = service->perf_data;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if service status information are retained.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetRetainStatusInformation(soap* s,
							 ns1__serviceIDType* service_id,
							 centreonscheduler__serviceGetRetainStatusInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->retain_status_information;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if service non status information are retained.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetRetainNonStatusInformation(soap* s,
							    ns1__serviceIDType* service_id,
							    centreonscheduler__serviceGetRetainNonStatusInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->retain_nonstatus_information;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable status information retention on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetRetainStatusInformation(soap* s,
							 ns1__serviceIDType* service_id,
							 bool enable,
							 centreonscheduler__serviceSetRetainStatusInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->retain_status_information = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable non status information retention on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetRetainNonStatusInformation(soap* s,
							    ns1__serviceIDType* service_id,
							    bool enable,
							    centreonscheduler__serviceSetRetainNonStatusInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->retain_nonstatus_information = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if stalking on critical is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStalkOnCritical(soap* s,
						 ns1__serviceIDType* service_id,
						 centreonscheduler__serviceGetStalkOnCriticalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->stalk_on_critical;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if stalking on ok is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStalkOnOk(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonscheduler__serviceGetStalkOnOkResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->stalk_on_ok;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if stalking on unknown is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStalkOnUnknown(soap* s,
						ns1__serviceIDType* service_id,
						centreonscheduler__serviceGetStalkOnUnknownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->stalk_on_unknown;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if stalking on warning is enabled on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStalkOnWarning(soap* s,
						ns1__serviceIDType* service_id,
						centreonscheduler__serviceGetStalkOnWarningResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->stalk_on_warning;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on critical.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetStalkOnCritical(soap* s,
						 ns1__serviceIDType* service_id,
						 bool enable,
						 centreonscheduler__serviceSetStalkOnCriticalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->stalk_on_critical = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on ok.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetStalkOnOk(soap* s,
					   ns1__serviceIDType* service_id,
					   bool enable,
					   centreonscheduler__serviceSetStalkOnOkResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->stalk_on_ok = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on unknown.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetStalkOnUnknown(soap* s,
						ns1__serviceIDType* service_id,
						bool enable,
						centreonscheduler__serviceSetStalkOnUnknownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->stalk_on_unknown = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on warning.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetStalkOnWarning(soap* s,
						ns1__serviceIDType* service_id,
						bool enable,
						centreonscheduler__serviceSetStalkOnWarningResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s { %s }, %d)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 enable);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->stalk_on_warning = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the current state of the state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStateCurrent(soap* s,
					      ns1__serviceIDType* service_id,
					      centreonscheduler__serviceGetStateCurrentResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->current_state;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the initial state of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStateInitial(soap* s,
					      ns1__serviceIDType* service_id,
					      centreonscheduler__serviceGetStateInitialResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->initial_state;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last service state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStateLast(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonscheduler__serviceGetStateLastResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->last_state;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the state changed.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStateLastChange(soap* s,
						 ns1__serviceIDType* service_id,
						 centreonscheduler__serviceGetStateLastChangeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->last_state_change;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the service was in a critical state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStateLastCritical(soap* s,
						   ns1__serviceIDType* service_id,
						   centreonscheduler__serviceGetStateLastCriticalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->last_time_critical;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the service last hard state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStateLastHard(soap* s,
					       ns1__serviceIDType* service_id,
					       centreonscheduler__serviceGetStateLastHardResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->last_hard_state;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time at which the hard state changed.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStateLastHardChange(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonscheduler__serviceGetStateLastHardChangeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->last_hard_state_change;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the service was in an ok state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStateLastOk(soap* s,
					     ns1__serviceIDType* service_id,
					     centreonscheduler__serviceGetStateLastOkResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->last_time_ok;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the service was in an unknown state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStateLastUnknown(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonscheduler__serviceGetStateLastUnknownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->last_time_unknown;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the service was in a warning state.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStateLastWarning(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonscheduler__serviceGetStateLastWarningResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->last_time_warning;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the percent state change of the service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStatePercentChange(soap* s,
						    ns1__serviceIDType* service_id,
						    centreonscheduler__serviceGetStatePercentChangeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->percent_state_change;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get state type (hard or soft).
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceGetStateType(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonscheduler__serviceGetStateTypeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = service->state_type;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Acknowledge a problem on a host.
 *
 *  @param[in]  s                    Unused.
 *  @param[in]  host_id              Host to set data.
 *  @param[in]  acknowledgement_type Acknowledgement information.
 *  @param[out] res                  Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__acknowledgementOnHostAdd(soap* s,
						ns1__hostIDType* host_id,
						ns1__acknowledgementType* acknowledgement_type,
						centreonscheduler__acknowledgementOnHostAddResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, { %s, %s, %d, %d, %d })\n",
		 __func__,
		 host_id->name.c_str(),
		 acknowledgement_type->author.c_str(),
		 acknowledgement_type->comment.c_str(),
		 acknowledgement_type->notify,
		 acknowledgement_type->persistent,
		 acknowledgement_type->sticky);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  int type = (acknowledgement_type->sticky == true ? ACKNOWLEDGEMENT_STICKY : ACKNOWLEDGEMENT_NORMAL);

  char* author = my_strdup(acknowledgement_type->author.c_str());
  char* comment = my_strdup(acknowledgement_type->comment.c_str());

  acknowledge_host_problem(host,
			   author,
			   comment,
			   type,
			   acknowledgement_type->notify,
			   acknowledgement_type->persistent);

  delete[] author;
  delete[] comment;

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Remove an acknowledgement on a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__acknowledgementOnHostRemove(soap* s,
						   ns1__hostIDType* host_id,
						   centreonscheduler__acknowledgementOnHostRemoveResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 host_id->name.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  remove_host_acknowledgement(host);
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Acknowledge a problem on a service.
 *
 *  @param[in]  s                    Unused.
 *  @param[in]  service_id           Service to set data.
 *  @param[in]  acknowledgement_type Acknowledgement information.
 *  @param[out] res                  Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__acknowledgementOnServiceAdd(soap* s,
						   ns1__serviceIDType* service_id,
						   ns1__acknowledgementType* acknowledgement_type,
						   centreonscheduler__acknowledgementOnServiceAddResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } }, { %s, %s, %d, %d, %d })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 acknowledgement_type->author.c_str(),
		 acknowledgement_type->comment.c_str(),
		 acknowledgement_type->notify,
		 acknowledgement_type->persistent,
		 acknowledgement_type->sticky);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  int type = (acknowledgement_type->sticky == true ? ACKNOWLEDGEMENT_STICKY : ACKNOWLEDGEMENT_NORMAL);

  char* author = my_strdup(acknowledgement_type->author.c_str());
  char* comment = my_strdup(acknowledgement_type->comment.c_str());

  acknowledge_service_problem(service,
			      author,
			      comment,
			      type,
			      acknowledgement_type->notify,
			      acknowledgement_type->persistent);
  delete[] author;
  delete[] comment;

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Remove an acknowledgement on a service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__acknowledgementOnServiceRemove(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonscheduler__acknowledgementOnServiceRemoveResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  remove_service_acknowledgement(service);
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Process a host check result.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  result_type       Process Result to check.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__checkHostProcessResult(soap* s,
					      ns1__hostIDType* host_id,
					      ns1__checkResultType* result_type,
					      centreonscheduler__checkHostProcessResultResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, { %d, ... })\n",
		 __func__,
		 host_id->name.c_str(),
		 result_type->retval);

  log_debug_info(DEBUGL_CHECKS, 2,
		 "Webservice: call %s: output=%s\n",
		 __func__,
		 result_type->output.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (process_passive_host_check(time(NULL),
				 host->name,
				 result_type->retval,
				 result_type->output.c_str()) == ERROR) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' check process result failed.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Schedule a host check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__checkHostSchedule(soap* s,
					 ns1__hostIDType* host_id,
					 long delay,
					 centreonscheduler__checkHostScheduleResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %ld)\n",
		 __func__,
		 host_id->name.c_str(),
		 delay);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  schedule_host_check(host, delay, CHECK_OPTION_NONE);
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Schedule a forced host check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__checkHostScheduleForced(soap* s,
					       ns1__hostIDType* host_id,
					       long delay,
					       centreonscheduler__checkHostScheduleForcedResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %ld)\n",
		 __func__,
		 host_id->name.c_str(),
		 delay);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  schedule_host_check(host, delay, CHECK_OPTION_FORCE_EXECUTION);
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Schedule a check of all services associated with the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__checkHostScheduleServices(soap* s,
						 ns1__hostIDType* host_id,
						 long delay,
						 centreonscheduler__checkHostScheduleServicesResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %ld)\n",
		 __func__,
		 host_id->name.c_str(),
		 delay);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
    if (tmp->service_ptr != NULL) {
      schedule_service_check(tmp->service_ptr, delay, CHECK_OPTION_NONE);
    }
  }

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Schedule a forced check of all services associated with the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule's delay.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__checkHostScheduleServicesForced(soap* s,
						       ns1__hostIDType* host_id,
						       long delay,
						       centreonscheduler__checkHostScheduleServicesForcedResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %ld)\n",
		 __func__,
		 host_id->name.c_str(),
		 delay);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
    if (tmp->service_ptr != NULL) {
      schedule_service_check(tmp->service_ptr, delay, CHECK_OPTION_FORCE_EXECUTION);
    }
  }

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Process a service check result.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  result_type       Process Result to check.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__checkServiceProcessResult(soap* s,
						 ns1__serviceIDType* service_id,
						 ns1__checkResultType* result_type,
						 centreonscheduler__checkServiceProcessResultResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } }, { %d, ... })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 result_type->retval);

  log_debug_info(DEBUGL_CHECKS, 2,
		 "Webservice: call %s: output=%s\n",
		 __func__,
		 result_type->output.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }
  if (process_passive_service_check(time(NULL),
				    service_id->host->name.c_str(),
				    service->description,
				    result_type->retval,
				    result_type->output.c_str()) == ERROR) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' check process result failed "
      + "for host `" + service_id->host->name + "'.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Schedule a service check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__checkServiceSchedule(soap* s,
					    ns1__serviceIDType* service_id,
					    long delay,
					    centreonscheduler__checkServiceScheduleResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } }, %ld)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 delay);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  schedule_service_check(service, delay, CHECK_OPTION_NONE);
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Schedule a forced service check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__checkServiceScheduleForced(soap* s,
						  ns1__serviceIDType* service_id,
						  long delay,
						  centreonscheduler__checkServiceScheduleForcedResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } }, %ld)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 delay);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  schedule_service_check(service, delay, CHECK_OPTION_FORCE_EXECUTION);
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the author of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__downtimeGetAuthor(soap* s,
					 ns1__downtimeIDType* downtime_id,
					 centreonscheduler__downtimeGetAuthorResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%lu)\n",
		 __func__,
		 downtime_id->downtime);

  scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
  if (downtime == NULL) {
    std::ostringstream oss;
    oss << downtime_id->downtime;

    res.error->code = 1;
    res.error->message = "Downtime `" + oss.str() + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (downtime->author != NULL) {
    res.value = downtime->author;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the duration of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__downtimeGetDuration(soap* s,
					   ns1__downtimeIDType* downtime_id,
					   centreonscheduler__downtimeGetDurationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%lu)\n",
		 __func__,
		 downtime_id->downtime);

  scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
  if (downtime == NULL) {
    std::ostringstream oss;
    oss << downtime_id->downtime;

    res.error->code = 1;
    res.error->message = "Downtime `" + oss.str() + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = downtime->duration;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the end time of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__downtimeGetEnd(soap* s,
				      ns1__downtimeIDType* downtime_id,
				      centreonscheduler__downtimeGetEndResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%lu)\n",
		 __func__,
		 downtime_id->downtime);

  scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
  if (downtime == NULL) {
    std::ostringstream oss;
    oss << downtime_id->downtime;

    res.error->code = 1;
    res.error->message = "Downtime `" + oss.str() + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = downtime->end_time;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if a downtime is fixed or not.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__downtimeGetFixed(soap* s,
					ns1__downtimeIDType* downtime_id,
					centreonscheduler__downtimeGetFixedResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%lu)\n",
		 __func__,
		 downtime_id->downtime);

  scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
  if (downtime == NULL) {
    std::ostringstream oss;
    oss << downtime_id->downtime;

    res.error->code = 1;
    res.error->message = "Downtime `" + oss.str() + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = downtime->fixed;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the start time of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__downtimeGetStart(soap* s,
					ns1__downtimeIDType* downtime_id,
					centreonscheduler__downtimeGetStartResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%lu)\n",
		 __func__,
		 downtime_id->downtime);

  scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
  if (downtime == NULL) {
    std::ostringstream oss;
    oss << downtime_id->downtime;

    res.error->code = 1;
    res.error->message = "Downtime `" + oss.str() + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = downtime->start_time;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Delete a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to set data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__downtimeDelete(soap* s,
				      ns1__downtimeIDType* downtime_id,
				      centreonscheduler__downtimeDeleteResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%lu)\n",
		 __func__,
		 downtime_id->downtime);

  if (unschedule_downtime(HOST_DOWNTIME, downtime_id->downtime) == ERROR
      && unschedule_downtime(SERVICE_DOWNTIME, downtime_id->downtime) == ERROR) {
    std::ostringstream oss;
    oss << downtime_id->downtime;

    res.error->code = 1;
    res.error->message = "Downtime `" + oss.str() + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Schedule a downtime on a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  downtime_type     Downtime information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__downtimeAddToHost(soap* s,
					 ns1__hostIDType* host_id,
					 ns1__downtimeType* downtime_type,
					 centreonscheduler__downtimeAddToHostResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, { %ld, %ld, %d, %lu, %f, %s, %s })\n",
		 __func__,
		 host_id->name.c_str(),
		 downtime_type->starttime,
		 downtime_type->endtime,
		 downtime_type->fixed,
		 downtime_type->triggerid->downtime,
		 downtime_type->duration,
		 downtime_type->author.c_str(),
		 downtime_type->comment.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  char* author = my_strdup(downtime_type->author.c_str());
  char* comment = my_strdup(downtime_type->comment.c_str());

  if (schedule_downtime(HOST_DOWNTIME,
			host->name,
			NULL,
			time(NULL),
			author,
			comment,
			downtime_type->starttime,
			downtime_type->endtime,
			downtime_type->fixed,
			downtime_type->triggerid->downtime,
			downtime_type->duration,
			&res.downtimeid->downtime) == ERROR) {
    delete[] author;
    delete[] comment;
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' add downtime to host failed.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] author;
  delete[] comment;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Schedule downtime for a host and all of its children.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  downtime_type     Downtime information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__downtimeAddAndPropagateToHost(soap* s,
						     ns1__hostIDType* host_id,
						     ns1__downtimeType* downtime_type,
						     centreonscheduler__downtimeAddAndPropagateToHostResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, { %ld, %ld, %d, %lu, %f, %s, %s })\n",
		 __func__,
		 host_id->name.c_str(),
		 downtime_type->starttime,
		 downtime_type->endtime,
		 downtime_type->fixed,
		 downtime_type->triggerid->downtime,
		 downtime_type->duration,
		 downtime_type->author.c_str(),
		 downtime_type->comment.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  time_t entry_time = time(NULL);

  char* author = my_strdup(downtime_type->author.c_str());
  char* comment = my_strdup(downtime_type->comment.c_str());

  if (schedule_downtime(HOST_DOWNTIME,
			host->name,
			NULL,
			entry_time,
			author,
			comment,
			downtime_type->starttime,
			downtime_type->endtime,
			downtime_type->fixed,
			downtime_type->triggerid->downtime,
			downtime_type->duration,
			&res.downtimeid->downtime) == ERROR) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' add downtime faild.";
  }
  else {
    schedule_and_propagate_downtime(host,
				    entry_time,
				    author,
				    comment,
				    downtime_type->starttime,
				    downtime_type->endtime,
				    downtime_type->fixed,
				    0,
				    downtime_type->duration);
    res.error->code = 0;
  }

  delete[] author;
  delete[] comment;

  return (SOAP_OK);
}

/**
 *  Schedule downtime for a host and all of its children.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  downtime_type     Downtime information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__downtimeAddAndPropagateTriggeredToHost(soap* s,
							      ns1__hostIDType* host_id,
							      ns1__downtimeType* downtime_type,
							      centreonscheduler__downtimeAddAndPropagateTriggeredToHostResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, { %ld, %ld, %d, %lu, %f, %s, %s })\n",
		 __func__,
		 host_id->name.c_str(),
		 downtime_type->starttime,
		 downtime_type->endtime,
		 downtime_type->fixed,
		 downtime_type->triggerid->downtime,
		 downtime_type->duration,
		 downtime_type->author.c_str(),
		 downtime_type->comment.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  time_t entry_time = time(NULL);

  char* author = my_strdup(downtime_type->author.c_str());
  char* comment = my_strdup(downtime_type->comment.c_str());

  if (schedule_downtime(HOST_DOWNTIME,
			host->name,
			NULL,
			entry_time,
			author,
			comment,
			downtime_type->starttime,
			downtime_type->endtime,
			downtime_type->fixed,
			downtime_type->triggerid->downtime,
			downtime_type->duration,
			&res.downtimeid->downtime) == ERROR) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' add downtime faild.";
  }
  else {
    schedule_and_propagate_downtime(host,
				    entry_time,
				    author,
				    comment,
				    downtime_type->starttime,
				    downtime_type->endtime,
				    downtime_type->fixed,
				    res.downtimeid->downtime,
				    downtime_type->duration);
    res.error->code = 0;
  }

  delete[] author;
  delete[] comment;

  return (SOAP_OK);
}

/**
 *  Schedule a downtime on all services of a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  downtime_type     Downtime information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__downtimeAddToHostServices(soap* s,
						 ns1__hostIDType* host_id,
						 ns1__downtimeType* downtime_type,
						 centreonscheduler__downtimeAddToHostServicesResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, { %ld, %ld, %d, %lu, %f, %s, %s })\n",
		 __func__,
		 host_id->name.c_str(),
		 downtime_type->starttime,
		 downtime_type->endtime,
		 downtime_type->fixed,
		 downtime_type->triggerid->downtime,
		 downtime_type->duration,
		 downtime_type->author.c_str(),
		 downtime_type->comment.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.error->code = 0;

  time_t entry_time = time(NULL);

  char* author = my_strdup(downtime_type->author.c_str());
  char* comment = my_strdup(downtime_type->comment.c_str());

  for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
    if (tmp->service_ptr !=NULL) {
      if (schedule_downtime(SERVICE_DOWNTIME,
			    host->name,
			    tmp->service_ptr->description,
			    entry_time,
			    author,
			    comment,
			    downtime_type->starttime,
			    downtime_type->endtime,
			    downtime_type->fixed,
			    downtime_type->triggerid->downtime,
			    downtime_type->duration,
			    NULL) == ERROR) {
	res.error->code = 1;
      }
    }
  }

  delete[] author;
  delete[] comment;

  if (res.error->code == 0) {
    res.error->message = "Host `" + host_id->name + "' one or more service cannot schedule downtime.";
  }

  return (SOAP_OK);
}

/**
 *  Schedule a downtime on a service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  downtime_type     Downtime information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__downtimeAddToService(soap* s,
					    ns1__serviceIDType* service_id,
					    ns1__downtimeType* downtime_type,
					    centreonscheduler__downtimeAddToServiceResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } }, { %ld, %ld, %d, %lu, %f, %s, %s })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 downtime_type->starttime,
		 downtime_type->endtime,
		 downtime_type->fixed,
		 downtime_type->triggerid->downtime,
		 downtime_type->duration,
		 downtime_type->author.c_str(),
		 downtime_type->comment.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  char* author = my_strdup(downtime_type->author.c_str());
  char* comment = my_strdup(downtime_type->comment.c_str());

  if (schedule_downtime(SERVICE_DOWNTIME,
			service->host_name,
			service->description,
			time(NULL),
			author,
			comment,
			downtime_type->starttime,
			downtime_type->endtime,
			downtime_type->fixed,
			downtime_type->triggerid->downtime,
			downtime_type->duration,
			&res.downtimeid->downtime) == ERROR) {
    delete[] author;
    delete[] comment;
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service + "' add downtime to service failed.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] author;
  delete[] comment;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Delay a host notification.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Notification delay.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__notificationHostDelay(soap* s,
					     ns1__hostIDType* host_id,
					     long delay,
					     centreonscheduler__notificationHostDelayResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %ld)\n",
		 __func__,
		 host_id->name.c_str(),
		 delay);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  host->next_host_notification = delay;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Send a notification on a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  notification_type Notification information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__notificationHostSend(soap* s,
					    ns1__hostIDType* host_id,
					    ns1__notificationType* notification_type,
					    centreonscheduler__notificationHostSendResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, { %s, %d, %s, %d, %d })\n",
		 __func__,
		 host_id->name.c_str(),
		 notification_type->author.c_str(),
		 notification_type->broadcast,
		 notification_type->comment.c_str(),
		 notification_type->forced,
		 notification_type->increment);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  int options = (notification_type->broadcast == true ? NOTIFICATION_OPTION_BROADCAST : 0)
    | (notification_type->forced == true ? NOTIFICATION_OPTION_FORCED : 0)
    | (notification_type->increment == true ? NOTIFICATION_OPTION_INCREMENT : 0);

  char* author = my_strdup(notification_type->author.c_str());
  char* comment = my_strdup(notification_type->comment.c_str());

  if (host_notification(host,
			NOTIFICATION_CUSTOM,
			author,
			comment,
			options) == ERROR) {
    delete[] author;
    delete[] comment;
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' send notification failed.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] author;
  delete[] comment;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Delay a service notification.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  delay             Notification delay.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__notificationServiceDelay(soap* s,
						ns1__serviceIDType* service_id,
						long delay,
						centreonscheduler__notificationServiceDelayResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } }, %ld)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 delay);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  service->next_notification = delay;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Send a notification on a service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  notification_type Notification information.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__notificationServiceSend(soap* s,
					       ns1__serviceIDType* service_id,
					       ns1__notificationType* notification_type,
					       centreonscheduler__notificationServiceSendResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } }, { %s, %d, %s, %d, %d })\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 notification_type->author.c_str(),
		 notification_type->broadcast,
		 notification_type->comment.c_str(),
		 notification_type->forced,
		 notification_type->increment);

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  int options = (notification_type->broadcast == true ? NOTIFICATION_OPTION_BROADCAST : 0)
    | (notification_type->forced == true ? NOTIFICATION_OPTION_FORCED : 0)
    | (notification_type->increment == true ? NOTIFICATION_OPTION_INCREMENT : 0);


  char* author = my_strdup(notification_type->author.c_str());
  char* comment = my_strdup(notification_type->comment.c_str());

  if (service_notification(service,
			   NOTIFICATION_CUSTOM,
			   author,
			   comment,
			   options) == ERROR) {
    delete[] author;
    delete[] comment;
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service + "' send notification failed.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] author;
  delete[] comment;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if event handlers are enabled globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getEventHandlersEnabled(soap* s,
					       centreonscheduler__getEventHandlersEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = enable_event_handlers;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if failure prediction is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getFailurePredictionEnabled(soap* s,
						   centreonscheduler__getFailurePredictionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = enable_failure_prediction;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if flap detection is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getFlapDetectionEnabled(soap* s,
					       centreonscheduler__getFlapDetectionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = enable_flap_detection;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if hosts active checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getHostsChecksActiveEnabled(soap* s,
						   centreonscheduler__getHostsChecksActiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = execute_host_checks;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if hosts passive checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getHostsChecksPassiveEnabled(soap* s,
						    centreonscheduler__getHostsChecksPassiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = accept_passive_host_checks;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the global host event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getHostsEventHandler(soap* s,
					    centreonscheduler__getHostsEventHandlerResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.command->command = global_host_event_handler;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if the hosts freshness checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getHostsFreshnessChecksEnabled(soap* s,
						      centreonscheduler__getHostsFreshnessChecksEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = check_host_freshness;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if host obsession is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getHostsObsessOverEnabled(soap* s,
						 centreonscheduler__getHostsObsessOverEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = obsess_over_hosts;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if notifications are enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getNotificationsEnabled(soap* s,
					       centreonscheduler__getNotificationsEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = enable_notifications;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if performance data processing is enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getPerfdataProcessingEnabled(soap* s,
						    centreonscheduler__getPerfdataProcessingEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = process_performance_data;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if services active checks are enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getServicesChecksActiveEnabled(soap* s,
						      centreonscheduler__getServicesChecksActiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = execute_service_checks;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if services passive checks are enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getServicesChecksPassiveEnabled(soap* s,
						       centreonscheduler__getServicesChecksPassiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = accept_passive_service_checks;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the global service event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getServicesEventHandler(soap* s,
					       centreonscheduler__getServicesEventHandlerResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.command->command = global_service_event_handler;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if services freshness checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getServicesFreshnessChecksEnabled(soap* s,
							 centreonscheduler__getServicesFreshnessChecksEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = check_service_freshness;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if services obsession is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getServicesObsessOverEnabled(soap* s,
						    centreonscheduler__getServicesObsessOverEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  res.value = obsess_over_services;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable event handlers globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setEventHandlersEnabled(soap* s,
					       bool enable,
					       centreonscheduler__setEventHandlersEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);

  enable_event_handlers = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable failure prediction globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setFailurePredictionEnabled(soap* s,
						   bool enable,
						   centreonscheduler__setFailurePredictionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  enable_failure_prediction = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setFlapDetectionEnabled(soap* s,
					       bool enable,
					       centreonscheduler__setFlapDetectionEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  enable_flap_detection = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable active host checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostsChecksActiveEnabled(soap* s,
						   bool enable,
						   centreonscheduler__setHostsChecksActiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  execute_host_checks = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable passive host checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostsChecksPassiveEnabled(soap* s,
						    bool enable,
						    centreonscheduler__setHostsChecksPassiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  accept_passive_host_checks = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Change the global host event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  command_id        Command to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostsEventHandler(soap* s,
					    ns1__commandIDType* command_id,
					    centreonscheduler__setHostsEventHandlerResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 command_id->command.c_str());

  command* command = find_command(command_id->command.c_str());
  if (command == NULL) {
    res.error->code = 1;
    res.error->message = "Command `" + command_id->command + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] global_host_event_handler;
  global_host_event_handler = my_strdup(command_id->command.c_str());
  global_host_event_handler_ptr = command;

  modified_host_process_attributes |= MODATTR_EVENT_HANDLER_COMMAND;

  broker_adaptive_program_data(NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
			       NEBFLAG_NONE,NEBATTR_NONE,
			       CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER,
			       MODATTR_EVENT_HANDLER_COMMAND,
			       modified_host_process_attributes,
			       MODATTR_NONE,
			       modified_service_process_attributes,
			       NULL);

  update_program_status(false);

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable host freshness checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostsFreshnessChecksEnabled(soap* s,
						      bool enable,
						      centreonscheduler__setHostsFreshnessChecksEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  check_host_freshness = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable obsession over host checks.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostsObsessOverEnabled(soap* s,
						 bool enable,
						 centreonscheduler__setHostsObsessOverEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  obsess_over_hosts = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setNotificationsEnabled(soap* s,
					       bool enable,
					       centreonscheduler__setNotificationsEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  enable_notifications = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable performance data processing globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setPerfdataProcessingEnabled(soap* s,
						    bool enable,
						    centreonscheduler__setPerfdataProcessingEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  process_performance_data = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable active service checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setServicesChecksActiveEnabled(soap* s,
						      bool enable,
						      centreonscheduler__setServicesChecksActiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  execute_service_checks = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable passive service checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setServicesChecksPassiveEnabled(soap* s,
						       bool enable,
						       centreonscheduler__setServicesChecksPassiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  accept_passive_service_checks = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Change the global service event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  command_id        Command to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setServicesEventHandler(soap* s,
					       ns1__commandIDType* command_id,
					       centreonscheduler__setServicesEventHandlerResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 command_id->command.c_str());

  command* command = find_command(command_id->command.c_str());
  if (command == NULL) {
    res.error->code = 1;
    res.error->message = "Command `" + command_id->command + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] global_service_event_handler;
  global_service_event_handler = my_strdup(command_id->command.c_str());
  global_service_event_handler_ptr = command;

  modified_service_process_attributes |= MODATTR_EVENT_HANDLER_COMMAND;

  broker_adaptive_program_data(NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
			       NEBFLAG_NONE,
			       NEBATTR_NONE,
			       CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER,
			       MODATTR_NONE,
			       modified_host_process_attributes,
			       MODATTR_EVENT_HANDLER_COMMAND,
			       modified_service_process_attributes,
			       NULL);

  update_program_status(false);

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable service freshness checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setServicesFreshnessChecksEnabled(soap* s,
							 bool enable,
							 centreonscheduler__setServicesFreshnessChecksEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  check_service_freshness = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable obsession over service checks.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setServicesObsessOverEnabled(soap* s,
						    bool enable,
						    centreonscheduler__setServicesObsessOverEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s(%d)\n", __func__, enable);

  obsess_over_services = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the host check period.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  timeperiod_id     Timeperiod to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetCheckPeriod(soap* s,
					  ns1__hostIDType* host_id,
					  ns1__timeperiodIDType* timeperiod_id,
					  centreonscheduler__hostSetCheckPeriodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 host_id->name.c_str(),
		 timeperiod_id->timeperiod.c_str());

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (timeperiod_id->timeperiod == "") {
    res.error->message = "Host `" + host_id->name + "' timeperiod is empty.";
  }

  delete[] host->check_period;
  host->check_period = my_strdup(timeperiod_id->timeperiod.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable active checks on all services of the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetServicesCheckActiveEnabled(soap* s,
							 ns1__hostIDType* host_id,
							 bool enable,
							 centreonscheduler__hostSetServicesCheckActiveEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
    if (tmp->service_ptr != NULL) {
      tmp->service_ptr->checks_enabled = enable;
    }
  }

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on all services of the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__hostSetServicesNotificationsEnabled(soap* s,
							   ns1__hostIDType* host_id,
							   bool enable,
							   centreonscheduler__hostSetServicesNotificationsEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 host_id->name.c_str(),
		 enable);

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
    if (tmp->service_ptr != NULL) {
      tmp->service_ptr->notifications_enabled = enable;
    }
  }

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the service check period.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  timeperiod_id     Timeperiod to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetCheckPeriod(soap* s,
					     ns1__serviceIDType* service_id,
					     ns1__timeperiodIDType* timeperiod_id,
					     centreonscheduler__serviceSetCheckPeriodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } }, %s)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 timeperiod_id->timeperiod.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (timeperiod_id->timeperiod == "") {
    res.error->message = "Service `" + service_id->service + "' timeperiod is empty.";
  }

  delete[] service->check_period;
  service->check_period = my_strdup(timeperiod_id->timeperiod.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set notification period of service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  timeperiod_id     Timeperiod to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__serviceSetNotificationsPeriod(soap* s,
						     ns1__serviceIDType* service_id,
						     ns1__timeperiodIDType* timeperiod_id,
						     centreonscheduler__serviceSetNotificationsPeriodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s({ %s, { %s } }, %s)\n",
		 __func__,
		 service_id->service.c_str(),
		 service_id->host->name.c_str(),
		 timeperiod_id->timeperiod.c_str());

  service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
  if (service == NULL) {
    res.error->code = 1;
    res.error->message = "Service `" + service_id->service
      + "' with Host `" + service_id->host->name + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] service->notification_period;
  service->notification_period = my_strdup(timeperiod_id->timeperiod.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the contact alias.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetAlias(soap* s,
				       ns1__contactIDType* contact_id,
				       centreonscheduler__contactGetAliasResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (contact->alias != NULL) {
    res.value = contact->alias;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if the contact can submit external commands.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetCanSubmitCommands(soap* s,
						   ns1__contactIDType* contact_id,
						   centreonscheduler__contactGetCanSubmitCommandsResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->can_submit_commands;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the contact email.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetEmail(soap* s,
				       ns1__contactIDType* contact_id,
				       centreonscheduler__contactGetEmailResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (contact->email != NULL) {
    res.value = contact->email;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the contact modified attributes.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetModifiedAttributes(soap* s,
						    ns1__contactIDType* contact_id,
						    centreonscheduler__contactGetModifiedAttributesResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->modified_attributes;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the contact host modified attributes.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetModifiedAttributesHost(soap* s,
							ns1__contactIDType* contact_id,
							centreonscheduler__contactGetModifiedAttributesHostResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->modified_host_attributes;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the contact service modified attributes.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetModifiedAttributesService(soap* s,
							   ns1__contactIDType* contact_id,
							   centreonscheduler__contactGetModifiedAttributesServiceResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->modified_service_attributes;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host notification command of the contact.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnHostCommand(soap* s,
							    ns1__contactIDType* contact_id,
							    centreonscheduler__contactGetNotificationsOnHostCommandResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (contact->host_notification_commands->cmd != NULL) {
    res.command->command = contact->host_notification_commands->cmd;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified by down hosts.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnHostDown(soap* s,
							 ns1__contactIDType* contact_id,
							 centreonscheduler__contactGetNotificationsOnHostDownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->notify_on_host_down;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified by hosts downtimes.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnHostDowntime(soap* s,
							     ns1__contactIDType* contact_id,
							     centreonscheduler__contactGetNotificationsOnHostDowntimeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->notify_on_host_downtime;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact will be notified by host events.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnHostEnabled(soap* s,
							    ns1__contactIDType* contact_id,
							    centreonscheduler__contactGetNotificationsOnHostEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }


  res.value = contact->host_notifications_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when hosts are flapping.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnHostFlapping(soap* s,
							     ns1__contactIDType* contact_id,
							     centreonscheduler__contactGetNotificationsOnHostFlappingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->notify_on_host_flapping;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the contact received a host notification.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnHostLast(soap* s,
							 ns1__contactIDType* contact_id,
							 centreonscheduler__contactGetNotificationsOnHostLastResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->last_host_notification;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when hosts recover.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnHostRecovery(soap* s,
							     ns1__contactIDType* contact_id,
							     centreonscheduler__contactGetNotificationsOnHostRecoveryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->notify_on_host_recovery;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the host notification timeperiod of the contact.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnHostTimeperiod(soap* s,
							       ns1__contactIDType* contact_id,
							       centreonscheduler__contactGetNotificationsOnHostTimeperiodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (contact->host_notification_period_ptr->name != NULL)
    res.value->timeperiod = contact->host_notification_period_ptr->name;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when hosts are unreachable.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnHostUnreachable(soap* s,
								ns1__contactIDType* contact_id,
								centreonscheduler__contactGetNotificationsOnHostUnreachableResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->notify_on_host_unreachable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the service notification command of the contact.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnServiceCommand(soap* s,
							       ns1__contactIDType* contact_id,
							       centreonscheduler__contactGetNotificationsOnServiceCommandResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (contact->service_notification_commands->cmd != NULL) {
    res.command->command = contact->service_notification_commands->cmd;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when services are critical.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnServiceCritical(soap* s,
								ns1__contactIDType* contact_id,
								centreonscheduler__contactGetNotificationsOnServiceCriticalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->notify_on_service_critical;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified by services downtimes.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnServiceDowntime(soap* s,
								ns1__contactIDType* contact_id,
								centreonscheduler__contactGetNotificationsOnServiceDowntimeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->notify_on_service_downtime;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact will be notified by service events.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnServiceEnabled(soap* s,
							       ns1__contactIDType* contact_id,
							       centreonscheduler__contactGetNotificationsOnServiceEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->service_notifications_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when services are flapping.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnServiceFlapping(soap* s,
								ns1__contactIDType* contact_id,
								centreonscheduler__contactGetNotificationsOnServiceFlappingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->notify_on_service_flapping;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the last time the contact received a service notification.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnServiceLast(soap* s,
							    ns1__contactIDType* contact_id,
							    centreonscheduler__contactGetNotificationsOnServiceLastResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->last_service_notification;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when services recover.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnServiceRecovery(soap* s,
								ns1__contactIDType* contact_id,
								centreonscheduler__contactGetNotificationsOnServiceRecoveryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->notify_on_service_recovery;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the service notification timeperiod of the contact.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnServiceTimeperiod(soap* s,
								  ns1__contactIDType* contact_id,
								  centreonscheduler__contactGetNotificationsOnServiceTimeperiodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (contact->service_notification_period_ptr->name != NULL)
    res.value->timeperiod = contact->service_notification_period_ptr->name;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when services states are unknown.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetNotificationsOnServiceUnknown(soap* s,
							       ns1__contactIDType* contact_id,
							       centreonscheduler__contactGetNotificationsOnServiceUnknownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->notify_on_service_unknown;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Get the contact pager.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetPager(soap* s,
				       ns1__contactIDType* contact_id,
				       centreonscheduler__contactGetPagerResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  if (contact->pager != NULL) {
    res.value = contact->pager;
  }
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact status information should be retained.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetRetainStatusInformation(soap* s,
							 ns1__contactIDType* contact_id,
							 centreonscheduler__contactGetRetainStatusInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->retain_status_information;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Check if contact non status information should be retained.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactGetRetainStatusNonInformation(soap* s,
							    ns1__contactIDType* contact_id,
							    centreonscheduler__contactGetRetainStatusNonInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s)\n",
		 __func__,
		 contact_id->contact.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  res.value = contact->retain_nonstatus_information;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the contact alias.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  alias             New contact's alias.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetAlias(soap* s,
				       ns1__contactIDType* contact_id,
				       std::string alias,
				       centreonscheduler__contactSetAliasResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 alias.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] contact->alias;
  contact->alias = my_strdup(alias.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable a contact to submit commands.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetCanSubmitCommands(soap* s,
						   ns1__contactIDType* contact_id,
						   bool enable,
						   centreonscheduler__contactSetCanSubmitCommandsResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->can_submit_commands = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the email address of a contact.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  email             New contact's email.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetEmail(soap* s,
				       ns1__contactIDType* contact_id,
				       std::string email,
				       centreonscheduler__contactSetEmailResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 email.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] contact->email;
  contact->email = my_strdup(email.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host down.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnHostDown(soap* s,
							 ns1__contactIDType* contact_id,
							 bool enable,
							 centreonscheduler__contactSetNotificationsOnHostDownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->notify_on_host_down = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnHostDowntime(soap* s,
							     ns1__contactIDType* contact_id,
							     bool enable,
							     centreonscheduler__contactSetNotificationsOnHostDowntimeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->notify_on_host_downtime = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnHostEnabled(soap* s,
							    ns1__contactIDType* contact_id,
							    bool enable,
							    centreonscheduler__contactSetNotificationsOnHostEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->host_notifications_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on flappy hosts.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnHostFlapping(soap* s,
							     ns1__contactIDType* contact_id,
							     bool enable,
							     centreonscheduler__contactSetNotificationsOnHostFlappingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->notify_on_host_flapping = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host recover.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnHostRecovery(soap* s,
							     ns1__contactIDType* contact_id,
							     bool enable,
							     centreonscheduler__contactSetNotificationsOnHostRecoveryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->notify_on_host_recovery = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the host notifications timeperiod of the contact.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  timeperiod_id     Timeperiod to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnHostTimeperiod(soap* s,
							       ns1__contactIDType* contact_id,
							       ns1__timeperiodIDType* timeperiod_id,
							       centreonscheduler__contactSetNotificationsOnHostTimeperiodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 timeperiod_id->timeperiod.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] contact->host_notification_period;
  contact->host_notification_period = my_strdup(timeperiod_id->timeperiod.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host unreachable.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnHostUnreachable(soap* s,
								ns1__contactIDType* contact_id,
								bool enable,
								centreonscheduler__contactSetNotificationsOnHostUnreachableResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->notify_on_host_unreachable = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service critical.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnServiceCritical(soap* s,
								ns1__contactIDType* contact_id,
								bool enable,
								centreonscheduler__contactSetNotificationsOnServiceCriticalResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->notify_on_service_critical = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnServiceDowntime(soap* s,
								ns1__contactIDType* contact_id,
								bool enable,
								centreonscheduler__contactSetNotificationsOnServiceDowntimeResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->notify_on_service_downtime = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnServiceEnabled(soap* s,
							       ns1__contactIDType* contact_id,
							       bool enable,
							       centreonscheduler__contactSetNotificationsOnServiceEnabledResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->service_notifications_enabled = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service flapping.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnServiceFlapping(soap* s,
								ns1__contactIDType* contact_id,
								bool enable,
								centreonscheduler__contactSetNotificationsOnServiceFlappingResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->notify_on_service_flapping = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service recovery.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnServiceRecovery(soap* s,
								ns1__contactIDType* contact_id,
								bool enable,
								centreonscheduler__contactSetNotificationsOnServiceRecoveryResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->notify_on_service_recovery = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the service notification timeperiod of the contact.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  timeperiod_id     Timeperiod to get data.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnServiceTimeperiod(soap* s,
								  ns1__contactIDType* contact_id,
								  ns1__timeperiodIDType* timeperiod_id,
								  centreonscheduler__contactSetNotificationsOnServiceTimeperiodResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 timeperiod_id->timeperiod.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] contact->service_notification_period;
  contact->service_notification_period = my_strdup(timeperiod_id->timeperiod.c_str());
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service unknown.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnServiceUnknown(soap* s,
							       ns1__contactIDType* contact_id,
							       bool enable,
							       centreonscheduler__contactSetNotificationsOnServiceUnknownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->notify_on_service_unknown = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service warning.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnServiceWarning(soap* s,
							       ns1__contactIDType* contact_id,
							       bool enable,
							       centreonscheduler__contactSetNotificationsOnServiceWarningResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->notify_on_service_warning = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the contact pager.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  pager             New contact's pager.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetPager(soap* s,
				       ns1__contactIDType* contact_id,
				       std::string pager,
				       centreonscheduler__contactSetPagerResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 pager.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] contact->pager;
  contact->pager = my_strdup(pager.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable status information retention for contact.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetRetainStatusInformation(soap* s,
							 ns1__contactIDType* contact_id,
							 bool enable,
							 centreonscheduler__contactSetRetainStatusInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->retain_status_information = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable non status information retention for contact.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetRetainStatusNonInformation(soap* s,
							    ns1__contactIDType* contact_id,
							    bool enable,
							    centreonscheduler__contactSetRetainStatusNonInformationResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %d)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 enable);

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  contact->retain_nonstatus_information = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the contact notification command for host events.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  command_id        Command's informations.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnHostCommand(soap* s,
							    ns1__contactIDType* contact_id,
							    ns1__commandIDType* command_id,
							    centreonscheduler__contactSetNotificationsOnHostCommandResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 command_id->command.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] contact->host_notification_commands->cmd;
  contact->host_notification_commands->cmd = my_strdup(command_id->command.c_str());
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the service notification command.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  contact_id        Contact to set data.
 *  @param[in]  command_id        Command's informations.
 *  @param[out] res               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__contactSetNotificationsOnServiceCommand(soap* s,
							       ns1__contactIDType* contact_id,
							       ns1__commandIDType* command_id,
							       centreonscheduler__contactSetNotificationsOnServiceCommandResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2,
		 "Webservice: %s(%s, %s)\n",
		 __func__,
		 contact_id->contact.c_str(),
		 command_id->command.c_str());

  contact* contact = find_contact(contact_id->contact.c_str());
  if (contact == NULL) {
    res.error->code = 1;
    res.error->message = "Contact `" + contact_id->contact + "' not found.";

    log_debug_info(DEBUGL_COMMANDS, 2,
		   "Webservice: %s failed. %s\n",
		   __func__,
		   res.error->message.c_str());
    return (SOAP_OK);
  }

  delete[] contact->service_notification_commands->cmd;
  contact->service_notification_commands->cmd = my_strdup(command_id->command.c_str());
  res.error->code = 0;

  return (SOAP_OK);
}
