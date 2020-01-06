/*
** Copyright 2011-2019 Centreon
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

#ifndef CCE_CONFIGURATION_APPLIER_STATE_HH
#define CCE_CONFIGURATION_APPLIER_STATE_HH

#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/servicedependency.hh"
#include "com/centreon/engine/timeperiod.hh"

CCE_BEGIN()

// Forward declaration.
namespace commands {
class command;
class connector;
class service;
}  // namespace commands

namespace retention {
class state;
}

namespace configuration {
namespace applier {
/**
 *  @class state state.hh
 *  @brief Simple configuration applier for state class.
 *
 *  Simple configuration applier for state class.
 */
class state {
 public:
  void apply(configuration::state& new_cfg);
  void apply(configuration::state& new_cfg, retention::state& state);
  static state& instance();
  void clear();

  servicedependency_mmap const& servicedependencies() const throw();
  servicedependency_mmap& servicedependencies() throw();
  servicedependency_mmap::iterator servicedependencies_find(
      configuration::servicedependency::key_type const& k);
  std::unordered_map<std::string, std::string>& user_macros();
  std::unordered_map<std::string, std::string>::const_iterator user_macros_find(
      std::string const& key) const;
  void lock();
  void unlock();

 private:
  enum processing_state {
    state_waiting,
    state_apply,
    state_error,
    state_ready
  };

  state();
  state(state const&);
  ~state() throw();

#ifdef DEBUG_CONFIG
  void _check_serviceescalations() const;
  void _check_hostescalations() const;
  void _check_contacts() const;
  void _check_contactgroups() const;
  void _check_services() const;
  void _check_hosts() const;
#endif

  state& operator=(state const&);
  void _apply(configuration::state const& new_cfg);
  template <typename ConfigurationType, typename ApplierType>
  void _apply(difference<std::set<ConfigurationType>> const& diff);
  void _apply(configuration::state& new_cfg, retention::state& state);
  template <typename ConfigurationType, typename ApplierType>
  void _expand(configuration::state& new_state);
  void _processing(configuration::state& new_cfg,
                   retention::state* state = NULL);
  template <typename ConfigurationType, typename ApplierType>
  void _resolve(std::set<ConfigurationType>& cfg);

  std::mutex _apply_lock;
  state* _config;
  processing_state _processing_state;

  servicedependency_mmap _servicedependencies;
  std::unordered_map<std::string, std::string> _user_macros;
};
}  // namespace applier
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_APPLIER_STATE_HH
