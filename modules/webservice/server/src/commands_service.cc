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

#include <algorithm>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/webservice/commands.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/objects/service.hh"
#include "soapH.h"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Find target service.
 *
 *  @param[in] hst Host name.
 *  @param[in] svc Service description.
 *
 *  @return Service object.
 */
static service* find_target_service(char const* hst, char const* svc) {
  service* svcptr(find_service(hst, svc));
  if (!svcptr)
    throw (engine_error() << "cannot find service ('" << hst << "', '"
           << svc << "')");
  return (svcptr);
}

/**
 *  Notify event broker of service update.
 *
 *  @param[in] svc Service object.
 */
static void notify_event_broker(service* svc) {
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_service_data(
    NEBTYPE_SERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);
  return ;
}

/**************************************
*                                     *
*           Acknowledgement           *
*                                     *
**************************************/

/**
 *  Get whether or not the service is acknowledged.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if service is acknowledged.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetAcknowledgementIsSet(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
                 service_id->service.c_str()));

  // Get requested value.
  res = (svc->acknowledgement_type != ACKNOWLEDGEMENT_NONE);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the current type of the acknowledgement on a service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Acknowledgement type.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetAcknowledgementType(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned short& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->acknowledgement_type;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*                Check                *
*                                     *
**************************************/

/**
 *  Check if active checks are enabled on the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if active checks are enabled on service.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckActiveEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->checks_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the service check command.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Check command.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckCommand(
      soap* s,
      ns1__serviceIDType* service_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  if (svc->service_check_command)
    res = svc->service_check_command;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the current check attempt of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Current check attempt.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckCurrentAttempt(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->current_attempt;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the normal check interval.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Normal check interval.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckIntervalNormal(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = static_cast<unsigned int>(svc->check_interval);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the retry check interval.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Retry check interval.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckIntervalRetry(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = static_cast<unsigned int>(svc->retry_interval);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the date when the last check was executed.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Time at which the last service check was
 *                         executed.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckLast(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->last_check;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the max check attempts of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Max check attempt.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckMaxAttempts(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->max_attempts;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the time at which the next service check is scheduled to run.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Time at which the next check will be
 *                         executed.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckNext(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->next_check;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the service check options.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Check options.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckOptions(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->check_options;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if passive checks are enabled on this service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if passive checks are enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckPassiveEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->accept_passive_service_checks;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the service check period.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Service check period.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckPeriod(
      soap* s,
      ns1__serviceIDType* service_id,
      centreonengine__serviceGetCheckPeriodResponse& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Set check period.
  if (svc->check_period) {
    res.val = soap_new_ns1__timeperiodIDType(s, 1);
    res.val->name = svc->check_period;
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if the service should be scheduled.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if service should be scheduled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckShouldBeScheduled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->should_be_scheduled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the type of the service check.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Check type.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCheckType(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned short& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->check_type;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable active checks on the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckActiveEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetCheckActiveEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable active service checks.
  svc->checks_enabled = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the service check command.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  cmd        New check command.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckCommand(
      soap* s,
      ns1__serviceIDType* service_id,
      std::string cmd,
      centreonengine__serviceSetCheckCommandResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Update check command.
  if (!cmd.empty()) {
    // Find target command.
    command* cmd_ptr(find_command(cmd.c_str()));
    if (!cmd_ptr)
      throw (engine_error()
             << "cannot update check command of service ('"
             << service_id->host->name << "', '" << service_id->service
             << "'): command '" << cmd << "' does not exist");

    // Set new check command.
    delete [] svc->service_check_command;
    svc->service_check_command = my_strdup(cmd.c_str());
    svc->check_command_ptr = cmd_ptr;
  }
  // Remove check command.
  else {
    delete [] svc->service_check_command;
    svc->service_check_command = NULL;
    svc->check_command_ptr = NULL;
  }

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the normal check interval of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  interval   Normal check interval.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckIntervalNormal(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int interval,
      centreonengine__serviceSetCheckIntervalNormalResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Set new check interval.
  svc->check_interval = interval;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the normal check interval of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  interval   Retry check interval.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckIntervalRetry(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int interval,
      centreonengine__serviceSetCheckIntervalRetryResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Set new check interval.
  svc->retry_interval = interval;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the max check attempts of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  attempts   Max check attempts.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckMaxAttempts(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int attempts,
      centreonengine__serviceSetCheckMaxAttemptsResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Max check attempts must be greater than 0.
  if (!attempts)
    throw (engine_error()
           << "attempt to set max check attempts of service ('"
           << service_id->host->name << "', '" << service_id->service
           << "') to 0, which is forbidden");

  // Update max check attempts.
  svc->max_attempts = attempts;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable host passive checks.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckPassiveEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetCheckPassiveEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable passive checks.
  svc->accept_passive_service_checks = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Update or remove check timeperiod.
 *
 *  @param[in]  s             SOAP object.
 *  @param[in]  service_id    Target service.
 *  @param[in]  timeperiod_id Target timeperiod.
 *  @param[out] res           Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCheckPeriod(
      soap* s,
      ns1__serviceIDType* service_id,
      ns1__timeperiodIDType* timeperiod_id,
      centreonengine__serviceSetCheckPeriodResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
                 service_id->service.c_str()));

  // Update timeperiod.
  if (!timeperiod_id->name.empty()) {
    // Find target timeperiod.
    timeperiod*
      tmprd(find_timeperiod(timeperiod_id->name.c_str()));
    if (!tmprd)
      throw (engine_error()
             << "cannot update check period of service ('"
             << service_id->host->name << "', '" << service_id->service
             << "'): timeperiod '" << timeperiod_id->name
             << "' does not exist");

    // Set new timeperiod.
    delete [] svc->check_period;
    svc->check_period = my_strdup(timeperiod_id->name.c_str());
    svc->check_period_ptr = tmprd;
  }
  // Remove timeperiod.
  else {
    delete [] svc->check_period;
    svc->check_period = NULL;
    svc->check_period_ptr = NULL;
  }

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*           Custom Variable           *
*                                     *
**************************************/

