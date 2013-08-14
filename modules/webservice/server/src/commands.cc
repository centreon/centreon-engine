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

#include <sstream>
#include <string>
#include <string.h>
#include <strings.h>
#include "centreonengine.nsmap"
#include "soapH.h"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/modules/webservice/sync.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/objects/downtime.hh"
#include "com/centreon/engine/sretention.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Restart Engine.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__processRestart(soap* s,
                                   centreonengine__processRestartResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    sigrestart = true;
    logger(log_process_info, basic)
      << "Webservice: program restarting...";
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  webservice::sync::instance().worker_finish();
  return (SOAP_OK);
}

/**
 *  Shutdown Engine.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__processShutdown(soap* s,
                                    centreonengine__processShutdownResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    sigshutdown = true;
    logger(log_process_info, basic)
      << "Webservice: program shutting down...";
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  webservice::sync::instance().worker_finish();
  return (SOAP_OK);
}

/**
 *  Read state information.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__stateInformationLoad(soap* s,
                                         centreonengine__stateInformationLoadResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    read_initial_state_information();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  webservice::sync::instance().worker_finish();
  return (SOAP_OK);
}

/**
 *  Save state information.
 *
 *  @param[in]  s      Unused.
 *  @param[out] res    Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__stateInformationSave(soap* s,
                                         centreonengine__stateInformationSaveResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    save_state_information(false);

    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Acknowledge a problem on a host.
 *
 *  @param[in]  s                    Unused.
 *  @param[in]  host_id              Host to set data.
 *  @param[in]  acknowledgement_type Acknowledgement information.
 *  @param[out] res                  Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__acknowledgementOnHostAdd(soap* s,
                                             ns1__hostIDType* host_id,
                                             ns1__acknowledgementType* acknowledgement_type,
                                             centreonengine__acknowledgementOnHostAddResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << acknowledgement_type->author
      << ", " << acknowledgement_type->comment
      << ", " << acknowledgement_type->notify
      << ", " << acknowledgement_type->persistent
      << ", " << acknowledgement_type->sticky << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    int type = (acknowledgement_type->sticky == true ? ACKNOWLEDGEMENT_STICKY : ACKNOWLEDGEMENT_NORMAL);

    char* author = string::dup(acknowledgement_type->author.c_str());
    char* comment = string::dup(acknowledgement_type->comment.c_str());

    acknowledge_host_problem(host,
                             author,
                             comment,
                             type,
                             acknowledgement_type->notify,
                             acknowledgement_type->persistent);

    delete[] author;
    delete[] comment;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Remove an acknowledgement on a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__acknowledgementOnHostRemove(soap* s,
                                                ns1__hostIDType* host_id,
                                                centreonengine__acknowledgementOnHostRemoveResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << host_id->name << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    remove_host_acknowledgement(host);
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Acknowledge a problem on a service.
 *
 *  @param[in]  s                    Unused.
 *  @param[in]  service_id           Service to set data.
 *  @param[in]  acknowledgement_type Acknowledgement information.
 *  @param[out] res                  Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__acknowledgementOnServiceAdd(soap* s,
                                                ns1__serviceIDType* service_id,
                                                ns1__acknowledgementType* acknowledgement_type,
                                                centreonengine__acknowledgementOnServiceAddResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name
      << " } }, { " << acknowledgement_type->author
      << ", " << acknowledgement_type->comment
      << ", " << acknowledgement_type->notify
      << ", " << acknowledgement_type->persistent
      << ", " << acknowledgement_type->sticky << " })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    int type = (acknowledgement_type->sticky == true ? ACKNOWLEDGEMENT_STICKY : ACKNOWLEDGEMENT_NORMAL);

    char* author = string::dup(acknowledgement_type->author.c_str());
    char* comment = string::dup(acknowledgement_type->comment.c_str());

    acknowledge_service_problem(service,
                                author,
                                comment,
                                type,
                                acknowledgement_type->notify,
                                acknowledgement_type->persistent);
    delete[] author;
    delete[] comment;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Remove an acknowledgement on a service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__acknowledgementOnServiceRemove(soap* s,
                                                   ns1__serviceIDType* service_id,
                                                   centreonengine__acknowledgementOnServiceRemoveResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
<< "Webservice: " << __func__ << "({ " << service_id->service
<< ", { " << service_id->host->name << " } })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    remove_service_acknowledgement(service);
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Process a host check result.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  result_type       Process Result to check.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkHostProcessResult(soap* s,
                                           ns1__hostIDType* host_id,
                                           ns1__checkResultType* result_type,
                                           centreonengine__checkHostProcessResultResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << result_type->retval << ", ... })";

    logger(dbg_checks, most)
      << "Webservice: call " << __func__ << ": output=" << result_type->output;

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (process_passive_host_check(time(NULL),
                                   host->name,
                                   result_type->retval,
                                   result_type->output.c_str()) == ERROR) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' check process result failed.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a host check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkHostSchedule(soap* s,
                                      ns1__hostIDType* host_id,
                                      long delay,
                                      centreonengine__checkHostScheduleResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << delay << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    schedule_host_check(host, delay, CHECK_OPTION_NONE);
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a forced host check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkHostScheduleForced(soap* s,
                                            ns1__hostIDType* host_id,
                                            long delay,
                                            centreonengine__checkHostScheduleForcedResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << delay << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    schedule_host_check(host, delay, CHECK_OPTION_FORCE_EXECUTION);
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a check of all services associated with the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkHostScheduleServices(soap* s,
                                              ns1__hostIDType* host_id,
                                              long delay,
                                              centreonengine__checkHostScheduleServicesResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << delay << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
      if (tmp->service_ptr != NULL) {
        schedule_service_check(tmp->service_ptr, delay, CHECK_OPTION_NONE);
      }
    }
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a forced check of all services associated with the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Schedule's delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkHostScheduleServicesForced(soap* s,
                                                    ns1__hostIDType* host_id,
                                                    long delay,
                                                    centreonengine__checkHostScheduleServicesForcedResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << delay << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
      if (tmp->service_ptr != NULL) {
        schedule_service_check(tmp->service_ptr, delay, CHECK_OPTION_FORCE_EXECUTION);
      }
    }
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Process a service check result.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  result_type       Process Result to check.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkServiceProcessResult(soap* s,
                                              ns1__serviceIDType* service_id,
                                              ns1__checkResultType* result_type,
                                              centreonengine__checkServiceProcessResultResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name
      << " } }, { " << result_type->retval << ", ... })";

    logger(dbg_checks, most)
      << "Webservice: call " << __func__ << ": output=" << result_type->output;

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (process_passive_service_check(time(NULL),
                                      service_id->host->name.c_str(),
                                      service->description,
                                      result_type->retval,
                                      result_type->output.c_str()) == ERROR) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' check process result failed "
        + "for host `" + service_id->host->name + "'.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a service check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkServiceSchedule(soap* s,
                                         ns1__serviceIDType* service_id,
                                         long delay,
                                         centreonengine__checkServiceScheduleResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name << " } }, " << delay << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    schedule_service_check(service, delay, CHECK_OPTION_NONE);
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a forced service check.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  delay             Schedule delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__checkServiceScheduleForced(soap* s,
                                               ns1__serviceIDType* service_id,
                                               long delay,
                                               centreonengine__checkServiceScheduleForcedResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name << " } }, " << delay << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    schedule_service_check(service, delay, CHECK_OPTION_FORCE_EXECUTION);
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the author of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeGetAuthor(soap* s,
                                      ns1__downtimeIDType* downtime_id,
                                      std::string& val) {
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
    if (downtime == NULL) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    if (downtime->author != NULL) {
      val = downtime->author;
    }
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the duration of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeGetDuration(soap* s,
                                        ns1__downtimeIDType* downtime_id,
                                        double& val) {
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
    if (downtime == NULL) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = downtime->duration;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the end time of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeGetEnd(soap* s,
                                   ns1__downtimeIDType* downtime_id,
                                   unsigned long& val) {
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
    if (downtime == NULL) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = downtime->end_time;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if a downtime is fixed or not.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeGetFixed(soap* s,
                                     ns1__downtimeIDType* downtime_id,
                                     bool& val) {
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
    if (downtime == NULL) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = downtime->fixed;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Get the start time of a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to get data.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeGetStart(soap* s,
                                     ns1__downtimeIDType* downtime_id,
                                     unsigned long& val) {
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
 << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    scheduled_downtime* downtime = find_downtime(ANY_DOWNTIME, downtime_id->downtime);
    if (downtime == NULL) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    val = downtime->start_time;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Delete a downtime.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  downtime_id       Downtime to set data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeDelete(soap* s,
                                   ns1__downtimeIDType* downtime_id,
                                   centreonengine__downtimeDeleteResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << downtime_id->downtime << ")";

    if (unschedule_downtime(HOST_DOWNTIME, downtime_id->downtime) == ERROR
        && unschedule_downtime(SERVICE_DOWNTIME, downtime_id->downtime) == ERROR) {
      std::ostringstream oss;
      oss << downtime_id->downtime;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Downtime `" + oss.str() + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
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
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << downtime_type->starttime
      << ", " << downtime_type->endtime
      << ", " << downtime_type->fixed
      << ", " << downtime_type->triggerid->downtime
      << ", " << downtime_type->duration
      << ", " << downtime_type->author
      << ", " << downtime_type->comment << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    char* author = string::dup(downtime_type->author.c_str());
    char* comment = string::dup(downtime_type->comment.c_str());

    unsigned long downtime_id;
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
                          static_cast<unsigned long>(downtime_type->duration),
                          &downtime_id) == ERROR) {
      delete[] author;
      delete[] comment;
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' add downtime to host failed.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    res.downtimeid = soap_new_ns1__downtimeIDType(s, 1);
    res.downtimeid->downtime = downtime_id;

    delete[] author;
    delete[] comment;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
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
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << downtime_type->starttime
      << ", " << downtime_type->endtime
      << ", " << downtime_type->fixed
      << ", " << downtime_type->triggerid->downtime
      << ", " << downtime_type->duration
      << ", " << downtime_type->author
      << ", " << downtime_type->comment << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    time_t entry_time = time(NULL);

    char* author = string::dup(downtime_type->author.c_str());
    char* comment = string::dup(downtime_type->comment.c_str());

    unsigned long downtime_id;
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
                          static_cast<unsigned long>(downtime_type->duration),
                          &downtime_id) == ERROR) {

      delete[] author;
      delete[] comment;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' add downtime faild.";
      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    res.downtimeid = soap_new_ns1__downtimeIDType(s, 1);
    res.downtimeid->downtime = downtime_id;

    schedule_and_propagate_downtime(host,
                                    entry_time,
                                    author,
                                    comment,
                                    downtime_type->starttime,
                                    downtime_type->endtime,
                                    downtime_type->fixed,
                                    0,
                                    static_cast<unsigned long>(downtime_type->duration));

    delete[] author;
    delete[] comment;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
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
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << downtime_type->starttime
      << ", " << downtime_type->endtime
      << ", " << downtime_type->fixed
      << ", " << downtime_type->triggerid->downtime
      << ", " << downtime_type->duration
      << ", " << downtime_type->author
      << ", " << downtime_type->comment << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    time_t entry_time = time(NULL);

    char* author = string::dup(downtime_type->author.c_str());
    char* comment = string::dup(downtime_type->comment.c_str());

    unsigned long downtime_id;
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
                          static_cast<unsigned long>(downtime_type->duration),
                          &downtime_id) == ERROR) {
      delete[] author;
      delete[] comment;

      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' add downtime faild.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    res.downtimeid = soap_new_ns1__downtimeIDType(s, 1);
    res.downtimeid->downtime = downtime_id;

    schedule_and_propagate_downtime(host,
                                    entry_time,
                                    author,
                                    comment,
                                    downtime_type->starttime,
                                    downtime_type->endtime,
                                    downtime_type->fixed,
                                    res.downtimeid->downtime,
                                    static_cast<unsigned long>(downtime_type->duration));

    delete[] author;
    delete[] comment;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Schedule a downtime on all services of a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  downtime_type     Downtime information.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__downtimeAddToHostServices(soap* s,
                                              ns1__hostIDType* host_id,
                                              ns1__downtimeType* downtime_type,
                                              centreonengine__downtimeAddToHostServicesResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << downtime_type->starttime
      << ", " << downtime_type->endtime
      << ", " << downtime_type->fixed
      << ", " << downtime_type->triggerid->downtime
      << ", " << downtime_type->duration
      << ", " << downtime_type->author
      << ", " << downtime_type->comment << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }


    time_t entry_time = time(NULL);

    char* author = string::dup(downtime_type->author.c_str());
    char* comment = string::dup(downtime_type->comment.c_str());
    bool is_error = false;

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
                              static_cast<unsigned long>(downtime_type->duration),
                              NULL) == ERROR) {
          is_error = true;
        }
      }
    }

    delete[] author;
    delete[] comment;

    if (is_error == 0) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' one or more service cannot schedule downtime.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
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
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << service_id->service
      << ", { " << service_id->host->name
      << " } }, { " << downtime_type->starttime
      << ", " << downtime_type->endtime
      << ", " << downtime_type->fixed
      << ", " << downtime_type->triggerid->downtime
      << ", " << downtime_type->duration
      << ", " << downtime_type->author
      << ", " << downtime_type->comment << " })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    char* author = string::dup(downtime_type->author.c_str());
    char* comment = string::dup(downtime_type->comment.c_str());

    unsigned long downtime_id;
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
                          static_cast<unsigned long>(downtime_type->duration),
                          &downtime_id) == ERROR) {
      delete[] author;
      delete[] comment;
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service + "' add downtime to service failed.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }
    res.downtimeid = soap_new_ns1__downtimeIDType(s, 1);
    res.downtimeid->downtime = downtime_id;

    delete[] author;
    delete[] comment;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Delay a host notification.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  delay             Notification delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__notificationHostDelay(soap* s,
                                          ns1__hostIDType* host_id,
                                          long delay,
                                          centreonengine__notificationHostDelayResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", " << delay << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    host->next_host_notification = delay;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Send a notification on a host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  notification_type Notification information.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__notificationHostSend(soap* s,
                                         ns1__hostIDType* host_id,
                                         ns1__notificationType* notification_type,
                                         centreonengine__notificationHostSendResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << host_id->name
      << ", { " << notification_type->author
      << ", " << notification_type->broadcast
      << ", " << notification_type->comment
      << ", " << notification_type->forced
      << ", " << notification_type->increment << " })";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    int options = (notification_type->broadcast == true ? NOTIFICATION_OPTION_BROADCAST : 0)
      | (notification_type->forced == true ? NOTIFICATION_OPTION_FORCED : 0)
      | (notification_type->increment == true ? NOTIFICATION_OPTION_INCREMENT : 0);

    char* author = string::dup(notification_type->author.c_str());
    char* comment = string::dup(notification_type->comment.c_str());

    if (host_notification(host,
                          NOTIFICATION_CUSTOM,
                          author,
                          comment,
                          options) == ERROR) {
      delete[] author;
      delete[] comment;
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' send notification failed.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] author;
    delete[] comment;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Delay a service notification.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  delay             Notification delay.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__notificationServiceDelay(soap* s,
                                             ns1__serviceIDType* service_id,
                                             long delay,
                                             centreonengine__notificationServiceDelayResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name << " } }, " << delay << ")";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    service->next_notification = delay;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Send a notification on a service.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  service_id        Service to set data.
 *  @param[in]  notification_type Notification information.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__notificationServiceSend(soap* s,
                                            ns1__serviceIDType* service_id,
                                            ns1__notificationType* notification_type,
                                            centreonengine__notificationServiceSendResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "({ " << service_id->service
      << ", { " << service_id->host->name
      << " } }, { " << notification_type->author
      << ", " << notification_type->broadcast
      << ", " << notification_type->comment
      << ", " << notification_type->forced
      << ", " << notification_type->increment << " })";

    service* service = find_service(service_id->host->name.c_str(), service_id->service.c_str());
    if (service == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service
        + "' with Host `" + service_id->host->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    int options = (notification_type->broadcast == true ? NOTIFICATION_OPTION_BROADCAST : 0)
      | (notification_type->forced == true ? NOTIFICATION_OPTION_FORCED : 0)
      | (notification_type->increment == true ? NOTIFICATION_OPTION_INCREMENT : 0);


    char* author = string::dup(notification_type->author.c_str());
    char* comment = string::dup(notification_type->comment.c_str());

    if (service_notification(service,
                             NOTIFICATION_CUSTOM,
                             author,
                             comment,
                             options) == ERROR) {
      delete[] author;
      delete[] comment;
      std::string* error = soap_new_std__string(s, 1);
      *error = "Service `" + service_id->service + "' send notification failed.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] author;
    delete[] comment;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

// /**
//  *  Dump all object list.
//  *
//  *  @param[in]  s                 Unused.
//  *  @param[out] res               Unused.
//  *
//  *  @return SOAP_OK on success.
//  */
// int centreonengine__dumpObjectList(soap* s,
//                                    centreonengine__dumpObjectListResponse& res) {
//   (void)res;

