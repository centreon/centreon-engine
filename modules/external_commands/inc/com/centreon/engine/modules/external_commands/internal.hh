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

#ifndef CCE_MODULES_EXTERNAL_COMMANDS_INTERNAL_HH
#define CCE_MODULES_EXTERNAL_COMMANDS_INTERNAL_HH

#include "com/centreon/engine/modules/external_commands/processing.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace modules {
namespace external_commands {
extern processing gl_processor;
}
}  // namespace modules

CCE_END()

#endif  // !CCE_MODULES_EXTERNAL_COMMANDS_INTERNAL_HH