/**
 *  Get the value of a service custom variable.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  variable   Custom variable name.
 *  @param[out] res        Custom variable value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetCustomVariable(
      soap* s,
      ns1__serviceIDType* service_id,
      std::string variable,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Browse custom variables.
  res.clear();
  for(customvariablesmember* tmp(svc->custom_variables);
      tmp;
      tmp = tmp->next)
    if (tmp->variable_name
        && !strcasecmp(tmp->variable_name, variable.c_str())
        && tmp->variable_value) {
      res = tmp->variable_value;
      break ;
    }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set a custom variable.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  varname    Target variable.
 *  @param[in]  varvalue   New variable value.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetCustomVariable(
      soap* s,
      ns1__serviceIDType* service_id,
      std::string varname,
      std::string varvalue,
      centreonengine__serviceSetCustomVariableResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                << ", " << service_id->service << "}, " << varname
                << ", " << varvalue)

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
                 service_id->service.c_str()));

  // Find existing custom variable.
  customvariablesmember** cvar;
  for (cvar = &svc->custom_variables; *cvar; cvar = &(*cvar)->next)
    if ((*cvar)->variable_name
        && !strcmp((*cvar)->variable_name, varname.c_str()))
      break ;

  // Update variable.
  if (!varvalue.empty()) {
    // Create new variable if not existing.
    if (!*cvar) {
      *cvar = new customvariablesmember;
      (*cvar)->next = NULL;
      (*cvar)->variable_name = my_strdup(varname.c_str());
    }
    else {
      delete [] (*cvar)->variable_value;
      (*cvar)->variable_value = NULL;
    }

    // Set new value.
    (*cvar)->variable_value = my_strdup(varvalue.c_str());
  }
  // Delete variable.
  if (*cvar) {
    customvariablesmember* to_delete(*cvar);
    *cvar = (*cvar)->next;
    delete [] to_delete->variable_name;
    delete [] to_delete->variable_value;
    delete to_delete;
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Downtime               *
*                                     *
**************************************/

