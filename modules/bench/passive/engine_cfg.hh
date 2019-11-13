/*
** Copyright 2015 Merethis
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

#ifndef ENGINE_CFG_HH
#define ENGINE_CFG_HH

#include <list>
#include <string>

/**
 *  @class engine_cfg engine_cfg.hh
 *  @brief Generate Centreon Engine configuration files.
 *
 *  Generate Centreon Engine configuration files suitable for the
 *  passive check results benchmark tool.
 */
class engine_cfg {
 public:
  engine_cfg(std::string const& additional,
             int expected_passive,
             int active_hosts,
             int active_services,
             int passive_hosts,
             int passive_services,
             bool auto_delete = true);
  ~engine_cfg();
  std::string command_file() const;
  std::string directory() const;
  std::string main_file() const;

 private:
  engine_cfg(engine_cfg const& other);
  engine_cfg& operator=(engine_cfg const& other);
  void _write_file(std::string const& target, std::string const& content);

  bool _auto_delete;
  std::string _directory;
  std::list<std::string> _generated;
};

#endif  // !ENGINE_CFG_HH
