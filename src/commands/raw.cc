/*
** Copyright 2011-2015,2017 Centreon
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

#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/commands/environment.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::commands;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Constructor.
 *
 *  @param[in] name         The command name.
 *  @param[in] command_line The command line.
 *  @param[in] listener     The listener who catch events.
 */
raw::raw(
       std::string const& name,
       std::string const& command_line,
       command_listener* listener)
  : command(name, command_line, listener), process_listener() {}

/**
 *  Copy constructor
 *
 *  @param[in] right Object to copy.
 */
raw::raw(raw const& right) : command(right), process_listener(right) {}

/**
 *  Destructor.
 */
raw::~raw() throw () {
  try {
    concurrency::locker lock(&_lock);
    while (!_processes_busy.empty()) {
      process* p(_processes_busy.begin()->first);
      lock.unlock();
      p->wait();
      lock.relock();
    }
    for (std::list<process*>::const_iterator
           it(_processes_free.begin()), end(_processes_free.end());
         it != end;
         ++it)
      delete *it;
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: Raw command destructor failed: " << e.what();
  }
}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
raw& raw::operator=(raw const& right) {
  if (this != &right)
    command::operator=(right);
  return (*this);
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
commands::command* raw::clone() const {
  return (new raw(*this));
}

/**
 *  Run a command.
 *
 *  @param[in] args    The command arguments.
 *  @param[in] macros  The macros data struct.
 *  @param[in] timeout The command timeout.
 *
 *  @return The command id.
 */
unsigned long raw::run(
                     std::string const& processed_cmd,
                     nagios_macros& macros,
                     unsigned int timeout) {
  logger(dbg_commands, basic)
    << "raw::run: cmd='" << processed_cmd << "', timeout=" << timeout;

  // Get process and put into the busy list.
  process* p(NULL);
  unsigned long command_id(get_uniq_id());
  {
    concurrency::locker lock(&_lock);
    p = _get_free_process();
    _processes_busy[p] = command_id;
  }

  logger(dbg_commands, basic)
    << "raw::run: id=" << command_id << ", process=" << p;

  // Setup environement macros if is necessary.
  environment env;
  _build_environment_macros(macros, env);

  try {
    // Start process.
    p->exec(processed_cmd.c_str(), env.data(), timeout);
    logger(dbg_commands, basic)
      << "raw::run: start process success: id=" << command_id;
  }
  catch (...) {
    logger(dbg_commands, basic)
      << "raw::run: start process failed: id=" << command_id;

    concurrency::locker lock(&_lock);
    _processes_busy.erase(p);
    throw;
  }
  return (command_id);
}

/**
 *  Run a command and wait the result.
 *
 *  @param[in]  args    The command arguments.
 *  @param[in]  macros  The macros data struct.
 *  @param[in]  timeout The command timeout.
 *  @param[out] res     The result of the command.
 */
void raw::run(
            std::string const& processed_cmd,
            nagios_macros& macros,
            unsigned int timeout,
            result& res) {
  logger(dbg_commands, basic)
    << "raw::run: cmd='" << processed_cmd << "', timeout=" << timeout;

  // Get process.
  process p;
  unsigned long command_id(get_uniq_id());

  logger(dbg_commands, basic)
    << "raw::run: id=" << command_id << ", process=" << &p;

  // Setup environement macros if is necessary.
  environment env;
  _build_environment_macros(macros, env);

  // Start process.
  try {
    p.exec(processed_cmd.c_str(), env.data(), timeout);
    logger(dbg_commands, basic)
      << "raw::run: start process success: id=" << command_id;
  }
  catch (...) {
    logger(dbg_commands, basic)
      << "raw::run: start process failed: id=" << command_id;
    throw;
  }

  // Wait for completion.
  p.wait();

  // Get process output.
  p.read(res.output);

  // Set result informations.
  res.command_id = command_id;
  res.start_time = p.start_time();
  res.end_time = p.end_time();
  res.exit_code = p.exit_code();
  res.exit_status = p.exit_status();

  if (res.exit_status == process::timeout) {
    res.exit_code = STATE_UNKNOWN;
    res.output = "(Process Timeout)";
  }
  else if ((res.exit_status == process::crash)
           || (res.exit_code < -1)
           || (res.exit_code > 3))
    res.exit_code = STATE_UNKNOWN;

  logger(dbg_commands, basic)
    << "raw::run: end process: "
    "id=" << command_id << ", "
    "start_time=" << res.start_time.to_mseconds() << ", "
    "end_time=" << res.end_time.to_mseconds() << ", "
    "exit_code=" << res.exit_code << ", "
    "exit_status=" << res.exit_status << ", "
    "output='" << res.output << "'";
  return;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Provide by process_listener interface but not used.
 *
 *  @param[in] p  Unused.
 */
void raw::data_is_available(process& p) throw () {
  (void)p;
  return;
}

/**
 *  Provide by process_listener interface but not used.
 *
 *  @param[in] p  Unused.
 */
void raw::data_is_available_err(process& p) throw () {
  (void)p;
  return;
}

/**
 *  Provide by process_listener interface. Call at the end
 *  of the process execution.
 *
 *  @param[in] p  The process to finished.
 */
void raw::finished(process& p) throw () {
  try {
    logger(dbg_commands, basic)
      << "raw::finished: process=" << &p;

    unsigned long command_id(0);
    {
      concurrency::locker lock(&_lock);
      // Find process from the busy list.
      umap<process*, unsigned long>::iterator
        it(_processes_busy.find(&p));
      if (it == _processes_busy.end()) {
        // Put the process into the free list.
        _processes_free.push_back(&p);
        lock.unlock();

        logger(log_runtime_warning, basic)
          << "Warning: Invalid process pointer: "
             "process not found into process busy list";
        return ;
      }
      // Get command_id and remove the process from the busy list.
      command_id = it->second;
      _processes_busy.erase(it);
    }

    logger(dbg_commands, basic)
      << "raw::finished: id=" << command_id;

    // Build check result.
    result res;

    // Get process output.
    p.read(res.output);

    // Release process, put into the free list.
    {
      concurrency::locker lock(&_lock);
      _processes_free.push_back(&p);
    }

    // Set result informations.
    res.command_id = command_id;
    res.start_time = p.start_time();
    res.end_time = p.end_time();
    res.exit_code = p.exit_code();
    res.exit_status = p.exit_status();

    if (res.exit_status == process::timeout) {
      res.exit_code = STATE_UNKNOWN;
      res.output = "(Process Timeout)";
    }
    else if ((res.exit_status == process::crash)
             || (res.exit_code < -1)
             || (res.exit_code > 3))
      res.exit_code = STATE_UNKNOWN;

    logger(dbg_commands, basic)
      << "raw::finished: "
      "id=" << command_id << ", "
      "start_time=" << res.start_time.to_mseconds() << ", "
      "end_time=" << res.end_time.to_mseconds() << ", "
      "exit_code=" << res.exit_code << ", "
      "exit_status=" << res.exit_status << ", "
      "output='" << res.output << "'";

    // Forward result to the listener.
    if (_listener)
      (_listener->finished)(res);
  }
  catch (std::exception const& e) {
    logger(log_runtime_warning, basic)
      << "Warning: Raw process termination routine failed: "
      << e.what();

    // Release process, put into the free list.
    concurrency::locker lock(&_lock);
    _processes_free.push_back(&p);
  }
  return;
}

/**
 *  Build argv macro environment variables.
 *
 *  @param[in]  macros  The macros data struct.
 *  @param[out] env     The environment to fill.
 */
void raw::_build_argv_macro_environment(
            nagios_macros const& macros,
            environment& env) {
  for (unsigned int i(0); i < MAX_COMMAND_ARGUMENTS; ++i) {
    char const* value(macros.argv[i] ? macros.argv[i] : "");
    std::ostringstream oss;
    oss << MACRO_ENV_VAR_PREFIX "ARG" << (i + 1) << "=" << value;
    env.add(oss.str());
  }
  return;
}

/**
 *  Build contact address environment variables.
 *
 *  @param[in]  macros  The macros data struct.
 *  @param[out] env     The environment to fill.
 */
void raw::_build_contact_address_environment(
            nagios_macros const& macros,
            environment& env) {
  if (!macros.contact_ptr)
    return;
  for (unsigned int i(0); i < MAX_CONTACT_ADDRESSES; ++i) {
    char const* value(macros.contact_ptr->address[i]);
    if (!value)
      value = "";
    std::ostringstream oss;
    oss << MACRO_ENV_VAR_PREFIX "CONTACTADDRESS" << i << "=" << value;
    env.add(oss.str());
  }
  return;
}

/**
 *  Build custom contact macro environment variables.
 *
 *  @param[in,out] macros  The macros data struct.
 *  @param[out]    env     The environment to fill.
 */
void raw::_build_custom_contact_macro_environment(
            nagios_macros& macros,
            environment& env) {
  // Build custom contact variable.
  contact* hst(macros.contact_ptr);
  if (hst) {
    for (customvariablesmember* customvar(hst->custom_variables);
         customvar;
         customvar = customvar->next)
      if (customvar->variable_name) {
        char const* value(customvar->variable_value);
        if (!value)
          value = "";
        std::string name("_CONTACT");
        name.append(customvar->variable_name);
        add_custom_variable_to_object(
          &macros.custom_contact_vars,
          name.c_str(),
          value,
          customvar->is_sent);
      }
  }
  // Set custom contact variable into the environement
  for (customvariablesmember* customvar(macros.custom_contact_vars);
       customvar;
       customvar = customvar->next)
    if (customvar->variable_name) {
      char const* value("");
      if (customvar->variable_value)
        value = clean_macro_chars(
                  customvar->variable_value,
                  STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);
      std::string line;
      line.append(MACRO_ENV_VAR_PREFIX);
      line.append(customvar->variable_name);
      line.append("=");
      line.append(value);
      env.add(line);
    }
  return;
}

/**
 *  Build custom host macro environment variables.
 *
 *  @param[in,out] macros  The macros data struct.
 *  @param[out]    env     The environment to fill.
 */
void raw::_build_custom_host_macro_environment(
            nagios_macros& macros,
            environment& env) {
  // Build custom host variable.
  host* hst(macros.host_ptr);
  if (hst) {
    for (customvariablesmember* customvar(hst->custom_variables);
         customvar;
         customvar = customvar->next)
      if (customvar->variable_name) {
        char const* value("");
        if (customvar->variable_value)
          value = customvar->variable_value;
        std::string name("_HOST");
        name.append(customvar->variable_name);
        add_custom_variable_to_object(
          &macros.custom_host_vars,
          name.c_str(),
          value,
          customvar->is_sent);
      }
  }
  // Set custom host variable into the environement
  for (customvariablesmember* customvar(macros.custom_host_vars);
       customvar;
       customvar = customvar->next)
    if (customvar->variable_name) {
      char const* value("");
      if (customvar->variable_value)
        value = clean_macro_chars(
                  customvar->variable_value,
                  STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);
      std::string line;
      line.append(MACRO_ENV_VAR_PREFIX);
      line.append(customvar->variable_name);
      line.append("=");
      line.append(value);
      env.add(line);
    }
  return;
}

/**
 *  Build custom service macro environment variables.
 *
 *  @param[in,out] macros  The macros data struct.
 *  @param[out]    env     The environment to fill.
 */
void raw::_build_custom_service_macro_environment(
            nagios_macros& macros,
            environment& env) {
  // Build custom service variable.
  service* hst(macros.service_ptr);
  if (hst) {
    for (customvariablesmember* customvar(hst->custom_variables);
         customvar;
         customvar = customvar->next)
      if (customvar->variable_name) {
        char const* value(customvar->variable_value);
        if (!value)
          value = "";
        std::string name("_SERVICE");
        name.append(customvar->variable_name);
        add_custom_variable_to_object(
          &macros.custom_service_vars,
          name.c_str(),
          value,
          customvar->is_sent);
      }
  }
  // Set custom service variable into the environement
  for (customvariablesmember* customvar(macros.custom_service_vars);
       customvar;
       customvar = customvar->next)
    if (customvar->variable_name) {
      char const* value("");
      if (customvar->variable_value)
        value = clean_macro_chars(
                  customvar->variable_value,
                  STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS);
      std::string line;
      line.append(MACRO_ENV_VAR_PREFIX);
      line.append(customvar->variable_name);
      line.append("=");
      line.append(value);
      env.add(line);
    }
  return;
}

/**
 *  Build all macro environemnt variable.
 *
 *  @param[in,out] macros  The macros data struct.
 *  @param[out]    env     The environment to fill.
 */
void raw::_build_environment_macros(
            nagios_macros& macros,
            environment& env) {
  if (config->enable_environment_macros()) {
    _build_macrosx_environment(macros, env);
    _build_argv_macro_environment(macros, env);
    _build_custom_host_macro_environment(macros, env);
    _build_custom_service_macro_environment(macros, env);
    _build_custom_contact_macro_environment(macros, env);
    _build_contact_address_environment(macros, env);
  }
  return;
}

/**
 *  Build macrox environment variables.
 *
 *  @param[in,out] macros  The macros data struct.
 *  @param[out]    env     The environment to fill.
 */
void raw::_build_macrosx_environment(
            nagios_macros& macros,
            environment& env) {
  for (unsigned int i(0); i < MACRO_X_COUNT; ++i) {
    int release_memory(0);

    // Need to grab macros?
    if (!macros.x[i]) {
      // Skip summary macro in lage instalation tweaks.
      if ((i < MACRO_TOTALHOSTSUP)
          || (i > MACRO_TOTALSERVICEPROBLEMSUNHANDLED)
          || !config->use_large_installation_tweaks()) {
        grab_macrox_value_r(
          &macros,
          i,
          NULL,
          NULL,
          &macros.x[i],
          &release_memory);
      }
    }

    // Add into the environment.
    if (macro_x_names[i]) {
      std::string line;
      line.append(MACRO_ENV_VAR_PREFIX);
      line.append(macro_x_names[i]);
      line.append("=");
      line.append(macros.x[i] ? macros.x[i] : "");
      env.add(line);
    }

    // Release memory if necessary.
    if (release_memory) {
      delete[] macros.x[i];
      macros.x[i] = NULL;
    }
  }
  return;
}

/**
 *  Get one process to execute command.
 *
 *  @return A process.
 */
process* raw::_get_free_process() {
  // If any process are available, create new one.
  if (_processes_free.empty()) {
    process* p(new process(this));
    p->enable_stream(process::in, false);
    p->enable_stream(process::err, false);
    p->setpgid_on_exec(config->use_setpgid());
    return (p);
  }
  // Get a free process.
  process* p(_processes_free.front());
  _processes_free.pop_front();
  return (p);
}