/**
 *  Get the service downtime depth.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Downtime depth.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetDowntimeDepth(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->scheduled_downtime_depth;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if service has a pending flexible downtime.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if service has pending flexible
 *                         downtime.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetDowntimeFlexPending(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->pending_flex_downtime;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*            Event handler            *
*                                     *
**************************************/

/**
 *  Get the service event handler.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Event handler.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetEventHandler(
      soap* s,
      ns1__serviceIDType* service_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get event handler.
  if (svc->event_handler)
    res = svc->event_handler;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if the service event handler is enabled.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if event handler is enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetEventHandlerEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->event_handler_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the service event handler.
 *
 *  @param[in]  s             SOAP object.
 *  @param[in]  service_id    Target service.
 *  @param[in]  event_handler Event handler.
 *  @param[out] res           Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetEventHandler(
      soap* s,
      ns1__serviceIDType* service_id,
      std::string event_handler,
      centreonengine__serviceSetEventHandlerResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Update event handler.
  if (!event_handler.empty()) {
    // Find target command.
    command* cmd(find_command(event_handler.c_str()));
    if (!cmd)
      throw (engine_error()
             << "cannot update event handler of service ('"
             << service_id->host->name << "', '" << service_id->service
             << "': command '" << event_handler << "' does not exist");

    // Set new event handler.
    delete [] svc->event_handler;
    svc->event_handler = my_strdup(event_handler.c_str());
    svc->event_handler_ptr = cmd;
  }
  // Remove event handler.
  else {
    delete [] svc->event_handler;
    svc->event_handler = NULL;
    svc->event_handler_ptr = NULL;
  }

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable service event handler.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetEventHandlerEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetEventHandlerEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable event handler.
  svc->event_handler_enabled = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*         Failure Prediction          *
*                                     *
**************************************/

/**
 *  Check if failure prediction is enabled on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if failure prediction is enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFailurePredictionEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->failure_prediction_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get service failure prediction options.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Failure prediction options.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFailurePredictionOptions(
      soap* s,
      ns1__serviceIDType* service_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  if (svc->failure_prediction_options)
    res = svc->failure_prediction_options;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable failure prediction on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFailurePredictionEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetFailurePredictionEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable failure prediction.
  svc->failure_prediction_enabled = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*           Flap Detection            *
*                                     *
**************************************/

/**
 *  Get the flap detection comment ID of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Flap detection comment ID.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionCommentID(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->flapping_comment_id;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check whether flap detection is enabled on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if flap detection is enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->flap_detection_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if service is flapping.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if service is flapping.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionIsFlapping(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->is_flapping;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on critical state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if flap detection is enabled on critical
 *                         states.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionOnCritical(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->flap_detection_on_critical;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on ok state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if flap detection is enabled on ok
 *                         states.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionOnOk(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->flap_detection_on_ok;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on unknown state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if flap detection is enabled on unknown
 *                         states.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionOnUnknown(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->flap_detection_on_unknown;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on warning state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if flap detection is enabled on warning
 *                         states.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionOnWarning(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->flap_detection_on_warning;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the service high flap threshold.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        High flap threshold.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionThresholdHigh(
      soap* s,
      ns1__serviceIDType* service_id,
      double& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->high_flap_threshold;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the service low flap threshold.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Low flap threshold.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFlapDetectionThresholdLow(
      soap* s,
      ns1__serviceIDType* service_id,
      double& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->low_flap_threshold;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetFlapDetectionEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable flap detection.
  svc->flap_detection_enabled = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on critical state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionOnCritical(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetFlapDetectionOnCriticalResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable flap detection on critical states.
  svc->flap_detection_on_critical = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on ok state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionOnOk(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetFlapDetectionOnOkResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable flap detection on OK states.
  svc->flap_detection_on_ok = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on unknown state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionOnUnknown(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetFlapDetectionOnUnknownResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable flap detection on unknown states.
  svc->flap_detection_on_unknown = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on warning state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionOnWarning(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetFlapDetectionOnWarningResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable flap detection on warning states.
  svc->flap_detection_on_warning = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the high flap threshold of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  threshold  New threshold.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionThresholdHigh(
      soap* s,
      ns1__serviceIDType* service_id,
      double threshold,
      centreonengine__serviceSetFlapDetectionThresholdHighResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Update high flap threshold.
  svc->high_flap_threshold = threshold;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the low flap threshold of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  threshold  New threshold.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFlapDetectionThresholdLow(
      soap* s,
      ns1__serviceIDType* service_id,
      double threshold,
      centreonengine__serviceSetFlapDetectionThresholdLowResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Update low flap threshold.
  svc->low_flap_threshold = threshold;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Freshness              *
*                                     *
**************************************/

