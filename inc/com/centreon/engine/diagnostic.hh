/*
** Copyright 2013 Merethis
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

#ifndef CCE_DIAGNOSTIC_HH
#define CCE_DIAGNOSTIC_HH

#include <string>
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

/**
 *  @class diagnostic diagnostic.hh "com/centreon/engine/diagnostic.hh"
 *  @brief Diagnostic class.
 *
 *  Generate a diagnostic file that is useful for opening tickets
 *  against Merethis support center.
 */
class diagnostic {
 public:
  diagnostic();
  diagnostic(diagnostic const& right);
  ~diagnostic();
  diagnostic& operator=(diagnostic const& right);
  void generate(std::string const& cfg_file, std::string const& out_file = "");

 private:
  std::string _build_target_path(std::string const& base,
                                 std::string const& file);
  void _exec_and_write_to_file(std::string const& cmd,
                               std::string const& out_file);
  void _exec_cp(std::string const& src, std::string const& dst);
};

CCE_END()

#endif  // !CCE_DIAGNOSTIC_HH
