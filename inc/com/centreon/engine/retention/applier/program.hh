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

#ifndef CCE_RETENTION_APPLIER_PROGRAM_HH
#define CCE_RETENTION_APPLIER_PROGRAM_HH

#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/retention/program.hh"

CCE_BEGIN()

// Forward declaration.
namespace configuration {
class state;
}

namespace retention {
namespace applier {
class program {
 public:
  void apply(configuration::state& config, retention::program const& obj);

 private:
  bool _find_command(std::string const& command_line);
};
}  // namespace applier
}  // namespace retention

CCE_END()

#endif  // !CCE_RETENTION_APPLIER_PROGRAM_HH
