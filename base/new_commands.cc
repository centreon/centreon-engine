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

#include <string>
#include <string.h>
#include "centreonscheduler.nsmap" // gSOAP namespaces.
#include "soapH.h"
#include "objects.hh"
#include "logging.hh"
#include "sretention.hh"

extern int sigrestart;
extern int sigshutdown;

/**
 *  Restart Scheduler.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__restart(soap* s,
			       centreonscheduler__restartResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_EVENTS, 0, "Webservice: program restart\n");
  sigrestart = true;
  logit(NSLOG_PROCESS_INFO, true, "Webservice: program restarting...\n");

  res.error->code = 0;
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
int centreonscheduler__shutdown(soap* s,
				centreonscheduler__shutdownResponse& res) {
  (void)s;

  log_debug_info(DEBUGL_EVENTS, 0, "Webservice: program shutdown\n");
  sigshutdown = true;
  logit(NSLOG_PROCESS_INFO, true, "Webservice: program shutting down...\n");

  res.error->code = 0;
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
int centreonscheduler__readStateInformation(soap* s,
					    centreonscheduler__readStateInformationResponse& res) {
  (void)s;

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
int centreonscheduler__saveStateInformation(soap* s,
					    centreonscheduler__saveStateInformationResponse& res) {
  (void)s;

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
int centreonscheduler__getHostAcknowledgementIsSet(soap* s,
						   ns1__hostIDType* host_id,
						   centreonscheduler__getHostAcknowledgementIsSetResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostAcknowledgementType(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__getHostAcknowledgementTypeResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostAddress(soap* s,
				      ns1__hostIDType* host_id,
				      centreonscheduler__getHostAddressResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = (host->address != NULL ? host->address : "");
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
int centreonscheduler__setHostAddress(soap* s,
				      ns1__hostIDType* host_id,
				      std::string address,
				      centreonscheduler__setHostAddressResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  if (address == "") {
    res.error->message = "Host `" + host_id->name + "' address is empty.";
  }

  delete[] host->address;
  host->address = new char[address.size() + 1];
  strcpy(host->address, address.c_str());

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
int centreonscheduler__getHostCheckActiveEnabled(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__getHostCheckActiveEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostCheckCommand(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__getHostCheckCommandResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = (host->host_check_command != NULL ? host->host_check_command : "");
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
int centreonscheduler__getHostCheckCurrentAttempt(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__getHostCheckCurrentAttemptResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostCheckIntervalNormal(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__getHostCheckIntervalNormalResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostCheckIntervalRetry(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__getHostCheckIntervalRetryResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostCheckLast(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__getHostCheckLastResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostCheckMaxAttempts(soap* s,
					       ns1__hostIDType* host_id,
					       centreonscheduler__getHostCheckMaxAttemptsResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostCheckNext(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__getHostCheckNextResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostCheckOptions(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__getHostCheckOptionsResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostCheckPassiveEnabled(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__getHostCheckPassiveEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostCheckPeriod(soap* s,
					  ns1__hostIDType* host_id,
					  centreonscheduler__getHostCheckPeriodResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = (host->check_period != NULL ? host->check_period : "");
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
int centreonscheduler__getHostCheckShouldBeScheduled(soap* s,
						     ns1__hostIDType* host_id,
						     centreonscheduler__getHostCheckShouldBeScheduledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostCheckType(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__getHostCheckTypeResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostCheckActiveEnabled(soap* s,
						 ns1__hostIDType* host_id,
						 bool enable,
						 centreonscheduler__setHostCheckActiveEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  command
 *  @param[out] res      Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostCheckCommand(soap* s,
					   ns1__hostIDType* host_id,
					   std::string command,
					   centreonscheduler__setHostCheckCommandResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  if (command == "") {
    res.error->message = "Host `" + host_id->name + "' command is empty.";
  }

  delete[] host->host_check_command;
  host->host_check_command = new char[command.size() + 1];
  strcpy(host->host_check_command, command.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the normal check interval of the host.
 *
 *  @param[in]  s         Unused.
 *  @param[in]  host_id   Host to set data.
 *  @param[in]  interval Check interval time.
 *  @param[out] res       Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostCheckIntervalNormal(soap* s,
						  ns1__hostIDType* host_id,
						  unsigned int interval,
						  centreonscheduler__setHostCheckIntervalNormalResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  interval Check interval time.
 *  @param[out] res       Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostCheckIntervalRetry(soap* s,
						 ns1__hostIDType* host_id,
						 unsigned int interval,
						 centreonscheduler__setHostCheckIntervalRetryResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  attempts
 *  @param[out] res       Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostCheckMaxAttempts(soap* s,
					       ns1__hostIDType* host_id,
					       unsigned int attempts,
					       centreonscheduler__setHostCheckMaxAttemptsResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  if (attempts == 0) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' bad attempts value.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostCheckPassiveEnabled(soap* s,
						  ns1__hostIDType* host_id,
						  bool enable,
						  centreonscheduler__setHostCheckPassiveEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  host->accept_passive_host_checks = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the host check period.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  period Host's check period.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostCheckPeriod(soap* s,
					  ns1__hostIDType* host_id,
					  std::string period,
					  centreonscheduler__setHostCheckPeriodResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  if (period == "") {
    res.error->message = "Host `" + host_id->name + "' period is empty.";
  }

  delete[] host->check_period;
  host->check_period = new char[period.size() + 1];
  strcpy(host->check_period, period.c_str());

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
int centreonscheduler__getHostCircularPathChecked(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__getHostCircularPathCheckedResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostCircularPathHas(soap* s,
					      ns1__hostIDType* host_id,
					      centreonscheduler__getHostCircularPathHasResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostDowntimeDepth(soap* s,
					    ns1__hostIDType* host_id,
					    centreonscheduler__getHostDowntimeDepthResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostDowntimeFlexPending(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__getHostDowntimeFlexPendingResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostEventHandler(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__getHostEventHandlerResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = (host->event_handler != NULL ? host->event_handler : "");
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
int centreonscheduler__getHostEventHandlerEnabled(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__getHostEventHandlerEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  eventhandler
 *  @param[out] res           Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostEventHandler(soap* s,
					   ns1__hostIDType* host_id,
					   std::string eventhandler,
					   centreonscheduler__setHostEventHandlerResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  if (eventhandler == "") {
    res.error->message = "Host `" + host_id->name + "' eventhandler is empty.";
  }

  delete[] host->event_handler;
  host->event_handler = new char[eventhandler.size() + 1];
  strcpy(host->event_handler, eventhandler.c_str());

  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable host_id event handler.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostEventHandlerEnabled(soap* s,
						  ns1__hostIDType* host_id,
						  bool enable,
						  centreonscheduler__setHostEventHandlerEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFailurePredictionEnabled(soap* s,
						       ns1__hostIDType* host_id,
						       centreonscheduler__getHostFailurePredictionEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = host->failure_prediction_enabled;
  res.error->code = 0;

  return (SOAP_OK);
}
// XXX
/**
 *  Get host_id failure prediction options.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to get data.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__getHostFailurePredictionOptions(soap* s,
						       ns1__hostIDType* host_id,
						       centreonscheduler__getHostFailurePredictionOptionsResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = (host->failure_prediction_options != NULL ? host->failure_prediction_options : "");
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable failure prediction on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostFailurePredictionEnabled(soap* s,
						       ns1__hostIDType* host_id,
						       bool enable,
						       centreonscheduler__setHostFailurePredictionEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFlapDetectionCommentID(soap* s,
						     ns1__hostIDType* host_id,
						     centreonscheduler__getHostFlapDetectionCommentIDResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFlapDetectionEnabled(soap* s,
						   ns1__hostIDType* host_id,
						   centreonscheduler__getHostFlapDetectionEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFlapDetectionIsFlapping(soap* s,
						      ns1__hostIDType* host_id,
						      centreonscheduler__getHostFlapDetectionIsFlappingResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFlapDetectionOnDown(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__getHostFlapDetectionOnDownResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFlapDetectionOnUnreachable(soap* s,
							 ns1__hostIDType* host_id,
							 centreonscheduler__getHostFlapDetectionOnUnreachableResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFlapDetectionOnUp(soap* s,
						ns1__hostIDType* host_id,
						centreonscheduler__getHostFlapDetectionOnUpResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFlapDetectionStateHistoryLastUpdate(soap* s,
								  ns1__hostIDType* host_id,
								  centreonscheduler__getHostFlapDetectionStateHistoryLastUpdateResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFlapDetectionThresholdHigh(soap* s,
							 ns1__hostIDType* host_id,
							 centreonscheduler__getHostFlapDetectionThresholdHighResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFlapDetectionThresholdLow(soap* s,
							ns1__hostIDType* host_id,
							centreonscheduler__getHostFlapDetectionThresholdLowResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostFlapDetectionEnabled(soap* s,
						   ns1__hostIDType* host_id,
						   bool enable,
						   centreonscheduler__setHostFlapDetectionEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostFlapDetectionOnDown(soap* s,
						  ns1__hostIDType* host_id,
						  bool enable,
						  centreonscheduler__setHostFlapDetectionOnDownResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostFlapDetectionOnUnreachable(soap* s,
							 ns1__hostIDType* host_id,
							 bool enable,
							 centreonscheduler__setHostFlapDetectionOnUnreachableResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostFlapDetectionOnUp(soap* s,
						ns1__hostIDType* host_id,
						bool enable,
						centreonscheduler__setHostFlapDetectionOnUpResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  threshold
 *  @param[out] res        Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostFlapDetectionThresholdHigh(soap* s,
							 ns1__hostIDType* host_id,
							 double threshold,
							 centreonscheduler__setHostFlapDetectionThresholdHighResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  threshold
 *  @param[out] res        Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostFlapDetectionThresholdLow(soap* s,
							ns1__hostIDType* host_id,
							double threshold,
							centreonscheduler__setHostFlapDetectionThresholdLowResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFreshnessCheckEnabled(soap* s,
						    ns1__hostIDType* host_id,
						    centreonscheduler__getHostFreshnessCheckEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFreshnessIsActive(soap* s,
						ns1__hostIDType* host_id,
						centreonscheduler__getHostFreshnessIsActiveResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostFreshnessThreshold(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__getHostFreshnessThresholdResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostFreshnessCheckEnabled(soap* s,
						    ns1__hostIDType* host_id,
						    bool enable,
						    centreonscheduler__setHostFreshnessCheckEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  threshold
 *  @param[out] res        Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostFreshnessThreshold(soap* s,
						 ns1__hostIDType* host_id,
						 int threshold,
						 centreonscheduler__setHostFreshnessThresholdResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostModifiedAttributes(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__getHostModifiedAttributesResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNameAlias(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__getHostNameAliasResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = (host->alias != NULL ? host->alias : "");
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
int centreonscheduler__getHostNameDisplay(soap* s,
					  ns1__hostIDType* host_id,
					  centreonscheduler__getHostNameDisplayResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = (host->display_name != NULL ? host->display_name : "");
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set the host alias.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  alias  Host's alias.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostNameAlias(soap* s,
					ns1__hostIDType* host_id,
					std::string alias,
					centreonscheduler__setHostNameAliasResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  if (alias == "") {
    res.error->message = "Host `" + host_id->name + "' alias is empty.";
  }

  delete[] host->alias;
  host->alias = new char[alias.size() + 1];
  strcpy(host->alias, alias.c_str());

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
int centreonscheduler__setHostNameDisplay(soap* s,
					  ns1__hostIDType* host_id,
					  std::string displayname,
					  centreonscheduler__setHostNameDisplayResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  if (displayname == "") {
    res.error->message = "Host `" + host_id->name + "' displayname is empty.";
  }

  delete[] host->display_name;
  host->display_name = new char[displayname.size() + 1];
  strcpy(host->display_name, displayname.c_str());

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
int centreonscheduler__getHostNotificationCurrentID(soap* s,
						    ns1__hostIDType* host_id,
						    centreonscheduler__getHostNotificationCurrentIDResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationCurrentNumber(soap* s,
							ns1__hostIDType* host_id,
							centreonscheduler__getHostNotificationCurrentNumberResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationEnabled(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__getHostNotificationEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationFirstDelay(soap* s,
						     ns1__hostIDType* host_id,
						     centreonscheduler__getHostNotificationFirstDelayResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationInterval(soap* s,
						   ns1__hostIDType* host_id,
						   centreonscheduler__getHostNotificationIntervalResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationLast(soap* s,
					       ns1__hostIDType* host_id,
					       centreonscheduler__getHostNotificationLastResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationNext(soap* s,
					       ns1__hostIDType* host_id,
					       centreonscheduler__getHostNotificationNextResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationOnDown(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__getHostNotificationOnDownResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationOnDowntime(soap* s,
						     ns1__hostIDType* host_id,
						     centreonscheduler__getHostNotificationOnDowntimeResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationOnFlapping(soap* s,
						     ns1__hostIDType* host_id,
						     centreonscheduler__getHostNotificationOnFlappingResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationOnRecovery(soap* s,
						     ns1__hostIDType* host_id,
						     centreonscheduler__getHostNotificationOnRecoveryResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationOnUnreachable(soap* s,
							ns1__hostIDType* host_id,
							centreonscheduler__getHostNotificationOnUnreachableResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostNotificationPeriod(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__getHostNotificationPeriodResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = (host->notification_period != NULL ? host->notification_period : "");
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostNotificationEnabled(soap* s,
						  ns1__hostIDType* host_id,
						  bool enable,
						  centreonscheduler__setHostNotificationEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  delay  Delay of the first notification.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostNotificationFirstDelay(soap* s,
						     ns1__hostIDType* host_id,
						     unsigned int delay,
						     centreonscheduler__setHostNotificationFirstDelayResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  interval Notification interval.
 *  @param[out] res       Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostNotificationInterval(soap* s,
						   ns1__hostIDType* host_id,
						   unsigned int interval,
						   centreonscheduler__setHostNotificationIntervalResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostNotificationOnDown(soap* s,
						 ns1__hostIDType* host_id,
						 bool enable,
						 centreonscheduler__setHostNotificationOnDownResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostNotificationOnDowntime(soap* s,
						     ns1__hostIDType* host_id,
						     bool enable,
						     centreonscheduler__setHostNotificationOnDowntimeResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostNotificationOnFlapping(soap* s,
						     ns1__hostIDType* host_id,
						     bool enable,
						     centreonscheduler__setHostNotificationOnFlappingResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostNotificationOnRecovery(soap* s,
						     ns1__hostIDType* host_id,
						     bool enable,
						     centreonscheduler__setHostNotificationOnRecoveryResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostNotificationOnUnreachable(soap* s,
							ns1__hostIDType* host_id,
							bool enable,
							centreonscheduler__setHostNotificationOnUnreachableResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  host->notify_on_unreachable = enable;
  res.error->code = 0;

  return (SOAP_OK);
}

/**
 *  Set notification period of host.
 *
 *  @param[in]  s       Unused.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  period  Notification period.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostNotificationPeriod(soap* s,
						 ns1__hostIDType* host_id,
						 std::string period,
						 centreonscheduler__setHostNotificationPeriodResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  if (period == "") {
    res.error->message = "Host `" + host_id->name + "' period is empty.";
  }

  delete[] host->notification_period;
  host->notification_period = new char[period.size() + 1];
  strcpy(host->notification_period, period.c_str());

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
int centreonscheduler__getHostObsessOver(soap* s,
					 ns1__hostIDType* host_id,
					 centreonscheduler__getHostObsessOverResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostObsessOver(soap* s,
					 ns1__hostIDType* host_id,
					 bool enable,
					 centreonscheduler__setHostObsessOverResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostPerfdataProcessingEnabled(soap* s,
							ns1__hostIDType* host_id,
							centreonscheduler__getHostPerfdataProcessingEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostPerfdataProcessingEnabled(soap* s,
							ns1__hostIDType* host_id,
							bool enable,
							centreonscheduler__setHostPerfdataProcessingEnabledResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostPluginExecutionTime(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__getHostPluginExecutionTimeResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostPluginIsExecuting(soap* s,
						ns1__hostIDType* host_id,
						centreonscheduler__getHostPluginIsExecutingResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostPluginLatency(soap* s,
					    ns1__hostIDType* host_id,
					    centreonscheduler__getHostPluginLatencyResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostPluginOutput(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__getHostPluginOutputResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = (host->plugin_output != NULL ? host->plugin_output : "");
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
int centreonscheduler__getHostPluginPerfdata(soap* s,
					     ns1__hostIDType* host_id,
					     centreonscheduler__getHostPluginPerfdataResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = (host->perf_data != NULL ? host->perf_data : "");
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
int centreonscheduler__getHostRetainStatusInformation(soap* s,
						      ns1__hostIDType* host_id,
						      centreonscheduler__getHostRetainStatusInformationResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostRetainNonStatusInformation(soap* s,
							 ns1__hostIDType* host_id,
							 centreonscheduler__getHostRetainNonStatusInformationResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostRetainStatusInformation(soap* s,
						      ns1__hostIDType* host_id,
						      bool enable,
						      centreonscheduler__setHostRetainStatusInformationResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostRetainNonStatusInformation(soap* s,
							 ns1__hostIDType* host_id,
							 bool enable,
							 centreonscheduler__setHostRetainNonStatusInformationResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostServicesCount(soap* s,
					    ns1__hostIDType* host_id,
					    centreonscheduler__getHostServicesCountResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostServicesTotalCheckInterval(soap* s,
							 ns1__hostIDType* host_id,
							 centreonscheduler__getHostServicesTotalCheckIntervalResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStalkOnDown(soap* s,
					  ns1__hostIDType* host_id,
					  centreonscheduler__getHostStalkOnDownResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStalkOnUnreachable(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__getHostStalkOnUnreachableResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStalkOnUp(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__getHostStalkOnUpResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostStalkOnDown(soap* s,
					  ns1__hostIDType* host_id,
					  bool enable,
					  centreonscheduler__setHostStalkOnDownResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostStalkOnUnreachable(soap* s,
						 ns1__hostIDType* host_id,
						 bool enable,
						 centreonscheduler__setHostStalkOnUnreachableResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
 *  @param[in]  enable true to enable, false to disable.
 *  @param[out] res     Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonscheduler__setHostStalkOnUp(soap* s,
					ns1__hostIDType* host_id,
					bool enable,
					centreonscheduler__setHostStalkOnUpResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStateCurrent(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__getHostStateCurrentResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStateInitial(soap* s,
					   ns1__hostIDType* host_id,
					   centreonscheduler__getHostStateInitialResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStateLast(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__getHostStateLastResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStateLastChange(soap* s,
					      ns1__hostIDType* host_id,
					      centreonscheduler__getHostStateLastChangeResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStateLastDown(soap* s,
					    ns1__hostIDType* host_id,
					    centreonscheduler__getHostStateLastDownResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStateLastHard(soap* s,
					    ns1__hostIDType* host_id,
					    centreonscheduler__getHostStateLastHardResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStateLastHardChange(soap* s,
						  ns1__hostIDType* host_id,
						  centreonscheduler__getHostStateLastHardChangeResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStateLastUnreachable(soap* s,
						   ns1__hostIDType* host_id,
						   centreonscheduler__getHostStateLastUnreachableResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStateLastUp(soap* s,
					  ns1__hostIDType* host_id,
					  centreonscheduler__getHostStateLastUpResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStatePercentChange(soap* s,
						 ns1__hostIDType* host_id,
						 centreonscheduler__getHostStatePercentChangeResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
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
int centreonscheduler__getHostStateType(soap* s,
					ns1__hostIDType* host_id,
					centreonscheduler__getHostStateTypeResponse& res) {
  (void)s;

  host* host = find_host(host_id->name.c_str());
  if (host == NULL) {
    res.error->code = 1;
    res.error->message = "Host `" + host_id->name + "' not found.";
    return (SOAP_OK);
  }

  res.value = host->state_type;
  res.error->code = 0;

  return (SOAP_OK);
}
