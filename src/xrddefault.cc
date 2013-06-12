/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2009      Nagios Core Development Team and Community Contributors
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

#include "com/centreon/engine/xrddefault.hh"

using namespace com::centreon::engine;

int xrddefault_save_state_information() {
  logger(logging::dbg_functions, logging::basic)
    << "xrddefault_save_state_information()";

  std::ofstream stream(
                  xrddefault_retention_file,
                  std::ios::out | std::ios::trunc);
  if (!stream.is_open()) {
    logger(logging::log_runtime_error, logging::basic)
      << "retention: can't open retention file: open "
      << xrddefault_retention_file << " failed";
    return (ERROR);
  }

  dump::header(stream);
  dump::info(stream);
  dump::program(stream);
  dump::hosts(stream);
  dump::services(stream);
  dump::contacts(stream);
  dump::comments(stream);
  dump::downtimes(stream);
  return (OK);
}

int xrddefault_read_state_information() {
  return (ERROR);
}