/**
 *  Check if freshness checks are enabled on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if freshness check is enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFreshnessCheckEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->check_freshness;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if freshness check is active on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if freshness is active.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFreshnessIsActive(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->is_being_freshened;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the freshness threshold of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Freshness threshold.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetFreshnessThreshold(
      soap* s,
      ns1__serviceIDType* service_id,
      int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->freshness_threshold;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable freshness checks on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFreshnessCheckEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetFreshnessCheckEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable freshness check.
  svc->check_freshness = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the service freshness threshold.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  threshold  New threshold.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetFreshnessThreshold(
      soap* s,
      ns1__serviceIDType* service_id,
      int threshold,
      centreonengine__serviceSetFreshnessThresholdResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Update freshness threshold.
  svc->freshness_threshold = threshold;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*         Modified Attributes         *
*                                     *
**************************************/

/**
 *  Get the modified attributes on the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Modified attributes.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetModifiedAttributes(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->modified_attributes;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*                Name                 *
*                                     *
**************************************/

/**
 *  Get the service display name.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Display name.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNameDisplay(
      soap* s,
      ns1__serviceIDType* service_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  if (svc->display_name)
    res = svc->display_name;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the display name of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  name       Service's display name.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNameDisplay(
      soap* s,
      ns1__serviceIDType* service_id,
      std::string name,
      centreonengine__serviceSetNameDisplayResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  delete [] svc->display_name;
  svc->display_name = my_strdup(name.c_str());

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*            Notification             *
*                                     *
**************************************/

