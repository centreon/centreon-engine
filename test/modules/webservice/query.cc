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

#include <cstdlib>
#include "com/centreon/engine/error.hh"
#include "com/centreon/process.hh"
#include "test/modules/webservice/query.hh"
#include "test/paths.hh"

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
query::query() {}

/**
 *  Copy constructor.
 *
 *  @param[in] q Unused.
 */
query::query(query const& q) {
  (void)q;
}

/**
 *  Destructor.
 */
query::~query() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] q Unused.
 *
 *  @return This object.
 */
query& query::operator=(query const& q) {
  (void)q;
  return (*this);
}

/**
 *  Execute a webservice query.
 *
 *  @param[out] output Resulting output.
 *  @param[in]  q      Query.
 *
 *  @return Plugin output.
 */
int query::execute(std::string& output, std::string const& q) {
  // Command line.
  std::string cmdline(CENTENGINEWS_BINARY);
  cmdline.append(" ");
  cmdline.append(q);

  // Execute webservice client.
  com::centreon::process client;
  //client.setProcessChannelMode(QProcess::MergedChannels);
  client.exec(cmdline);

  // Exit code.
  int retval(EXIT_FAILURE);

  try {
    // Wait for termination.
    client.wait();

    // Get output.
    client.read(output);
    std::string buffer;
    client.read_err(buffer);
    output.append(buffer);

    // Return value.
    if (client.exit_status() != com::centreon::process::normal)
      throw (engine_error() << "centenginews crashed");
    retval = client.exit_code();
  }
  catch (...) {
    client.terminate();
    if (!client.wait(10000)) {
      client.kill();
      client.wait();
    }
    throw ;
  }

  return (retval);
}
