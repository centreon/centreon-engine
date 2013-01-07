/*
** Copyright 2012-2013 Merethis
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

#ifndef TEST_MOD_WS_QUERY_HH
#  define TEST_MOD_WS_QUERY_HH

#  include <string>

/**
 *  @class query query.hh "test/modules/webservice/query.hh"
 *  @brief Execute a query on Engine webservice.
 *
 *  Execute a webservice query on Engine by using the webservice client.
 */
class    query {
public:
         query();
         query(query const& q);
         ~query() throw ();
  query& operator=(query const& q);
  int    execute(
           std::string& output,
           std::string const& q);
};

#endif // !TEST_MOD_WS_QUERY_HH
