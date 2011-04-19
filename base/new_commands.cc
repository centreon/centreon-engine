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

#include <sstream>
#include <string>
#include <string.h>
#include <strings.h>
#include "centreonengine.nsmap" // gSOAP namespaces.
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
 *  Restart Engine.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__processRestart(soap* s,
				   centreonengine__processRestartResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_FUNCTIONS, 2, "Webservice: %s()\n", __func__);
  sigrestart = true;
  res.error->code = 0;
  logit(NSLOG_PROCESS_INFO, true, "Webservice: program restarting...\n");

  return (SOAP_OK);
}

/**
 *  Shutdown Engine.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__processShutdown(soap* s,
				    centreonengine__processShutdownResponse& res) {
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
int centreonengine__stateInformationLoad(soap* s,
					 centreonengine__stateInformationLoadResponse& res) {
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
int centreonengine__stateInformationSave(soap* s,
					 centreonengine__stateInformationSaveResponse& res) {
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
int centreonengine__hostGetAcknowledgementIsSet(soap* s,
						ns1__hostIDType* host_id,
						centreonengine__hostGetAcknowledgementIsSetResponse& res) {
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
int centreonengine__hostGetAcknowledgementType(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetAcknowledgementTypeResponse& res) {
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
int centreonengine__hostGetAddress(soap* s,
				   ns1__hostIDType* host_id,
				   centreonengine__hostGetAddressResponse& res) {
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
int centreonengine__hostSetAddress(soap* s,
				   ns1__hostIDType* host_id,
				   std::string address,
				   centreonengine__hostSetAddressResponse& res) {
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
int centreonengine__hostGetCheckActiveEnabled(soap* s,
					      ns1__hostIDType* host_id,
					      centreonengine__hostGetCheckActiveEnabledResponse& res) {
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
int centreonengine__hostGetCheckCommand(soap* s,
					ns1__hostIDType* host_id,
					centreonengine__hostGetCheckCommandResponse& res) {
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
int centreonengine__hostGetCheckCurrentAttempt(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetCheckCurrentAttemptResponse& res) {
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
int centreonengine__hostGetCheckIntervalNormal(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetCheckIntervalNormalResponse& res) {
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
int centreonengine__hostGetCheckIntervalRetry(soap* s,
					      ns1__hostIDType* host_id,
					      centreonengine__hostGetCheckIntervalRetryResponse& res) {
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
int centreonengine__hostGetCheckLast(soap* s,
				     ns1__hostIDType* host_id,
				     centreonengine__hostGetCheckLastResponse& res) {
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
int centreonengine__hostGetCheckMaxAttempts(soap* s,
					    ns1__hostIDType* host_id,
					    centreonengine__hostGetCheckMaxAttemptsResponse& res) {
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
int centreonengine__hostGetCheckNext(soap* s,
				     ns1__hostIDType* host_id,
				     centreonengine__hostGetCheckNextResponse& res) {
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
int centreonengine__hostGetCheckOptions(soap* s,
					ns1__hostIDType* host_id,
					centreonengine__hostGetCheckOptionsResponse& res) {
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
int centreonengine__hostGetCheckPassiveEnabled(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetCheckPassiveEnabledResponse& res) {
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
int centreonengine__hostGetCheckPeriod(soap* s,
				       ns1__hostIDType* host_id,
				       centreonengine__hostGetCheckPeriodResponse& res) {
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
int centreonengine__hostGetCheckShouldBeScheduled(soap* s,
						  ns1__hostIDType* host_id,
						  centreonengine__hostGetCheckShouldBeScheduledResponse& res) {
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
int centreonengine__hostGetCheckType(soap* s,
				     ns1__hostIDType* host_id,
				     centreonengine__hostGetCheckTypeResponse& res) {
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
int centreonengine__hostSetCheckActiveEnabled(soap* s,
					      ns1__hostIDType* host_id,
					      bool enable,
					      centreonengine__hostSetCheckActiveEnabledResponse& res) {
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
int centreonengine__hostSetCheckCommand(soap* s,
					ns1__hostIDType* host_id,
					std::string command,
					centreonengine__hostSetCheckCommandResponse& res) {
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
int centreonengine__hostSetCheckIntervalNormal(soap* s,
					       ns1__hostIDType* host_id,
					       unsigned int interval,
					       centreonengine__hostSetCheckIntervalNormalResponse& res) {
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
int centreonengine__hostSetCheckIntervalRetry(soap* s,
					      ns1__hostIDType* host_id,
					      unsigned int interval,
					      centreonengine__hostSetCheckIntervalRetryResponse& res) {
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
int centreonengine__hostSetCheckMaxAttempts(soap* s,
					    ns1__hostIDType* host_id,
					    unsigned int attempts,
					    centreonengine__hostSetCheckMaxAttemptsResponse& res) {
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
int centreonengine__hostSetCheckPassiveEnabled(soap* s,
					       ns1__hostIDType* host_id,
					       bool enable,
					       centreonengine__hostSetCheckPassiveEnabledResponse& res) {
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
int centreonengine__hostGetCircularPathChecked(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetCircularPathCheckedResponse& res) {
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
int centreonengine__hostGetCircularPathHas(soap* s,
					   ns1__hostIDType* host_id,
					   centreonengine__hostGetCircularPathHasResponse& res) {
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
int centreonengine__hostGetDowntimeDepth(soap* s,
					 ns1__hostIDType* host_id,
					 centreonengine__hostGetDowntimeDepthResponse& res) {
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
int centreonengine__hostGetDowntimeFlexPending(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetDowntimeFlexPendingResponse& res) {
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
int centreonengine__hostGetEventHandler(soap* s,
					ns1__hostIDType* host_id,
					centreonengine__hostGetEventHandlerResponse& res) {
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
int centreonengine__hostGetEventHandlerEnabled(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetEventHandlerEnabledResponse& res) {
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
int centreonengine__hostSetEventHandler(soap* s,
					ns1__hostIDType* host_id,
					std::string event_handler,
					centreonengine__hostSetEventHandlerResponse& res) {
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
int centreonengine__hostSetEventHandlerEnabled(soap* s,
					       ns1__hostIDType* host_id,
					       bool enable,
					       centreonengine__hostSetEventHandlerEnabledResponse& res) {
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
int centreonengine__hostGetFailurePredictionEnabled(soap* s,
						    ns1__hostIDType* host_id,
						    centreonengine__hostGetFailurePredictionEnabledResponse& res) {
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
int centreonengine__hostGetFailurePredictionOptions(soap* s,
						    ns1__hostIDType* host_id,
						    centreonengine__hostGetFailurePredictionOptionsResponse& res) {
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
int centreonengine__hostSetFailurePredictionEnabled(soap* s,
						    ns1__hostIDType* host_id,
						    bool enable,
						    centreonengine__hostSetFailurePredictionEnabledResponse& res) {
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
int centreonengine__hostGetFlapDetectionCommentID(soap* s,
						  ns1__hostIDType* host_id,
						  centreonengine__hostGetFlapDetectionCommentIDResponse& res) {
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
int centreonengine__hostGetFlapDetectionEnabled(soap* s,
						ns1__hostIDType* host_id,
						centreonengine__hostGetFlapDetectionEnabledResponse& res) {
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
int centreonengine__hostGetFlapDetectionIsFlapping(soap* s,
						   ns1__hostIDType* host_id,
						   centreonengine__hostGetFlapDetectionIsFlappingResponse& res) {
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
int centreonengine__hostGetFlapDetectionOnDown(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetFlapDetectionOnDownResponse& res) {
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
int centreonengine__hostGetFlapDetectionOnUnreachable(soap* s,
						      ns1__hostIDType* host_id,
						      centreonengine__hostGetFlapDetectionOnUnreachableResponse& res) {
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
int centreonengine__hostGetFlapDetectionOnUp(soap* s,
					     ns1__hostIDType* host_id,
					     centreonengine__hostGetFlapDetectionOnUpResponse& res) {
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
int centreonengine__hostGetFlapDetectionStateHistoryLastUpdate(soap* s,
							       ns1__hostIDType* host_id,
							       centreonengine__hostGetFlapDetectionStateHistoryLastUpdateResponse& res) {
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
int centreonengine__hostGetFlapDetectionThresholdHigh(soap* s,
						      ns1__hostIDType* host_id,
						      centreonengine__hostGetFlapDetectionThresholdHighResponse& res) {
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
int centreonengine__hostGetFlapDetectionThresholdLow(soap* s,
						     ns1__hostIDType* host_id,
						     centreonengine__hostGetFlapDetectionThresholdLowResponse& res) {
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
int centreonengine__hostSetFlapDetectionEnabled(soap* s,
						ns1__hostIDType* host_id,
						bool enable,
						centreonengine__hostSetFlapDetectionEnabledResponse& res) {
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
int centreonengine__hostSetFlapDetectionOnDown(soap* s,
					       ns1__hostIDType* host_id,
					       bool enable,
					       centreonengine__hostSetFlapDetectionOnDownResponse& res) {
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
int centreonengine__hostSetFlapDetectionOnUnreachable(soap* s,
						      ns1__hostIDType* host_id,
						      bool enable,
						      centreonengine__hostSetFlapDetectionOnUnreachableResponse& res) {
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
int centreonengine__hostSetFlapDetectionOnUp(soap* s,
					     ns1__hostIDType* host_id,
					     bool enable,
					     centreonengine__hostSetFlapDetectionOnUpResponse& res) {
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
int centreonengine__hostSetFlapDetectionThresholdHigh(soap* s,
						      ns1__hostIDType* host_id,
						      double threshold,
						      centreonengine__hostSetFlapDetectionThresholdHighResponse& res) {
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
int centreonengine__hostSetFlapDetectionThresholdLow(soap* s,
						     ns1__hostIDType* host_id,
						     double threshold,
						     centreonengine__hostSetFlapDetectionThresholdLowResponse& res) {
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
int centreonengine__hostGetFreshnessCheckEnabled(soap* s,
						 ns1__hostIDType* host_id,
						 centreonengine__hostGetFreshnessCheckEnabledResponse& res) {
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
int centreonengine__hostGetFreshnessIsActive(soap* s,
					     ns1__hostIDType* host_id,
					     centreonengine__hostGetFreshnessIsActiveResponse& res) {
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
int centreonengine__hostGetFreshnessThreshold(soap* s,
					      ns1__hostIDType* host_id,
					      centreonengine__hostGetFreshnessThresholdResponse& res) {
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
int centreonengine__hostSetFreshnessCheckEnabled(soap* s,
						 ns1__hostIDType* host_id,
						 bool enable,
						 centreonengine__hostSetFreshnessCheckEnabledResponse& res) {
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
int centreonengine__hostSetFreshnessThreshold(soap* s,
					      ns1__hostIDType* host_id,
					      int threshold,
					      centreonengine__hostSetFreshnessThresholdResponse& res) {
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
int centreonengine__hostGetModifiedAttributes(soap* s,
					      ns1__hostIDType* host_id,
					      centreonengine__hostGetModifiedAttributesResponse& res) {
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
int centreonengine__hostGetNameAlias(soap* s,
				     ns1__hostIDType* host_id,
				     centreonengine__hostGetNameAliasResponse& res) {
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
int centreonengine__hostGetNameDisplay(soap* s,
				       ns1__hostIDType* host_id,
				       centreonengine__hostGetNameDisplayResponse& res) {
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
int centreonengine__hostSetNameAlias(soap* s,
				     ns1__hostIDType* host_id,
				     std::string alias,
				     centreonengine__hostSetNameAliasResponse& res) {
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
int centreonengine__hostSetNameDisplay(soap* s,
				       ns1__hostIDType* host_id,
				       std::string displayname,
				       centreonengine__hostSetNameDisplayResponse& res) {
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
int centreonengine__hostGetNotificationsCurrentID(soap* s,
						  ns1__hostIDType* host_id,
						  centreonengine__hostGetNotificationsCurrentIDResponse& res) {
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
int centreonengine__hostGetNotificationsCurrentNumber(soap* s,
						      ns1__hostIDType* host_id,
						      centreonengine__hostGetNotificationsCurrentNumberResponse& res) {
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
int centreonengine__hostGetNotificationsEnabled(soap* s,
						ns1__hostIDType* host_id,
						centreonengine__hostGetNotificationsEnabledResponse& res) {
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
int centreonengine__hostGetNotificationsFirstDelay(soap* s,
						   ns1__hostIDType* host_id,
						   centreonengine__hostGetNotificationsFirstDelayResponse& res) {
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
int centreonengine__hostGetNotificationsInterval(soap* s,
						 ns1__hostIDType* host_id,
						 centreonengine__hostGetNotificationsIntervalResponse& res) {
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
int centreonengine__hostGetNotificationsLast(soap* s,
					     ns1__hostIDType* host_id,
					     centreonengine__hostGetNotificationsLastResponse& res) {
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
int centreonengine__hostGetNotificationsNext(soap* s,
					     ns1__hostIDType* host_id,
					     centreonengine__hostGetNotificationsNextResponse& res) {
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
int centreonengine__hostGetNotificationsOnDown(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetNotificationsOnDownResponse& res) {
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
int centreonengine__hostGetNotificationsOnDowntime(soap* s,
						   ns1__hostIDType* host_id,
						   centreonengine__hostGetNotificationsOnDowntimeResponse& res) {
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
int centreonengine__hostGetNotificationsOnFlapping(soap* s,
						   ns1__hostIDType* host_id,
						   centreonengine__hostGetNotificationsOnFlappingResponse& res) {
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
int centreonengine__hostGetNotificationsOnRecovery(soap* s,
						   ns1__hostIDType* host_id,
						   centreonengine__hostGetNotificationsOnRecoveryResponse& res) {
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
int centreonengine__hostGetNotificationsOnUnreachable(soap* s,
						      ns1__hostIDType* host_id,
						      centreonengine__hostGetNotificationsOnUnreachableResponse& res) {
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
int centreonengine__hostGetNotificationsPeriod(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetNotificationsPeriodResponse& res) {
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
int centreonengine__hostSetNotificationsEnabled(soap* s,
						ns1__hostIDType* host_id,
						bool enable,
						centreonengine__hostSetNotificationsEnabledResponse& res) {
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
int centreonengine__hostSetNotificationsFirstDelay(soap* s,
						   ns1__hostIDType* host_id,
						   unsigned int delay,
						   centreonengine__hostSetNotificationsFirstDelayResponse& res) {
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
int centreonengine__hostSetNotificationsInterval(soap* s,
						 ns1__hostIDType* host_id,
						 unsigned int interval,
						 centreonengine__hostSetNotificationsIntervalResponse& res) {
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
int centreonengine__hostSetNotificationsOnDown(soap* s,
					       ns1__hostIDType* host_id,
					       bool enable,
					       centreonengine__hostSetNotificationsOnDownResponse& res) {
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
int centreonengine__hostSetNotificationsOnDowntime(soap* s,
						   ns1__hostIDType* host_id,
						   bool enable,
						   centreonengine__hostSetNotificationsOnDowntimeResponse& res) {
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
int centreonengine__hostSetNotificationsOnFlapping(soap* s,
						   ns1__hostIDType* host_id,
						   bool enable,
						   centreonengine__hostSetNotificationsOnFlappingResponse& res) {
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
int centreonengine__hostSetNotificationsOnRecovery(soap* s,
						   ns1__hostIDType* host_id,
						   bool enable,
						   centreonengine__hostSetNotificationsOnRecoveryResponse& res) {
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
int centreonengine__hostSetNotificationsOnUnreachable(soap* s,
						      ns1__hostIDType* host_id,
						      bool enable,
						      centreonengine__hostSetNotificationsOnUnreachableResponse& res) {
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
int centreonengine__hostGetObsessOver(soap* s,
				      ns1__hostIDType* host_id,
				      centreonengine__hostGetObsessOverResponse& res) {
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
int centreonengine__hostSetObsessOver(soap* s,
				      ns1__hostIDType* host_id,
				      bool enable,
				      centreonengine__hostSetObsessOverResponse& res) {
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
int centreonengine__hostGetPerfdataProcessingEnabled(soap* s,
						     ns1__hostIDType* host_id,
						     centreonengine__hostGetPerfdataProcessingEnabledResponse& res) {
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
int centreonengine__hostSetPerfdataProcessingEnabled(soap* s,
						     ns1__hostIDType* host_id,
						     bool enable,
						     centreonengine__hostSetPerfdataProcessingEnabledResponse& res) {
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
int centreonengine__hostGetPluginExecutionTime(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetPluginExecutionTimeResponse& res) {
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
int centreonengine__hostGetPluginIsExecuting(soap* s,
					     ns1__hostIDType* host_id,
					     centreonengine__hostGetPluginIsExecutingResponse& res) {
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
int centreonengine__hostGetPluginLatency(soap* s,
					 ns1__hostIDType* host_id,
					 centreonengine__hostGetPluginLatencyResponse& res) {
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
int centreonengine__hostGetPluginOutput(soap* s,
					ns1__hostIDType* host_id,
					centreonengine__hostGetPluginOutputResponse& res) {
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
int centreonengine__hostGetPluginPerfdata(soap* s,
					  ns1__hostIDType* host_id,
					  centreonengine__hostGetPluginPerfdataResponse& res) {
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
int centreonengine__hostGetRetainStatusInformation(soap* s,
						   ns1__hostIDType* host_id,
						   centreonengine__hostGetRetainStatusInformationResponse& res) {
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
int centreonengine__hostGetRetainNonStatusInformation(soap* s,
						      ns1__hostIDType* host_id,
						      centreonengine__hostGetRetainNonStatusInformationResponse& res) {
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
int centreonengine__hostSetRetainStatusInformation(soap* s,
						   ns1__hostIDType* host_id,
						   bool enable,
						   centreonengine__hostSetRetainStatusInformationResponse& res) {
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
int centreonengine__hostSetRetainNonStatusInformation(soap* s,
						      ns1__hostIDType* host_id,
						      bool enable,
						      centreonengine__hostSetRetainNonStatusInformationResponse& res) {
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
int centreonengine__hostGetServicesCount(soap* s,
					 ns1__hostIDType* host_id,
					 centreonengine__hostGetServicesCountResponse& res) {
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
int centreonengine__hostGetServicesTotalCheckInterval(soap* s,
						      ns1__hostIDType* host_id,
						      centreonengine__hostGetServicesTotalCheckIntervalResponse& res) {
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
int centreonengine__hostGetStalkOnDown(soap* s,
				       ns1__hostIDType* host_id,
				       centreonengine__hostGetStalkOnDownResponse& res) {
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
int centreonengine__hostGetStalkOnUnreachable(soap* s,
					      ns1__hostIDType* host_id,
					      centreonengine__hostGetStalkOnUnreachableResponse& res) {
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
int centreonengine__hostGetStalkOnUp(soap* s,
				     ns1__hostIDType* host_id,
				     centreonengine__hostGetStalkOnUpResponse& res) {
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
int centreonengine__hostSetStalkOnDown(soap* s,
				       ns1__hostIDType* host_id,
				       bool enable,
				       centreonengine__hostSetStalkOnDownResponse& res) {
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
int centreonengine__hostSetStalkOnUnreachable(soap* s,
					      ns1__hostIDType* host_id,
					      bool enable,
					      centreonengine__hostSetStalkOnUnreachableResponse& res) {
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
int centreonengine__hostSetStalkOnUp(soap* s,
				     ns1__hostIDType* host_id,
				     bool enable,
				     centreonengine__hostSetStalkOnUpResponse& res) {
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
int centreonengine__hostGetStateCurrent(soap* s,
					ns1__hostIDType* host_id,
					centreonengine__hostGetStateCurrentResponse& res) {
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
int centreonengine__hostGetStateInitial(soap* s,
					ns1__hostIDType* host_id,
					centreonengine__hostGetStateInitialResponse& res) {
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
int centreonengine__hostGetStateLast(soap* s,
				     ns1__hostIDType* host_id,
				     centreonengine__hostGetStateLastResponse& res) {
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
int centreonengine__hostGetStateLastChange(soap* s,
					   ns1__hostIDType* host_id,
					   centreonengine__hostGetStateLastChangeResponse& res) {
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
int centreonengine__hostGetStateLastDown(soap* s,
					 ns1__hostIDType* host_id,
					 centreonengine__hostGetStateLastDownResponse& res) {
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
int centreonengine__hostGetStateLastHard(soap* s,
					 ns1__hostIDType* host_id,
					 centreonengine__hostGetStateLastHardResponse& res) {
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
int centreonengine__hostGetStateLastHardChange(soap* s,
					       ns1__hostIDType* host_id,
					       centreonengine__hostGetStateLastHardChangeResponse& res) {
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
int centreonengine__hostGetStateLastUnreachable(soap* s,
						ns1__hostIDType* host_id,
						centreonengine__hostGetStateLastUnreachableResponse& res) {
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
int centreonengine__hostGetStateLastUp(soap* s,
				       ns1__hostIDType* host_id,
				       centreonengine__hostGetStateLastUpResponse& res) {
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
int centreonengine__hostGetStatePercentChange(soap* s,
					      ns1__hostIDType* host_id,
					      centreonengine__hostGetStatePercentChangeResponse& res) {
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
int centreonengine__hostGetStateType(soap* s,
				     ns1__hostIDType* host_id,
				     centreonengine__hostGetStateTypeResponse& res) {
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
int centreonengine__hostSetNotificationsBeyondEnabled(soap* s,
						      ns1__hostIDType* host_id,
						      bool enable,
						      centreonengine__hostSetNotificationsBeyondEnabledResponse& res) {
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
int centreonengine__hostSetNotificationsOnSelfAndChildrenEnabled(soap* s,
								 ns1__hostIDType* host_id,
								 bool enable,
								 centreonengine__hostSetNotificationsOnSelfAndChildrenEnabledResponse& res) {
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
int centreonengine__hostSetNotificationsPeriod(soap* s,
					       ns1__hostIDType* host_id,
					       ns1__timeperiodIDType* timeperiod_id,
					       centreonengine__hostSetNotificationsPeriodResponse& res) {
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
int centreonengine__serviceGetAcknowledgementIsSet(soap* s,
						   ns1__serviceIDType* service_id,
						   centreonengine__serviceGetAcknowledgementIsSetResponse& res) {
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
int centreonengine__serviceGetAcknowledgementType(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonengine__serviceGetAcknowledgementTypeResponse& res) {
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
int centreonengine__serviceGetCheckActiveEnabled(soap* s,
						 ns1__serviceIDType* service_id,
						 centreonengine__serviceGetCheckActiveEnabledResponse& res) {
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
int centreonengine__serviceGetCheckCommand(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonengine__serviceGetCheckCommandResponse& res) {
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
int centreonengine__serviceGetCheckCurrentAttempt(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonengine__serviceGetCheckCurrentAttemptResponse& res) {
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
int centreonengine__serviceGetCheckIntervalNormal(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonengine__serviceGetCheckIntervalNormalResponse& res) {
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
int centreonengine__serviceGetCheckIntervalRetry(soap* s,
						 ns1__serviceIDType* service_id,
						 centreonengine__serviceGetCheckIntervalRetryResponse& res) {
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
int centreonengine__serviceGetCheckLast(soap* s,
					ns1__serviceIDType* service_id,
					centreonengine__serviceGetCheckLastResponse& res) {
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
int centreonengine__serviceGetCheckMaxAttempts(soap* s,
					       ns1__serviceIDType* service_id,
					       centreonengine__serviceGetCheckMaxAttemptsResponse& res) {
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
int centreonengine__serviceGetCheckNext(soap* s,
					ns1__serviceIDType* service_id,
					centreonengine__serviceGetCheckNextResponse& res) {
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
int centreonengine__serviceGetCheckOptions(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonengine__serviceGetCheckOptionsResponse& res) {
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
int centreonengine__serviceGetCheckPassiveEnabled(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonengine__serviceGetCheckPassiveEnabledResponse& res) {
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
int centreonengine__serviceGetCheckPeriod(soap* s,
					  ns1__serviceIDType* service_id,
					  centreonengine__serviceGetCheckPeriodResponse& res) {
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
int centreonengine__serviceGetCheckShouldBeScheduled(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonengine__serviceGetCheckShouldBeScheduledResponse& res) {
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
int centreonengine__serviceGetCheckType(soap* s,
					ns1__serviceIDType* service_id,
					centreonengine__serviceGetCheckTypeResponse& res) {
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
int centreonengine__serviceSetCheckActiveEnabled(soap* s,
						 ns1__serviceIDType* service_id,
						 bool enable,
						 centreonengine__serviceSetCheckActiveEnabledResponse& res) {
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
int centreonengine__serviceSetCheckCommand(soap* s,
					   ns1__serviceIDType* service_id,
					   std::string command,
					   centreonengine__serviceSetCheckCommandResponse& res) {
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
int centreonengine__serviceSetCheckIntervalNormal(soap* s,
						  ns1__serviceIDType* service_id,
						  unsigned int interval,
						  centreonengine__serviceSetCheckIntervalNormalResponse& res) {
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
int centreonengine__serviceSetCheckIntervalRetry(soap* s,
						 ns1__serviceIDType* service_id,
						 unsigned int interval,
						 centreonengine__serviceSetCheckIntervalRetryResponse& res) {
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
int centreonengine__serviceSetCheckMaxAttempts(soap* s,
					       ns1__serviceIDType* service_id,
					       unsigned int attempts,
					       centreonengine__serviceSetCheckMaxAttemptsResponse& res) {
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
int centreonengine__serviceSetCheckPassiveEnabled(soap* s,
						  ns1__serviceIDType* service_id,
						  bool enable,
						  centreonengine__serviceSetCheckPassiveEnabledResponse& res) {
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
int centreonengine__serviceGetCustomVariable(soap* s,
					     ns1__serviceIDType* service_id,
					     std::string variable,
					     centreonengine__serviceGetCustomVariableResponse& res) {
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
int centreonengine__serviceGetDowntimeDepth(soap* s,
					    ns1__serviceIDType* service_id,
					    centreonengine__serviceGetDowntimeDepthResponse& res) {
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
int centreonengine__serviceGetDowntimeFlexPending(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonengine__serviceGetDowntimeFlexPendingResponse& res) {
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
int centreonengine__serviceGetEventHandler(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonengine__serviceGetEventHandlerResponse& res) {
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
int centreonengine__serviceGetEventHandlerEnabled(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonengine__serviceGetEventHandlerEnabledResponse& res) {
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
int centreonengine__serviceSetEventHandler(soap* s,
					   ns1__serviceIDType* service_id,
					   std::string event_handler,
					   centreonengine__serviceSetEventHandlerResponse& res) {
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
int centreonengine__serviceSetEventHandlerEnabled(soap* s,
						  ns1__serviceIDType* service_id,
						  bool enable,
						  centreonengine__serviceSetEventHandlerEnabledResponse& res) {
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
int centreonengine__serviceGetFailurePredictionEnabled(soap* s,
						       ns1__serviceIDType* service_id,
						       centreonengine__serviceGetFailurePredictionEnabledResponse& res) {
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
int centreonengine__serviceGetFailurePredictionOptions(soap* s,
						       ns1__serviceIDType* service_id,
						       centreonengine__serviceGetFailurePredictionOptionsResponse& res) {
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
int centreonengine__serviceSetFailurePredictionEnabled(soap* s,
						       ns1__serviceIDType* service_id,
						       bool enable,
						       centreonengine__serviceSetFailurePredictionEnabledResponse& res) {
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
int centreonengine__serviceGetFlapDetectionCommentID(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonengine__serviceGetFlapDetectionCommentIDResponse& res) {
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
int centreonengine__serviceGetFlapDetectionEnabled(soap* s,
						   ns1__serviceIDType* service_id,
						   centreonengine__serviceGetFlapDetectionEnabledResponse& res) {
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
int centreonengine__serviceGetFlapDetectionIsFlapping(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonengine__serviceGetFlapDetectionIsFlappingResponse& res) {
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
int centreonengine__serviceGetFlapDetectionOnCritical(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonengine__serviceGetFlapDetectionOnCriticalResponse& res) {
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
int centreonengine__serviceGetFlapDetectionOnOk(soap* s,
						ns1__serviceIDType* service_id,
						centreonengine__serviceGetFlapDetectionOnOkResponse& res) {
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
int centreonengine__serviceGetFlapDetectionOnUnknown(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonengine__serviceGetFlapDetectionOnUnknownResponse& res) {
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
int centreonengine__serviceGetFlapDetectionOnWarning(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonengine__serviceGetFlapDetectionOnWarningResponse& res) {
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
int centreonengine__serviceGetFlapDetectionThresholdHigh(soap* s,
							 ns1__serviceIDType* service_id,
							 centreonengine__serviceGetFlapDetectionThresholdHighResponse& res) {
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
int centreonengine__serviceGetFlapDetectionThresholdLow(soap* s,
							ns1__serviceIDType* service_id,
							centreonengine__serviceGetFlapDetectionThresholdLowResponse& res) {
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
int centreonengine__serviceSetFlapDetectionEnabled(soap* s,
						   ns1__serviceIDType* service_id,
						   bool enable,
						   centreonengine__serviceSetFlapDetectionEnabledResponse& res) {
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
int centreonengine__serviceSetFlapDetectionOnCritical(soap* s,
						      ns1__serviceIDType* service_id,
						      bool enable,
						      centreonengine__serviceSetFlapDetectionOnCriticalResponse& res) {
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
int centreonengine__serviceSetFlapDetectionOnOk(soap* s,
						ns1__serviceIDType* service_id,
						bool enable,
						centreonengine__serviceSetFlapDetectionOnOkResponse& res) {
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
int centreonengine__serviceSetFlapDetectionOnUnknown(soap* s,
						     ns1__serviceIDType* service_id,
						     bool enable,
						     centreonengine__serviceSetFlapDetectionOnUnknownResponse& res) {
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
int centreonengine__serviceSetFlapDetectionOnWarning(soap* s,
						     ns1__serviceIDType* service_id,
						     bool enable,
						     centreonengine__serviceSetFlapDetectionOnWarningResponse& res) {
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
int centreonengine__serviceSetFlapDetectionThresholdHigh(soap* s,
							 ns1__serviceIDType* service_id,
							 double threshold,
							 centreonengine__serviceSetFlapDetectionThresholdHighResponse& res) {
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
int centreonengine__serviceSetFlapDetectionThresholdLow(soap* s,
							ns1__serviceIDType* service_id,
							double threshold,
							centreonengine__serviceSetFlapDetectionThresholdLowResponse& res) {
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
int centreonengine__serviceGetFreshnessCheckEnabled(soap* s,
						    ns1__serviceIDType* service_id,
						    centreonengine__serviceGetFreshnessCheckEnabledResponse& res) {
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
int centreonengine__serviceGetFreshnessIsActive(soap* s,
						ns1__serviceIDType* service_id,
						centreonengine__serviceGetFreshnessIsActiveResponse& res) {
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
int centreonengine__serviceGetFreshnessThreshold(soap* s,
						 ns1__serviceIDType* service_id,
						 centreonengine__serviceGetFreshnessThresholdResponse& res) {
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
int centreonengine__serviceSetFreshnessCheckEnabled(soap* s,
						    ns1__serviceIDType* service_id,
						    bool enable,
						    centreonengine__serviceSetFreshnessCheckEnabledResponse& res) {
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
int centreonengine__serviceSetFreshnessThreshold(soap* s,
						 ns1__serviceIDType* service_id,
						 int threshold,
						 centreonengine__serviceSetFreshnessThresholdResponse& res) {
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
int centreonengine__serviceGetModifiedAttributes(soap* s,
						 ns1__serviceIDType* service_id,
						 centreonengine__serviceGetModifiedAttributesResponse& res) {
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
int centreonengine__serviceGetNameDisplay(soap* s,
					  ns1__serviceIDType* service_id,
					  centreonengine__serviceGetNameDisplayResponse& res) {
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
int centreonengine__serviceSetNameDisplay(soap* s,
					  ns1__serviceIDType* service_id,
					  std::string name,
					  centreonengine__serviceSetNameDisplayResponse& res) {
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
int centreonengine__serviceGetNotificationsCurrentID(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonengine__serviceGetNotificationsCurrentIDResponse& res) {
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
int centreonengine__serviceGetNotificationsCurrentNumber(soap* s,
							 ns1__serviceIDType* service_id,
							 centreonengine__serviceGetNotificationsCurrentNumberResponse& res) {
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
int centreonengine__serviceGetNotificationsEnabled(soap* s,
						   ns1__serviceIDType* service_id,
						   centreonengine__serviceGetNotificationsEnabledResponse& res) {
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
int centreonengine__serviceGetNotificationsFirstDelay(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonengine__serviceGetNotificationsFirstDelayResponse& res) {
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
int centreonengine__serviceGetNotificationsInterval(soap* s,
						    ns1__serviceIDType* service_id,
						    centreonengine__serviceGetNotificationsIntervalResponse& res) {
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
int centreonengine__serviceGetNotificationsLast(soap* s,
						ns1__serviceIDType* service_id,
						centreonengine__serviceGetNotificationsLastResponse& res) {
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
int centreonengine__serviceGetNotificationsNext(soap* s,
						ns1__serviceIDType* service_id,
						centreonengine__serviceGetNotificationsNextResponse& res) {
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
int centreonengine__serviceGetNotificationsOnCritical(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonengine__serviceGetNotificationsOnCriticalResponse& res) {
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
int centreonengine__serviceGetNotificationsOnDowntime(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonengine__serviceGetNotificationsOnDowntimeResponse& res) {
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
int centreonengine__serviceGetNotificationsOnFlapping(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonengine__serviceGetNotificationsOnFlappingResponse& res) {
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
int centreonengine__serviceGetNotificationsOnRecovery(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonengine__serviceGetNotificationsOnRecoveryResponse& res) {
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
int centreonengine__serviceGetNotificationsOnUnknown(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonengine__serviceGetNotificationsOnUnknownResponse& res) {
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
int centreonengine__serviceGetNotificationsOnWarning(soap* s,
						     ns1__serviceIDType* service_id,
						     centreonengine__serviceGetNotificationsOnWarningResponse& res) {
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
int centreonengine__serviceGetNotificationsPeriod(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonengine__serviceGetNotificationsPeriodResponse& res) {
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
int centreonengine__serviceSetNotificationsEnabled(soap* s,
						   ns1__serviceIDType* service_id,
						   bool enable,
						   centreonengine__serviceSetNotificationsEnabledResponse& res) {
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
int centreonengine__serviceSetNotificationsFirstDelay(soap* s,
						      ns1__serviceIDType* service_id,
						      unsigned int delay,
						      centreonengine__serviceSetNotificationsFirstDelayResponse& res) {
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
int centreonengine__serviceSetNotificationsInterval(soap* s,
						    ns1__serviceIDType* service_id,
						    unsigned int interval,
						    centreonengine__serviceSetNotificationsIntervalResponse& res) {
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
int centreonengine__serviceSetNotificationsOnCritical(soap* s,
						      ns1__serviceIDType* service_id,
						      bool enable,
						      centreonengine__serviceSetNotificationsOnCriticalResponse& res) {
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
int centreonengine__serviceSetNotificationsOnDowntime(soap* s,
						      ns1__serviceIDType* service_id,
						      bool enable,
						      centreonengine__serviceSetNotificationsOnDowntimeResponse& res) {
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
int centreonengine__serviceSetNotificationsOnFlapping(soap* s,
						      ns1__serviceIDType* service_id,
						      bool enable,
						      centreonengine__serviceSetNotificationsOnFlappingResponse& res) {
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
int centreonengine__serviceSetNotificationsOnRecovery(soap* s,
						      ns1__serviceIDType* service_id,
						      bool enable,
						      centreonengine__serviceSetNotificationsOnRecoveryResponse& res) {
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
int centreonengine__serviceSetNotificationsOnUnknown(soap* s,
						     ns1__serviceIDType* service_id,
						     bool enable,
						     centreonengine__serviceSetNotificationsOnUnknownResponse& res) {
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
int centreonengine__serviceSetNotificationsOnWarning(soap* s,
						     ns1__serviceIDType* service_id,
						     bool enable,
						     centreonengine__serviceSetNotificationsOnWarningResponse& res) {
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
int centreonengine__serviceGetObsessOver(soap* s,
					 ns1__serviceIDType* service_id,
					 centreonengine__serviceGetObsessOverResponse& res) {
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
int centreonengine__serviceSetObsessOver(soap* s,
					 ns1__serviceIDType* service_id,
					 bool enable,
					 centreonengine__serviceSetObsessOverResponse& res) {
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
int centreonengine__serviceGetPerfdataProcessingEnabled(soap* s,
							ns1__serviceIDType* service_id,
							centreonengine__serviceGetPerfdataProcessingEnabledResponse& res) {
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
int centreonengine__serviceSetPerfdataProcessingEnabled(soap* s,
							ns1__serviceIDType* service_id,
							bool enable,
							centreonengine__serviceSetPerfdataProcessingEnabledResponse& res) {
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
int centreonengine__serviceGetPluginExecutionTime(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonengine__serviceGetPluginExecutionTimeResponse& res) {
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
int centreonengine__serviceGetPluginIsExecuting(soap* s,
						ns1__serviceIDType* service_id,
						centreonengine__serviceGetPluginIsExecutingResponse& res) {
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
int centreonengine__serviceGetPluginLatency(soap* s,
					    ns1__serviceIDType* service_id,
					    centreonengine__serviceGetPluginLatencyResponse& res) {
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
int centreonengine__serviceGetPluginOutput(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonengine__serviceGetPluginOutputResponse& res) {
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
int centreonengine__serviceGetPluginPerfdata(soap* s,
					     ns1__serviceIDType* service_id,
					     centreonengine__serviceGetPluginPerfdataResponse& res) {
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
int centreonengine__serviceGetRetainStatusInformation(soap* s,
						      ns1__serviceIDType* service_id,
						      centreonengine__serviceGetRetainStatusInformationResponse& res) {
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
int centreonengine__serviceGetRetainNonStatusInformation(soap* s,
							 ns1__serviceIDType* service_id,
							 centreonengine__serviceGetRetainNonStatusInformationResponse& res) {
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
int centreonengine__serviceSetRetainStatusInformation(soap* s,
						      ns1__serviceIDType* service_id,
						      bool enable,
						      centreonengine__serviceSetRetainStatusInformationResponse& res) {
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
int centreonengine__serviceSetRetainNonStatusInformation(soap* s,
							 ns1__serviceIDType* service_id,
							 bool enable,
							 centreonengine__serviceSetRetainNonStatusInformationResponse& res) {
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
int centreonengine__serviceGetStalkOnCritical(soap* s,
					      ns1__serviceIDType* service_id,
					      centreonengine__serviceGetStalkOnCriticalResponse& res) {
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
int centreonengine__serviceGetStalkOnOk(soap* s,
					ns1__serviceIDType* service_id,
					centreonengine__serviceGetStalkOnOkResponse& res) {
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
int centreonengine__serviceGetStalkOnUnknown(soap* s,
					     ns1__serviceIDType* service_id,
					     centreonengine__serviceGetStalkOnUnknownResponse& res) {
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
int centreonengine__serviceGetStalkOnWarning(soap* s,
					     ns1__serviceIDType* service_id,
					     centreonengine__serviceGetStalkOnWarningResponse& res) {
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
int centreonengine__serviceSetStalkOnCritical(soap* s,
					      ns1__serviceIDType* service_id,
					      bool enable,
					      centreonengine__serviceSetStalkOnCriticalResponse& res) {
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
int centreonengine__serviceSetStalkOnOk(soap* s,
					ns1__serviceIDType* service_id,
					bool enable,
					centreonengine__serviceSetStalkOnOkResponse& res) {
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
int centreonengine__serviceSetStalkOnUnknown(soap* s,
					     ns1__serviceIDType* service_id,
					     bool enable,
					     centreonengine__serviceSetStalkOnUnknownResponse& res) {
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
int centreonengine__serviceSetStalkOnWarning(soap* s,
					     ns1__serviceIDType* service_id,
					     bool enable,
					     centreonengine__serviceSetStalkOnWarningResponse& res) {
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
int centreonengine__serviceGetStateCurrent(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonengine__serviceGetStateCurrentResponse& res) {
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
int centreonengine__serviceGetStateInitial(soap* s,
					   ns1__serviceIDType* service_id,
					   centreonengine__serviceGetStateInitialResponse& res) {
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
int centreonengine__serviceGetStateLast(soap* s,
					ns1__serviceIDType* service_id,
					centreonengine__serviceGetStateLastResponse& res) {
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
int centreonengine__serviceGetStateLastChange(soap* s,
					      ns1__serviceIDType* service_id,
					      centreonengine__serviceGetStateLastChangeResponse& res) {
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
int centreonengine__serviceGetStateLastCritical(soap* s,
						ns1__serviceIDType* service_id,
						centreonengine__serviceGetStateLastCriticalResponse& res) {
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
int centreonengine__serviceGetStateLastHard(soap* s,
					    ns1__serviceIDType* service_id,
					    centreonengine__serviceGetStateLastHardResponse& res) {
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
int centreonengine__serviceGetStateLastHardChange(soap* s,
						  ns1__serviceIDType* service_id,
						  centreonengine__serviceGetStateLastHardChangeResponse& res) {
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
int centreonengine__serviceGetStateLastOk(soap* s,
					  ns1__serviceIDType* service_id,
					  centreonengine__serviceGetStateLastOkResponse& res) {
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
int centreonengine__serviceGetStateLastUnknown(soap* s,
					       ns1__serviceIDType* service_id,
					       centreonengine__serviceGetStateLastUnknownResponse& res) {
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
int centreonengine__serviceGetStateLastWarning(soap* s,
					       ns1__serviceIDType* service_id,
					       centreonengine__serviceGetStateLastWarningResponse& res) {
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
int centreonengine__serviceGetStatePercentChange(soap* s,
						 ns1__serviceIDType* service_id,
						 centreonengine__serviceGetStatePercentChangeResponse& res) {
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
int centreonengine__serviceGetStateType(soap* s,
					ns1__serviceIDType* service_id,
					centreonengine__serviceGetStateTypeResponse& res) {
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
int centreonengine__acknowledgementOnHostAdd(soap* s,
					     ns1__hostIDType* host_id,
					     ns1__acknowledgementType* acknowledgement_type,
					     centreonengine__acknowledgementOnHostAddResponse& res) {
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
int centreonengine__acknowledgementOnHostRemove(soap* s,
						ns1__hostIDType* host_id,
						centreonengine__acknowledgementOnHostRemoveResponse& res) {
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
int centreonengine__acknowledgementOnServiceAdd(soap* s,
						ns1__serviceIDType* service_id,
						ns1__acknowledgementType* acknowledgement_type,
						centreonengine__acknowledgementOnServiceAddResponse& res) {
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
int centreonengine__acknowledgementOnServiceRemove(soap* s,
						   ns1__serviceIDType* service_id,
						   centreonengine__acknowledgementOnServiceRemoveResponse& res) {
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
int centreonengine__checkHostProcessResult(soap* s,
					   ns1__hostIDType* host_id,
					   ns1__checkResultType* result_type,
					   centreonengine__checkHostProcessResultResponse& res) {
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
int centreonengine__checkHostSchedule(soap* s,
				      ns1__hostIDType* host_id,
				      long delay,
				      centreonengine__checkHostScheduleResponse& res) {
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
int centreonengine__checkHostScheduleForced(soap* s,
					    ns1__hostIDType* host_id,
					    long delay,
					    centreonengine__checkHostScheduleForcedResponse& res) {
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
int centreonengine__checkHostScheduleServices(soap* s,
					      ns1__hostIDType* host_id,
					      long delay,
					      centreonengine__checkHostScheduleServicesResponse& res) {
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
int centreonengine__checkHostScheduleServicesForced(soap* s,
						    ns1__hostIDType* host_id,
						    long delay,
						    centreonengine__checkHostScheduleServicesForcedResponse& res) {
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
int centreonengine__checkServiceProcessResult(soap* s,
					      ns1__serviceIDType* service_id,
					      ns1__checkResultType* result_type,
					      centreonengine__checkServiceProcessResultResponse& res) {
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
int centreonengine__checkServiceSchedule(soap* s,
					 ns1__serviceIDType* service_id,
					 long delay,
					 centreonengine__checkServiceScheduleResponse& res) {
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
int centreonengine__checkServiceScheduleForced(soap* s,
					       ns1__serviceIDType* service_id,
					       long delay,
					       centreonengine__checkServiceScheduleForcedResponse& res) {
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
int centreonengine__downtimeGetAuthor(soap* s,
				      ns1__downtimeIDType* downtime_id,
				      centreonengine__downtimeGetAuthorResponse& res) {
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
int centreonengine__downtimeGetDuration(soap* s,
					ns1__downtimeIDType* downtime_id,
					centreonengine__downtimeGetDurationResponse& res) {
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
int centreonengine__downtimeGetEnd(soap* s,
				   ns1__downtimeIDType* downtime_id,
				   centreonengine__downtimeGetEndResponse& res) {
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
int centreonengine__downtimeGetFixed(soap* s,
				     ns1__downtimeIDType* downtime_id,
				     centreonengine__downtimeGetFixedResponse& res) {
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
int centreonengine__downtimeGetStart(soap* s,
				     ns1__downtimeIDType* downtime_id,
				     centreonengine__downtimeGetStartResponse& res) {
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
int centreonengine__downtimeDelete(soap* s,
				   ns1__downtimeIDType* downtime_id,
				   centreonengine__downtimeDeleteResponse& res) {
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
int centreonengine__downtimeAddToHost(soap* s,
				      ns1__hostIDType* host_id,
				      ns1__downtimeType* downtime_type,
				      centreonengine__downtimeAddToHostResponse& res) {
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
int centreonengine__downtimeAddAndPropagateToHost(soap* s,
						  ns1__hostIDType* host_id,
						  ns1__downtimeType* downtime_type,
						  centreonengine__downtimeAddAndPropagateToHostResponse& res) {
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
int centreonengine__downtimeAddAndPropagateTriggeredToHost(soap* s,
							   ns1__hostIDType* host_id,
							   ns1__downtimeType* downtime_type,
							   centreonengine__downtimeAddAndPropagateTriggeredToHostResponse& res) {
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
int centreonengine__downtimeAddToHostServices(soap* s,
					      ns1__hostIDType* host_id,
					      ns1__downtimeType* downtime_type,
					      centreonengine__downtimeAddToHostServicesResponse& res) {
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
int centreonengine__downtimeAddToService(soap* s,
					 ns1__serviceIDType* service_id,
					 ns1__downtimeType* downtime_type,
					 centreonengine__downtimeAddToServiceResponse& res) {
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
int centreonengine__notificationHostDelay(soap* s,
					  ns1__hostIDType* host_id,
					  long delay,
					  centreonengine__notificationHostDelayResponse& res) {
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
int centreonengine__notificationHostSend(soap* s,
					 ns1__hostIDType* host_id,
					 ns1__notificationType* notification_type,
					 centreonengine__notificationHostSendResponse& res) {
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
int centreonengine__notificationServiceDelay(soap* s,
					     ns1__serviceIDType* service_id,
					     long delay,
					     centreonengine__notificationServiceDelayResponse& res) {
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
int centreonengine__notificationServiceSend(soap* s,
					    ns1__serviceIDType* service_id,
					    ns1__notificationType* notification_type,
					    centreonengine__notificationServiceSendResponse& res) {
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
int centreonengine__getEventHandlersEnabled(soap* s,
					    centreonengine__getEventHandlersEnabledResponse& res) {
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
int centreonengine__getFailurePredictionEnabled(soap* s,
						centreonengine__getFailurePredictionEnabledResponse& res) {
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
int centreonengine__getFlapDetectionEnabled(soap* s,
					    centreonengine__getFlapDetectionEnabledResponse& res) {
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
int centreonengine__getHostsChecksActiveEnabled(soap* s,
						centreonengine__getHostsChecksActiveEnabledResponse& res) {
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
int centreonengine__getHostsChecksPassiveEnabled(soap* s,
						 centreonengine__getHostsChecksPassiveEnabledResponse& res) {
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
int centreonengine__getHostsEventHandler(soap* s,
					 centreonengine__getHostsEventHandlerResponse& res) {
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
int centreonengine__getHostsFreshnessChecksEnabled(soap* s,
						   centreonengine__getHostsFreshnessChecksEnabledResponse& res) {
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
int centreonengine__getHostsObsessOverEnabled(soap* s,
					      centreonengine__getHostsObsessOverEnabledResponse& res) {
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
int centreonengine__getNotificationsEnabled(soap* s,
					    centreonengine__getNotificationsEnabledResponse& res) {
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
int centreonengine__getPerfdataProcessingEnabled(soap* s,
						 centreonengine__getPerfdataProcessingEnabledResponse& res) {
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
int centreonengine__getServicesChecksActiveEnabled(soap* s,
						   centreonengine__getServicesChecksActiveEnabledResponse& res) {
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
int centreonengine__getServicesChecksPassiveEnabled(soap* s,
						    centreonengine__getServicesChecksPassiveEnabledResponse& res) {
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
int centreonengine__getServicesEventHandler(soap* s,
					    centreonengine__getServicesEventHandlerResponse& res) {
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
int centreonengine__getServicesFreshnessChecksEnabled(soap* s,
						      centreonengine__getServicesFreshnessChecksEnabledResponse& res) {
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
int centreonengine__getServicesObsessOverEnabled(soap* s,
						 centreonengine__getServicesObsessOverEnabledResponse& res) {
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
int centreonengine__setEventHandlersEnabled(soap* s,
					    bool enable,
					    centreonengine__setEventHandlersEnabledResponse& res) {
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
int centreonengine__setFailurePredictionEnabled(soap* s,
						bool enable,
						centreonengine__setFailurePredictionEnabledResponse& res) {
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
int centreonengine__setFlapDetectionEnabled(soap* s,
					    bool enable,
					    centreonengine__setFlapDetectionEnabledResponse& res) {
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
int centreonengine__setHostsChecksActiveEnabled(soap* s,
						bool enable,
						centreonengine__setHostsChecksActiveEnabledResponse& res) {
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
int centreonengine__setHostsChecksPassiveEnabled(soap* s,
						 bool enable,
						 centreonengine__setHostsChecksPassiveEnabledResponse& res) {
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
int centreonengine__setHostsEventHandler(soap* s,
					 ns1__commandIDType* command_id,
					 centreonengine__setHostsEventHandlerResponse& res) {
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
int centreonengine__setHostsFreshnessChecksEnabled(soap* s,
						   bool enable,
						   centreonengine__setHostsFreshnessChecksEnabledResponse& res) {
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
int centreonengine__setHostsObsessOverEnabled(soap* s,
					      bool enable,
					      centreonengine__setHostsObsessOverEnabledResponse& res) {
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
int centreonengine__setNotificationsEnabled(soap* s,
					    bool enable,
					    centreonengine__setNotificationsEnabledResponse& res) {
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
int centreonengine__setPerfdataProcessingEnabled(soap* s,
						 bool enable,
						 centreonengine__setPerfdataProcessingEnabledResponse& res) {
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
int centreonengine__setServicesChecksActiveEnabled(soap* s,
						   bool enable,
						   centreonengine__setServicesChecksActiveEnabledResponse& res) {
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
int centreonengine__setServicesChecksPassiveEnabled(soap* s,
						    bool enable,
						    centreonengine__setServicesChecksPassiveEnabledResponse& res) {
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
int centreonengine__setServicesEventHandler(soap* s,
					    ns1__commandIDType* command_id,
					    centreonengine__setServicesEventHandlerResponse& res) {
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
int centreonengine__setServicesFreshnessChecksEnabled(soap* s,
						      bool enable,
						      centreonengine__setServicesFreshnessChecksEnabledResponse& res) {
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
int centreonengine__setServicesObsessOverEnabled(soap* s,
						 bool enable,
						 centreonengine__setServicesObsessOverEnabledResponse& res) {
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
int centreonengine__hostSetCheckPeriod(soap* s,
				       ns1__hostIDType* host_id,
				       ns1__timeperiodIDType* timeperiod_id,
				       centreonengine__hostSetCheckPeriodResponse& res) {
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
int centreonengine__hostSetServicesCheckActiveEnabled(soap* s,
						      ns1__hostIDType* host_id,
						      bool enable,
						      centreonengine__hostSetServicesCheckActiveEnabledResponse& res) {
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
int centreonengine__hostSetServicesNotificationsEnabled(soap* s,
							ns1__hostIDType* host_id,
							bool enable,
							centreonengine__hostSetServicesNotificationsEnabledResponse& res) {
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
int centreonengine__serviceSetCheckPeriod(soap* s,
					  ns1__serviceIDType* service_id,
					  ns1__timeperiodIDType* timeperiod_id,
					  centreonengine__serviceSetCheckPeriodResponse& res) {
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
int centreonengine__serviceSetNotificationsPeriod(soap* s,
						  ns1__serviceIDType* service_id,
						  ns1__timeperiodIDType* timeperiod_id,
						  centreonengine__serviceSetNotificationsPeriodResponse& res) {
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
int centreonengine__contactGetAlias(soap* s,
				    ns1__contactIDType* contact_id,
				    centreonengine__contactGetAliasResponse& res) {
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
int centreonengine__contactGetCanSubmitCommands(soap* s,
						ns1__contactIDType* contact_id,
						centreonengine__contactGetCanSubmitCommandsResponse& res) {
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
int centreonengine__contactGetEmail(soap* s,
				    ns1__contactIDType* contact_id,
				    centreonengine__contactGetEmailResponse& res) {
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
int centreonengine__contactGetModifiedAttributes(soap* s,
						 ns1__contactIDType* contact_id,
						 centreonengine__contactGetModifiedAttributesResponse& res) {
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
int centreonengine__contactGetModifiedAttributesHost(soap* s,
						     ns1__contactIDType* contact_id,
						     centreonengine__contactGetModifiedAttributesHostResponse& res) {
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
int centreonengine__contactGetModifiedAttributesService(soap* s,
							ns1__contactIDType* contact_id,
							centreonengine__contactGetModifiedAttributesServiceResponse& res) {
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
int centreonengine__contactGetNotificationsOnHostCommand(soap* s,
							 ns1__contactIDType* contact_id,
							 centreonengine__contactGetNotificationsOnHostCommandResponse& res) {
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
int centreonengine__contactGetNotificationsOnHostDown(soap* s,
						      ns1__contactIDType* contact_id,
						      centreonengine__contactGetNotificationsOnHostDownResponse& res) {
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
int centreonengine__contactGetNotificationsOnHostDowntime(soap* s,
							  ns1__contactIDType* contact_id,
							  centreonengine__contactGetNotificationsOnHostDowntimeResponse& res) {
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
int centreonengine__contactGetNotificationsOnHostEnabled(soap* s,
							 ns1__contactIDType* contact_id,
							 centreonengine__contactGetNotificationsOnHostEnabledResponse& res) {
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
int centreonengine__contactGetNotificationsOnHostFlapping(soap* s,
							  ns1__contactIDType* contact_id,
							  centreonengine__contactGetNotificationsOnHostFlappingResponse& res) {
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
int centreonengine__contactGetNotificationsOnHostLast(soap* s,
						      ns1__contactIDType* contact_id,
						      centreonengine__contactGetNotificationsOnHostLastResponse& res) {
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
int centreonengine__contactGetNotificationsOnHostRecovery(soap* s,
							  ns1__contactIDType* contact_id,
							  centreonengine__contactGetNotificationsOnHostRecoveryResponse& res) {
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
int centreonengine__contactGetNotificationsOnHostTimeperiod(soap* s,
							    ns1__contactIDType* contact_id,
							    centreonengine__contactGetNotificationsOnHostTimeperiodResponse& res) {
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
int centreonengine__contactGetNotificationsOnHostUnreachable(soap* s,
							     ns1__contactIDType* contact_id,
							     centreonengine__contactGetNotificationsOnHostUnreachableResponse& res) {
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
int centreonengine__contactGetNotificationsOnServiceCommand(soap* s,
							    ns1__contactIDType* contact_id,
							    centreonengine__contactGetNotificationsOnServiceCommandResponse& res) {
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
int centreonengine__contactGetNotificationsOnServiceCritical(soap* s,
							     ns1__contactIDType* contact_id,
							     centreonengine__contactGetNotificationsOnServiceCriticalResponse& res) {
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
int centreonengine__contactGetNotificationsOnServiceDowntime(soap* s,
							     ns1__contactIDType* contact_id,
							     centreonengine__contactGetNotificationsOnServiceDowntimeResponse& res) {
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
int centreonengine__contactGetNotificationsOnServiceEnabled(soap* s,
							    ns1__contactIDType* contact_id,
							    centreonengine__contactGetNotificationsOnServiceEnabledResponse& res) {
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
int centreonengine__contactGetNotificationsOnServiceFlapping(soap* s,
							     ns1__contactIDType* contact_id,
							     centreonengine__contactGetNotificationsOnServiceFlappingResponse& res) {
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
int centreonengine__contactGetNotificationsOnServiceLast(soap* s,
							 ns1__contactIDType* contact_id,
							 centreonengine__contactGetNotificationsOnServiceLastResponse& res) {
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
int centreonengine__contactGetNotificationsOnServiceRecovery(soap* s,
							     ns1__contactIDType* contact_id,
							     centreonengine__contactGetNotificationsOnServiceRecoveryResponse& res) {
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
int centreonengine__contactGetNotificationsOnServiceTimeperiod(soap* s,
							       ns1__contactIDType* contact_id,
							       centreonengine__contactGetNotificationsOnServiceTimeperiodResponse& res) {
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
int centreonengine__contactGetNotificationsOnServiceUnknown(soap* s,
							    ns1__contactIDType* contact_id,
							    centreonengine__contactGetNotificationsOnServiceUnknownResponse& res) {
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
int centreonengine__contactGetPager(soap* s,
				    ns1__contactIDType* contact_id,
				    centreonengine__contactGetPagerResponse& res) {
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
int centreonengine__contactGetRetainStatusInformation(soap* s,
						      ns1__contactIDType* contact_id,
						      centreonengine__contactGetRetainStatusInformationResponse& res) {
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
int centreonengine__contactGetRetainStatusNonInformation(soap* s,
							 ns1__contactIDType* contact_id,
							 centreonengine__contactGetRetainStatusNonInformationResponse& res) {
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
int centreonengine__contactSetAlias(soap* s,
				    ns1__contactIDType* contact_id,
				    std::string alias,
				    centreonengine__contactSetAliasResponse& res) {
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
int centreonengine__contactSetCanSubmitCommands(soap* s,
						ns1__contactIDType* contact_id,
						bool enable,
						centreonengine__contactSetCanSubmitCommandsResponse& res) {
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
int centreonengine__contactSetEmail(soap* s,
				    ns1__contactIDType* contact_id,
				    std::string email,
				    centreonengine__contactSetEmailResponse& res) {
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
int centreonengine__contactSetNotificationsOnHostDown(soap* s,
						      ns1__contactIDType* contact_id,
						      bool enable,
						      centreonengine__contactSetNotificationsOnHostDownResponse& res) {
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
int centreonengine__contactSetNotificationsOnHostDowntime(soap* s,
							  ns1__contactIDType* contact_id,
							  bool enable,
							  centreonengine__contactSetNotificationsOnHostDowntimeResponse& res) {
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
int centreonengine__contactSetNotificationsOnHostEnabled(soap* s,
							 ns1__contactIDType* contact_id,
							 bool enable,
							 centreonengine__contactSetNotificationsOnHostEnabledResponse& res) {
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
int centreonengine__contactSetNotificationsOnHostFlapping(soap* s,
							  ns1__contactIDType* contact_id,
							  bool enable,
							  centreonengine__contactSetNotificationsOnHostFlappingResponse& res) {
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
int centreonengine__contactSetNotificationsOnHostRecovery(soap* s,
							  ns1__contactIDType* contact_id,
							  bool enable,
							  centreonengine__contactSetNotificationsOnHostRecoveryResponse& res) {
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
int centreonengine__contactSetNotificationsOnHostTimeperiod(soap* s,
							    ns1__contactIDType* contact_id,
							    ns1__timeperiodIDType* timeperiod_id,
							    centreonengine__contactSetNotificationsOnHostTimeperiodResponse& res) {
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
int centreonengine__contactSetNotificationsOnHostUnreachable(soap* s,
							     ns1__contactIDType* contact_id,
							     bool enable,
							     centreonengine__contactSetNotificationsOnHostUnreachableResponse& res) {
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
int centreonengine__contactSetNotificationsOnServiceCritical(soap* s,
							     ns1__contactIDType* contact_id,
							     bool enable,
							     centreonengine__contactSetNotificationsOnServiceCriticalResponse& res) {
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
int centreonengine__contactSetNotificationsOnServiceDowntime(soap* s,
							     ns1__contactIDType* contact_id,
							     bool enable,
							     centreonengine__contactSetNotificationsOnServiceDowntimeResponse& res) {
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
int centreonengine__contactSetNotificationsOnServiceEnabled(soap* s,
							    ns1__contactIDType* contact_id,
							    bool enable,
							    centreonengine__contactSetNotificationsOnServiceEnabledResponse& res) {
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
int centreonengine__contactSetNotificationsOnServiceFlapping(soap* s,
							     ns1__contactIDType* contact_id,
							     bool enable,
							     centreonengine__contactSetNotificationsOnServiceFlappingResponse& res) {
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
int centreonengine__contactSetNotificationsOnServiceRecovery(soap* s,
							     ns1__contactIDType* contact_id,
							     bool enable,
							     centreonengine__contactSetNotificationsOnServiceRecoveryResponse& res) {
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
int centreonengine__contactSetNotificationsOnServiceTimeperiod(soap* s,
							       ns1__contactIDType* contact_id,
							       ns1__timeperiodIDType* timeperiod_id,
							       centreonengine__contactSetNotificationsOnServiceTimeperiodResponse& res) {
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
int centreonengine__contactSetNotificationsOnServiceUnknown(soap* s,
							    ns1__contactIDType* contact_id,
							    bool enable,
							    centreonengine__contactSetNotificationsOnServiceUnknownResponse& res) {
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
int centreonengine__contactSetNotificationsOnServiceWarning(soap* s,
							    ns1__contactIDType* contact_id,
							    bool enable,
							    centreonengine__contactSetNotificationsOnServiceWarningResponse& res) {
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
int centreonengine__contactSetPager(soap* s,
				    ns1__contactIDType* contact_id,
				    std::string pager,
				    centreonengine__contactSetPagerResponse& res) {
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
int centreonengine__contactSetRetainStatusInformation(soap* s,
						      ns1__contactIDType* contact_id,
						      bool enable,
						      centreonengine__contactSetRetainStatusInformationResponse& res) {
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
int centreonengine__contactSetRetainStatusNonInformation(soap* s,
							 ns1__contactIDType* contact_id,
							 bool enable,
							 centreonengine__contactSetRetainStatusNonInformationResponse& res) {
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
int centreonengine__contactSetNotificationsOnHostCommand(soap* s,
							 ns1__contactIDType* contact_id,
							 ns1__commandIDType* command_id,
							 centreonengine__contactSetNotificationsOnHostCommandResponse& res) {
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
int centreonengine__contactSetNotificationsOnServiceCommand(soap* s,
							    ns1__contactIDType* contact_id,
							    ns1__commandIDType* command_id,
							    centreonengine__contactSetNotificationsOnServiceCommandResponse& res) {
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