/**
 *  Get the ID of the current service notification.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsCurrentID(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->current_notification_id;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the current notification number of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Current notification number.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsCurrentNumber(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->current_notification_number;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are enabled on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if notifications are enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->notifications_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the first notification delay of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        First notification delay.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsFirstDelay(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = static_cast<unsigned int>(svc->first_notification_delay);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the notification interval of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Notification interval.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsInterval(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Notification interval.
  res = static_cast<unsigned int>(svc->notification_interval);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the time at which the last notification was sent.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Time at which the last notification was sent.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsLast(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->last_notification;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the time at which the next notification will be sent.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Time at which the next notification will be
 *                         sent.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsNext(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->next_notification;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is critical.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if notifications are sent on critical
 *                         state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnCritical(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->notify_on_critical;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is on downtime.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if notifications are sent on downtime.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnDowntime(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->notify_on_downtime;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is flappy.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if notifications are sent when service
 *                         is flapping.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnFlapping(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->notify_on_flapping;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service recovers.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if notifications are sent on service
 *                         recovery.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnRecovery(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->notify_on_recovery;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is unknown.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if notifications are sent for unknown
 *                         states.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnUnknown(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->notify_on_unknown;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if service is warning.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if notifications are sent for warning
 *                         states.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsOnWarning(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->notify_on_warning;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the notification period of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Notification period.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetNotificationsPeriod(
      soap* s,
      ns1__serviceIDType* service_id,
      centreonengine__serviceGetNotificationsPeriodResponse& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get notification period.
  if (svc->notification_period) {
    res.val = soap_new_ns1__timeperiodIDType(s, 1);
    res.val->name = svc->notification_period;
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetNotificationsEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable notifications.
  svc->notifications_enabled = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the time after which the first service notification will be sent.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  delay      Delay of the first notification.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsFirstDelay(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int delay,
      centreonengine__serviceSetNotificationsFirstDelayResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Set first notification delay.
  svc->first_notification_delay = delay;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the notification interval of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  interval   Notification interval.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsInterval(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int interval,
      centreonengine__serviceSetNotificationsIntervalResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Set notification interval.
  svc->notification_interval = interval;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is critical.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnCritical(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetNotificationsOnCriticalResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable notifications on critical states.
  svc->notify_on_critical = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is in downtime.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnDowntime(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetNotificationsOnDowntimeResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable notifications on downtimes.
  svc->notify_on_downtime = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is flappy.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnFlapping(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetNotificationsOnFlappingResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable notifications when service flaps.
  svc->notify_on_flapping = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service recovers.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnRecovery(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetNotificationsOnRecoveryResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable notifications on recovery.
  svc->notify_on_recovery = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is unknown.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnUnknown(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetNotificationsOnUnknownResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable notifications on unknown state.
  svc->notify_on_unknown = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when service is warning.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsOnWarning(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetNotificationsOnWarningResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable notifications on warning states.
  svc->notify_on_warning = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Update or remove notification period.
 *
 *  @param[in]  s
 *  @param[in]  service_id
 *  @param[in]  timeperiod_id
 *  @param[out] res
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetNotificationsPeriod(
      soap* s,
      ns1__serviceIDType* service_id,
      ns1__timeperiodIDType* timeperiod_id,
      centreonengine__serviceSetNotificationsPeriodResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
                 service_id->service.c_str()));

  // Update notification period.
  if (!timeperiod_id->name.empty()) {
    // Find target period.
    timeperiod*
      tmprd(find_timeperiod(timeperiod_id->name.c_str()));
    if (!tmprd)
      throw (engine_error()
             << "cannot update notification period of service ('"
             << service_id->host->name << "', '" << service_id->service
             << "'): timeperiod '" << timeperiod_id->name
             << "' does not exist");

    // Set new timeperiod.
    delete [] svc->notification_period;
    svc->notification_period
      = my_strdup(timeperiod_id->name.c_str());
    svc->notification_period_ptr = tmprd;
  }
  // Remove notification period.
  else {
    delete [] svc->notification_period;
    svc->notification_period = NULL;
    svc->notification_period_ptr = NULL;
  }

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Obsession              *
*                                     *
**************************************/

/**
 *  Check whether or not service is being obsessed over.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if service is being obsessed over.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetObsessOver(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->obsess_over_service;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable service obsession.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetObsessOver(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetObsessOverResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable obsession.
  svc->obsess_over_service = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Perfdata               *
*                                     *
**************************************/

/**
 *  Check if perfdata processing is enabled on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if performance data are processed.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPerfdataProcessingEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->process_performance_data;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable perfdata processing.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetPerfdataProcessingEnabled(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetPerfdataProcessingEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable perfdata processing.
  svc->process_performance_data = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*               Plugin                *
*                                     *
**************************************/

