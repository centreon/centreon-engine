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

#ifndef CCE_NOTIFIER_HH
# define CCE_NOTIFIER_HH

# include <array>
# include <string>
# include "com/centreon/engine/namespace.hh"

CCE_BEGIN()
class                         notifier {
 public:
  static std::array<std::string, 2> const tab_state_type;

                              notifier(std::string const& display_name,
                                       std::string const& check_command,
                                       int initial_state,
                                       double check_interval,
                                       double retry_interval);
  virtual                     ~notifier() {};

  std::string const&          get_display_name() const;
  void                        set_display_name(std::string const& name);
  std::string const&          get_check_command() const;
  void                        set_check_command(
                                std::string const& check_command);
  int                         get_initial_state() const;
  void                        set_initial_state(int initial_state);
  double                      get_check_interval() const;
  void                        set_check_interval(double check_interval);
  double                      get_retry_interval() const;
  void                        set_retry_interval(double retry_interval);

 protected:
  std::string                 _display_name;
  std::string                 _check_command;
  int                         _initial_state;
  double                      _check_interval;
  double                      _retry_interval;

};

CCE_END()

#endif // !CCE_NOTIFIER_HH
