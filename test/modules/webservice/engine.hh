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

#ifndef TEST_MOD_WS_ENGINE_HH
#  define TEST_MOD_WS_ENGINE_HH

#  include <string>
#  include "com/centreon/process.hh"

/**
 *  @class engine engine.hh "test/modules/webservice/engine.hh"
 *  @brief Engine process.
 *
 *  Manage launch and shutdown of Centreon Engine for testing purposes.
 */
class      engine {
public:
           engine();
           engine(engine const& e);
           ~engine() throw ();
  engine&  operator=(engine const& e);
  void     start(std::string const& cfg_file = "");
  void     stop();

private:
  com::centreon::process
           _centengine;
};

#endif // !TEST_MOD_WS_ENGINE_HH