//   try {
//     webservice::sync::instance().wait_thread_safeness();
//     logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
//     com::centreon::engine::logging::dump_object_list();
//     webservice::sync::instance().worker_finish();
//   }
//   catch (std::exception const& e) {
//     logger(dbg_commands, most)
//       << "Webservice: " << __func__ << " failed: " << e.what() << ".";
//     webservice::sync::instance().worker_finish();
//     return (soap_receiver_fault(s, "invalid argument", e.what()));
//   }
//   catch (...) {
//     logger(dbg_commands, most)
//       << "Webservice: " << __func__ << " failed. catch all.";
//     webservice::sync::instance().worker_finish();
//     return (soap_receiver_fault(s, "Runtime error.", "catch all"));

//   }
//   return (SOAP_OK);
// }

/**
 *  Check if event handlers are enabled globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getEventHandlersEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = enable_event_handlers;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if failure prediction is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getFailurePredictionEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = enable_failure_prediction;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if flap detection is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getFlapDetectionEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = enable_flap_detection;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if hosts active checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getHostsChecksActiveEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = execute_host_checks;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if hosts passive checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getHostsChecksPassiveEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = accept_passive_host_checks;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
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

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    res.command = soap_new_ns1__commandIDType(s, 1);
    res.command->command = global_host_event_handler;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if the hosts freshness checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getHostsFreshnessChecksEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = check_host_freshness;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if host obsession is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getHostsObsessOverEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = obsess_over_hosts;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if notifications are enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getNotificationsEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = enable_notifications;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if performance data processing is enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getPerfdataProcessingEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = process_performance_data;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if services active checks are enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getServicesChecksActiveEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = execute_service_checks;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if services passive checks are enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getServicesChecksPassiveEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = accept_passive_service_checks;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
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

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    res.command = soap_new_ns1__commandIDType(s, 1);
    res.command->command = global_service_event_handler;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if services freshness checks are globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getServicesFreshnessChecksEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = check_service_freshness;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Check if services obsession is globally enabled.
 *
 *  @param[in]  s                 Unused.
 *  @param[out] val               Result of operation.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__getServicesObsessOverEnabled(soap* s, bool& val) {
  (void)s;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    val = obsess_over_services;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable event handlers globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setEventHandlersEnabled(soap* s,
                                            bool enable,
                                            centreonengine__setEventHandlersEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "()";
    enable_event_handlers = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable failure prediction globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setFailurePredictionEnabled(soap* s,
                                                bool enable,
                                                centreonengine__setFailurePredictionEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    enable_failure_prediction = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setFlapDetectionEnabled(soap* s,
                                            bool enable,
                                            centreonengine__setFlapDetectionEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    enable_flap_detection = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable active host checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostsChecksActiveEnabled(soap* s,
                                                bool enable,
                                                centreonengine__setHostsChecksActiveEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    execute_host_checks = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable passive host checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostsChecksPassiveEnabled(soap* s,
                                                 bool enable,
                                                 centreonengine__setHostsChecksPassiveEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    accept_passive_host_checks = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Change the global host event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  command_id        Command to get data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostsEventHandler(soap* s,
                                         ns1__commandIDType* command_id,
                                         centreonengine__setHostsEventHandlerResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << command_id->command << ")";

    command* command = find_command(command_id->command.c_str());
    if (command == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Command `" + command_id->command + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] global_host_event_handler;
    global_host_event_handler = string::dup(command_id->command.c_str());
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
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable host freshness checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostsFreshnessChecksEnabled(soap* s,
                                                   bool enable,
                                                   centreonengine__setHostsFreshnessChecksEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    check_host_freshness = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable obsession over host checks.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostsObsessOverEnabled(soap* s,
                                              bool enable,
                                              centreonengine__setHostsObsessOverEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    obsess_over_hosts = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setNotificationsEnabled(soap* s,
                                            bool enable,
                                            centreonengine__setNotificationsEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    enable_notifications = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable performance data processing globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setPerfdataProcessingEnabled(soap* s,
                                                 bool enable,
                                                 centreonengine__setPerfdataProcessingEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    process_performance_data = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable active service checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServicesChecksActiveEnabled(soap* s,
                                                   bool enable,
                                                   centreonengine__setServicesChecksActiveEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    execute_service_checks = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable passive service checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServicesChecksPassiveEnabled(soap* s,
                                                    bool enable,
                                                    centreonengine__setServicesChecksPassiveEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    accept_passive_service_checks = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Change the global service event handler.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  command_id        Command to get data.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServicesEventHandler(soap* s,
                                            ns1__commandIDType* command_id,
                                            centreonengine__setServicesEventHandlerResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << command_id->command << ")";

    command* command = find_command(command_id->command.c_str());
    if (command == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Command `" + command_id->command + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    delete[] global_service_event_handler;
    global_service_event_handler = string::dup(command_id->command.c_str());
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
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable service freshness checks globally.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServicesFreshnessChecksEnabled(soap* s,
                                                      bool enable,
                                                      centreonengine__setServicesFreshnessChecksEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    check_service_freshness = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable obsession over service checks.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServicesObsessOverEnabled(soap* s,
                                                 bool enable,
                                                 centreonengine__setServicesObsessOverEnabledResponse& res) {
  (void)s;
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most) << "Webservice: " << __func__ << "(" << enable << ")";
    obsess_over_services = enable;
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable active checks on all services of the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetServicesCheckActiveEnabled(soap* s,
                                                      ns1__hostIDType* host_id,
                                                      bool enable,
                                                      centreonengine__hostSetServicesCheckActiveEnabledResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
      if (tmp->service_ptr != NULL) {
        tmp->service_ptr->checks_enabled = enable;
      }
    }
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on all services of the host.
 *
 *  @param[in]  s                 Unused.
 *  @param[in]  host_id           Host to set data.
 *  @param[in]  enable            true to enable, false to disable.
 *  @param[out] res               Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetServicesNotificationsEnabled(soap* s,
                                                        ns1__hostIDType* host_id,
                                                        bool enable,
                                                        centreonengine__hostSetServicesNotificationsEnabledResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
                   << "Webservice: " << __func__ << "(" << host_id->name
		   << ", " << enable << ")";

    host* host = find_host(host_id->name.c_str());
    if (host == NULL) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "Host `" + host_id->name + "' not found.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    for (servicesmember* tmp = host->services; tmp != NULL; tmp = tmp->next) {
      if (tmp->service_ptr != NULL) {
        tmp->service_ptr->notifications_enabled = enable;
      }
    }
    webservice::sync::instance().worker_finish();
  }
  catch (...) {
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. catch all.";
    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Runtime error.", "catch all"));
  }
  return (SOAP_OK);
}