/**
 *  Get the last execution time of the plugin.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Plugin execution time.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPluginExecutionTime(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = static_cast<unsigned int>(svc->execution_time);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if service check if currently executing.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if plugin is executing.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPluginIsExecuting(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->is_executing;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the service latency.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Latency.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPluginLatency(
      soap* s,
      ns1__serviceIDType* service_id,
      double& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->latency;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the plugin output.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Plugin output.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPluginOutput(
      soap* s,
      ns1__serviceIDType* service_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  if (svc->plugin_output)
    res = svc->plugin_output;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the plugin perfdata.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Performance data.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetPluginPerfdata(
      soap* s,
      ns1__serviceIDType* service_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  if (svc->perf_data)
    res = svc->perf_data;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Retention              *
*                                     *
**************************************/

/**
 *  Check if service status information are retained.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if status information is retained.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetRetainStatusInformation(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->retain_status_information;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if service non status information are retained.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if non-status information is retained.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetRetainNonStatusInformation(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->retain_nonstatus_information;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable status information retention on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetRetainStatusInformation(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetRetainStatusInformationResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable status retention.
  svc->retain_status_information = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable non status information retention on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetRetainNonStatusInformation(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetRetainNonStatusInformationResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable non-status retention.
  svc->retain_nonstatus_information = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Stalking               *
*                                     *
**************************************/

/**
 *  Check if stalking on critical is enabled on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if stalking is enabled on critical
 *                         states.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStalkOnCritical(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->stalk_on_critical;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if stalking on ok is enabled on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if stalking is enabled on ok states.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStalkOnOk(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->stalk_on_ok;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if stalking on unknown is enabled on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if stalking is enabled on unknown
 *                         states.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStalkOnUnknown(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->stalk_on_unknown;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if stalking on warning is enabled on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        true if stalking is enabled on warning
 *                         states.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStalkOnWarning(
      soap* s,
      ns1__serviceIDType* service_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->stalk_on_warning;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on critical.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetStalkOnCritical(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetStalkOnCriticalResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable stalking on critical states.
  svc->stalk_on_critical = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on ok.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetStalkOnOk(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetStalkOnOkResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable stalking on ok states.
  svc->stalk_on_ok = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on unknown.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetStalkOnUnknown(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetStalkOnUnknownResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable stalking on unknown states.
  svc->stalk_on_unknown = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on warning.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceSetStalkOnWarning(
      soap* s,
      ns1__serviceIDType* service_id,
      bool enable,
      centreonengine__serviceSetStalkOnWarningResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Enable or disable stalking on warning states.
  svc->stalk_on_warning = enable;

  // Notify event broker.
  notify_event_broker(svc);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*                State                *
*                                     *
**************************************/

/**
 *  Get the current state of the state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Current state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateCurrent(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned short& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->current_state;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the initial state of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Initial state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateInitial(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned short& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->initial_state;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last service state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Last state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLast(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned short& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->last_state;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the state changed.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Time at which service last changed state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastChange(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->last_state_change;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the service was in a critical state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Last time service was critical.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastCritical(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->last_time_critical;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the service last hard state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Last hard state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastHard(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->last_hard_state;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time at which the hard state changed.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Last time service changed hard state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastHardChange(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->last_hard_state_change;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the service was in an ok state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Last time service was ok.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastOk(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->last_time_ok;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the service was in an unknown state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Last time service was unknown.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastUnknown(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->last_time_unknown;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the service was in a warning state.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Last time service was warning.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateLastWarning(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->last_time_warning;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the percent state change of the service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        Percent state change.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStatePercentChange(
      soap* s,
      ns1__serviceIDType* service_id,
      ULONG64& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = static_cast<time_t>(svc->percent_state_change);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get state type (hard or soft).
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  service_id Target service.
 *  @param[out] res        State type.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__serviceGetStateType(
      soap* s,
      ns1__serviceIDType* service_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN("{" << service_id->host->name
                  << ", " << service_id->service << "}")

  // Find target service.
  service* svc(find_target_service(
                 service_id->host->name.c_str(),
		 service_id->service.c_str()));

  // Get requested value.
  res = svc->state_type;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}
