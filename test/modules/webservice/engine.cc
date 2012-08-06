/*
** Copyright 2012 Merethis
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

#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif // Win32 or POSIX.
#include "com/centreon/engine/error.hh"
#include "test/modules/webservice/engine.hh"
#include "test/paths.hh"

using namespace com::centreon::engine;

/**************************************
*                                     *
*            Public Methods           *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
engine::engine() {}

/**
 *  Copy constructor.
 *
 *  @param[in] e Unused.
 */
engine::engine(engine const& e) {
  (void)e;
}

/**
 *  Destructor.
 */
engine::~engine() throw () {}

/**
 *  Launch Engine.
 *
 *  @param[in] cfg_file Engine configuration file.
 */
void engine::start(std::string const& cfg_file) {
  std::string cmd(CENTENGINE_BINARY " ");
  if (cfg_file.empty())
    cmd.append(TEST_DIR "/running/etc/webservice.cfg");
  else
    cmd.append(cfg_file);
  _centengine.exec(cmd);
#ifdef _WIN32
  Sleep(1000);
#else
  sleep(1);
#endif // Win32 or POSIX.
  return ;
}

/**
 *  Stop Engine.
 */
void engine::stop() {
  _centengine.terminate();
  if (!_centengine.wait(10000)) {
    _centengine.kill();
    _centengine.wait();
  }
  return ;
}
