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
#include "com/centreon/engine/configuration/applier/logging.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/configuration/save/resource.hh"
#include "com/centreon/engine/modules/webservice/configuration/save/objects.hh"
#include "com/centreon/engine/modules/webservice/configuration/save/state.hh"
#include "com/centreon/engine/modules/webservice/sync.hh"
#include "soapH.h"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Update content of USERn macros.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  resource_id Resource to modify data (index start by 1).
 *  @param[out] val         Result of operation.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__updateResourceUser(soap* s,
                                       ns1__resourceUserIDType* resource_id,
                                       std::string value,
                                       centreonengine__updateResourceUserResponse& res) {
  (void) res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
	<< "Webservice: " << __func__ << "(" << resource_id->id << ")";

    if (resource_id->id == 0 || resource_id->id >= MAX_USER_MACROS) {
      std::string* error = soap_new_std__string(s, 1);
      *error = "invalid resource id.";

      logger(dbg_commands, most)
        << "Webservice: " << __func__ << " failed. " << *error;

      webservice::sync::instance().worker_finish();
      return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
    }

    unsigned int pos(resource_id->id - 1);
    delete[] macro_user[pos];
    macro_user[pos] = string::dup(value.c_str());
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
 *  Save resources objects.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  filename    The file name to save objects.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveResources(soap* s,
                                  std::string filename,
                                  centreonengine__saveResourcesResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::resource save;
    save.add_resource(macro_user);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save the engine configuration.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  filename    The file name to save objects.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveConfiguration(soap* s,
                                      std::string filename,
                                      centreonengine__saveConfigurationResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::state save;
    save << *config;
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save all engine objects.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  filename    The file name to save objects.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveAllObjects(soap* s,
                                   std::string filename,
                                   centreonengine__saveAllObjectsResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(command_list);
    save.add_list(contact_list);
    save.add_list(contactgroup_list);
    save.add_list(host_list);
    save.add_list(hostescalation_list);
    save.add_list(hostdependency_list);
    save.add_list(hostgroup_list);
    save.add_list(service_list);
    save.add_list(servicedependency_list);
    save.add_list(serviceescalation_list);
    save.add_list(servicegroup_list);
    save.add_list(timeperiod_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save command objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveCommands(soap* s,
                                 std::string filename,
                                 centreonengine__saveCommandsResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(command_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save contact objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveContacts(soap* s,
                                 std::string filename,
                                 centreonengine__saveContactsResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(contact_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save contactgroup objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveContactgroups(soap* s,
                                      std::string filename,
                                      centreonengine__saveContactgroupsResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(contactgroup_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save host objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveHosts(soap* s,
                              std::string filename,
                              centreonengine__saveHostsResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(host_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save host escalations objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveHostEscalations(soap* s,
                                        std::string filename,
                                        centreonengine__saveHostEscalationsResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(hostescalation_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save host dependencies objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveHostDependencies(soap* s,
                                         std::string filename,
                                         centreonengine__saveHostDependenciesResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(hostdependency_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save host groups objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveHostGroups(soap* s,
                                   std::string filename,
                                   centreonengine__saveHostGroupsResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(hostgroup_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save service objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveServices(soap* s,
                                 std::string filename,
                                 centreonengine__saveServicesResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(service_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save service dependencies objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveServiceDependencies(soap* s,
                                            std::string filename,
                                            centreonengine__saveServiceDependenciesResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(servicedependency_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save service escalations objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveServiceEscalations(soap* s,
                                           std::string filename,
                                           centreonengine__saveServiceEscalationsResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(serviceescalation_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save service groups objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveServiceGroups(soap* s,
                                      std::string filename,
                                      centreonengine__saveServiceGroupsResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(servicegroup_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Save timeperiod objects.
 *
 *  @param[in]  s            Unused.
 *  @param[in]  filename     The file name to save objects.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__saveTimeperiods(soap* s,
                                    std::string filename,
                                    centreonengine__saveTimeperiodsResponse& res) {
  (void)res;
  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << filename << ")";

    configuration::save::objects save;
    save.add_list(timeperiod_list);
    save.backup(filename);

    webservice::sync::instance().worker_finish();
  }
  catch (std::exception const& e) {
    std::string* error = soap_new_std__string(s, 1);
    *error = e.what();

    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed. " << *error;

    webservice::sync::instance().worker_finish();
    return (soap_receiver_fault(s, "Invalid parameter.", error->c_str()));
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
 *  Set global variable command_check_interval.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of command_check_interval.
 *  @param[in]  is_second   True if interval is in second, otherwise false.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setCommandCheckInterval(soap* s,
                                            int value,
                                            bool is_second,
                                            centreonengine__setCommandCheckIntervalResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ", " << is_second << ")";

    try {
      config->command_check_interval(value, is_second);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable check_external_commands.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable check_external_commands.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setCheckExternalCommands(soap* s,
                                             bool value,
                                             centreonengine__setCheckExternalCommandsResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->check_external_commands(value);

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
 *  Set global variable use_aggressive_host_checking.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable use_aggressive_host_checking.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setUseAggressiveHostChecking(soap* s,
                                                 bool value,
                                                 centreonengine__setUseAggressiveHostCheckingResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->use_aggressive_host_checking(value);

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
 *  Set global variable global_host_event_handler.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  command     The new command id.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setGlobalHostEventHandler(soap* s,
                                              ns1__commandIDType *command,
                                              centreonengine__setGlobalHostEventHandlerResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << command->command << ")";

    config->global_host_event_handler(command->command.c_str());

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
 *  Set global variable global_servive_event_handler.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  command     The new command id.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setGlobalServiceEventHandler(soap* s,
                                                 ns1__commandIDType *command,
                                                 centreonengine__setGlobalServiceEventHandlerResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << command->command << ")";

    try {
      config->global_service_event_handler(command->command.c_str());
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable soft_state_dependencies.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable soft_state_dependencies.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setSoftStateDependencies(soap* s,
                                             bool value,
                                             centreonengine__setSoftStateDependenciesResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->soft_state_dependencies(value);

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
 *  Set global variable check_service_freshness.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable check_service_freshness.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServiceFreshnessChecksEnabled(
      soap* s,
      bool value,
      centreonengine__setServiceFreshnessChecksEnabledResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->check_service_freshness(value);

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
 *  Set global variable service_freshness_check_interval.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of service_freshness_check_interval.
 *                          Value must be different of zero.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServiceFreshnessCheckInterval(soap* s,
                                                     unsigned int value,
                                                     centreonengine__setServiceFreshnessCheckIntervalResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->service_freshness_check_interval(value);

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
 *  Set global variable check_host_freshness.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable check_host_freshness.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostFreshnessChecksEnabled(
      soap* s,
      bool value,
      centreonengine__setHostFreshnessChecksEnabledResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->check_host_freshness(value);

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
 *  Set global variable host_freshness_check_interval.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of host_freshness_check_interval.
 *                          Value must be different of zero.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostFreshnessCheckInterval(soap* s,
                                                  unsigned int value,
                                                  centreonengine__setHostFreshnessCheckIntervalResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->host_freshness_check_interval(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable additional_freshnesslatency.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of additional_freshnesslatency.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setAdditionalFreshnessLatency(soap* s,
                                                  int value,
                                                  centreonengine__setAdditionalFreshnessLatencyResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->additional_freshness_latency(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable low_service_flap_threshold.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of low_service_flap_threshold.
 *                          Value must be between 0.0 and 100.0.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setLowServiceFlapThreshold(soap* s,
                                               float value,
                                               centreonengine__setLowServiceFlapThresholdResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->low_service_flap_threshold(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable hight_service_flap_threshold.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of hight_service_flap_threshold.
 *                          Value must be between 0.0 and 100.0.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHighServiceFlapThreshold(soap* s,
                                                float value,
                                                centreonengine__setHighServiceFlapThresholdResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->high_service_flap_threshold(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable low_host_flap_threshold.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of low_host_flap_threshold.
 *                          Value must be between 0.0 and 100.0.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setLowHostFlapThreshold(soap* s,
                                            float value,
                                            centreonengine__setLowHostFlapThresholdResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->low_host_flap_threshold(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable high_host_flap_threshold.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of high_host_flap_threshold.
 *                          Value must be between 0.0 and 100.0.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHighHostFlapThreshold(soap* s,
                                             float value,
                                             centreonengine__setHighHostFlapThresholdResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->high_host_flap_threshold(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable log_notifications.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable log_notifications.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setLogNotifications(soap* s,
                                        bool value,
                                        centreonengine__setLogNotificationsResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->log_notifications(value);

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
 *  Set global variable log_service_retries.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable log_service_retries.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setLogServiceRetries(soap* s,
                                         bool value,
                                         centreonengine__setLogServiceRetriesResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->log_service_retries(value);

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
 *  Set global variable log_host_retries.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable log_host_retries.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setLogHostRetries(soap* s,
                                      bool value,
                                      centreonengine__setLogHostRetriesResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->log_host_retries(value);

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
 *  Set global variable log_event_handlers.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable log_event_handlers.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setLogEventHandlers(soap* s,
                                        bool value,
                                        centreonengine__setLogEventHandlersResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->log_event_handlers(value);

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
 *  Set global variable log_initial_state.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable log_initial_state.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setLogInitialState(soap* s,
                                       bool value,
                                       centreonengine__setLogInitialStateResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->log_initial_states(value);

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
 *  Set global variable log_external_commands.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable log_external_commands.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setLogExternalCommands(soap* s,
                                           bool value,
                                           centreonengine__setLogExternalCommandsResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->log_external_commands(value);

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
 *  Set global variable log_passive_chekcs.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable log_passive_chekcs.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setLogPassiveChecks(soap* s,
                                        bool value,
                                        centreonengine__setLogPassiveChecksResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->log_passive_checks(value);

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
 *  Set global variable service_check_timeout.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of service_check_timeout.
 *                          Value must be different of zero.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServiceCheckTimeout(soap* s,
                                           unsigned int value,
                                           centreonengine__setServiceCheckTimeoutResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->service_check_timeout(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable host_check_timeout.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of host_check_timeout.
 *                          Value must be different of zero.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setHostCheckTimeout(soap* s,
                                        unsigned int value,
                                        centreonengine__setHostCheckTimeoutResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->host_check_timeout(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable event_handler_timeout.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of event_handler_timeout.
 *                          Value must be different of zero.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setEventHandlerTimeout(soap* s,
                                           unsigned int value,
                                           centreonengine__setEventHandlerTimeoutResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->event_handler_timeout(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable notification_timeout.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of notification_timeout.
 *                          Value must be different of zero.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setNotificationTimeout(soap* s,
                                           unsigned int value,
                                           centreonengine__setNotificationTimeoutResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->notification_timeout(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable ocsp_timeout.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of ocsp_timeout.
 *                          Value must be different of zero.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setOcspTimeout(soap* s,
                                   unsigned int value,
                                   centreonengine__setOcspTimeoutResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->ocsp_timeout(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable ochp_timeout.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of ochp_timeout.
 *                          Value must be different of zero.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setOchpTimeout(soap* s,
                                   unsigned int value,
                                   centreonengine__setOchpTimeoutResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->ochp_timeout(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable retention_update_interval.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of retention_update_interval.
 *                          Value must be different of zero.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setRetentionUpdateInterval(soap* s,
                                               unsigned int value,
                                               centreonengine__setRetentionUpdateIntervalResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      unsigned int interval(config->retention_update_interval() * 60);
      config->retention_update_interval(value);
      value *= 60;

      std::list<timed_event*> lst_events;
      for (timed_event* event(event_list_high); event != NULL; event = event->next)
        if (event->event_type == EVENT_RETENTION_SAVE)
          lst_events.push_back(event);
      for (std::list<timed_event*>::const_iterator
             it(lst_events.begin()), end(lst_events.end());
           it != end;
           ++it) {
        timed_event* event(*it);
        remove_event(event, &event_list_high, &event_list_high_tail);
        event->run_time = (event->run_time - interval) + value;
        event->event_interval = value;
        add_event(event, &event_list_high, &event_list_high_tail);
      }
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable sleep_time.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of sleep_time.
 *                          Value must be greater than 0.0.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setSleepTime(soap* s,
                                 float value,
                                 centreonengine__setSleepTimeResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->sleep_time(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable service_interleave_factor.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  is_smart    Enable or disable smart detection.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setServiceInterleaveFactor(soap* s,
                                               bool is_smart,
                                               centreonengine__setServiceInterleaveFactorResponse& res) {
  using namespace com::centreon::engine::configuration;

  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << is_smart << ")";

    config->service_interleave_factor_method(is_smart ? state::ilf_smart : state::ilf_user);

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
 *  Set global variable max_concurrent_checks.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of max_concurrent_checks.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setMaxConcurrentChecks(soap* s,
                                           unsigned int value,
                                           centreonengine__setMaxConcurrentChecksResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->max_parallel_service_checks(value);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable check_result_reaper_frequency.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of check_result_reaper_frequency.
 *                          Value must be different of zero.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setCheckResultReaperFrequency(soap* s,
                                                  unsigned int value,
                                                  centreonengine__setCheckResultReaperFrequencyResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      unsigned int interval(config->check_reaper_interval());
      config->check_reaper_interval(value);

      std::list<timed_event*> lst_events;
      for (timed_event* event(event_list_high); event != NULL; event = event->next)
        if (event->event_type == EVENT_CHECK_REAPER)
          lst_events.push_back(event);
      for (std::list<timed_event*>::const_iterator
             it(lst_events.begin()), end(lst_events.end());
           it != end;
           ++it) {
        timed_event* event(*it);
        remove_event(event, &event_list_high, &event_list_high_tail);
        event->run_time = (event->run_time - interval) + value;
        event->event_interval = value;
        add_event(event, &event_list_high, &event_list_high_tail);
      }
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable use_lage_installation_tweaks.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       Enable or disable use_lage_installation_tweaks.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setUseLargeInstallationTweaks(soap* s,
                                                  bool value,
                                                  centreonengine__setUseLargeInstallationTweaksResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->use_large_installation_tweaks(value);

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
 *  Set global variable debug_verbosity.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of debug_verbosity.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setDebugVerbosity(soap* s,
                                      unsigned int value,
                                      centreonengine__setDebugVerbosityResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->debug_verbosity(value);
      com::centreon::engine::configuration::applier::logging::instance().apply(*config);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
 *  Set global variable debug_level.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of debug_level.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setDebugLevel(soap* s,
                                  unsigned long value,
                                  centreonengine__setDebugLevelResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    config->debug_level(value);
    com::centreon::engine::configuration::applier::logging::instance().apply(*config);

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
 *  Set global variable debug_file_size.
 *
 *  @param[in]  s           Unused.
 *  @param[in]  value       The new value of debug_file_size.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__setMaxDebugFileSize(soap* s,
                                        ULONG64 value,
                                        centreonengine__setMaxDebugFileSizeResponse& res) {
  (void)res;

  try {
    webservice::sync::instance().wait_thread_safeness();

    logger(dbg_functions, most)
      << "Webservice: " << __func__ << "(" << value << ")";

    try {
      config->max_debug_file_size(value);
      com::centreon::engine::configuration::applier::logging::instance().apply(*config);
    }
    catch (std::exception const& e) {
      std::string* error = soap_new_std__string(s, 1);
      *error = e.what();

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
