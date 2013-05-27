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

#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine;

static char const* whitespaces(" \t\r\n");

/**
 *  Split data into element.
 *
 *  @param[int] data  The data to split.
 *  @param[out] out   The list to fill.
 *  @param[in]  delim The delimiter.
 *
 *  @return The list of string split by delimiter.
 */
std::list<std::string>& misc::split(
                          std::string const& data,
                          std::list<std::string>& out,
                          char delim) {
  if (data.empty())
    return (out);

  std::size_t last(0);
  std::size_t current(0);
  while ((current = data.find(delim, current)) != std::string::npos) {
    out.push_back(data.substr(last, current - last));
    last = ++current;
  }
  out.push_back(last ? data.substr(last) : data);
  return (out);
}

/**
 *  Trim a string.
 *
 *  @param[in] str The string.
 *
 *  @return The trimming stream.
 */
std::string& misc::trim(std::string& str) throw () {
  size_t pos(str.find_last_not_of(whitespaces));
  if (pos == std::string::npos)
    str.clear();
  else {
    str.erase(pos + 1);
    if ((pos = str.find_first_not_of(whitespaces)) != std::string::npos)
      str.erase(0, pos);
  }
  return (str);
}

/**
 *  Trim at the left of the string.
 *
 *  @param[in] str The string.
 *
 *  @return The trimming stream.
 */
std::string& misc::trim_left(std::string& str) throw () {
  size_t pos(str.find_first_not_of(whitespaces));
  if (pos != std::string::npos)
    str.erase(0, pos);
  return (str);
}

/**
 *  Trim at the right of the string.
 *
 *  @param[in] str The string.
 *
 *  @return The trimming stream.
 */
std::string& misc::trim_right(std::string& str) throw () {
  size_t pos(str.find_last_not_of(whitespaces));
  if (pos == std::string::npos)
    str.clear();
  else
    str.erase(pos + 1);
  return (str);
}
