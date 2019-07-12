/*
** Copyright 2011-2014,2017 Centreon
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

#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;

static char const* whitespaces(" \t\r\n");

/**
 *  Get the next valid line.
 *
 *  @param[in, out] stream The current stream to read new line.
 *  @param[out]     line   The line to fill.
 *  @param[in, out] pos    The current position.
 *
 *  @return True if data is available, false if no data.
 */
bool string::get_next_line(
       std::ifstream& stream,
       std::string& line,
       unsigned int& pos) {
  while (std::getline(stream, line, '\n')) {
    ++pos;
    string::trim(line);
    if (!line.empty()) {
      char c(line[0]);
      if (c != '#' && c != ';' && c != '\x0')
        return (true);
    }
  }
  return (false);
}

/**
 *  Get key and value from line.
 *
 *  @param[in,out] line  The line to process.
 *  @param[out]    key   The key pointer.
 *  @param[out]    value The value pointer.
 *  @param[in]     delim The delimiter.
 */
bool string::split(
               std::string& line,
               char const** key,
               char const** value,
               char delim) {
  std::size_t delim_pos(line.find_first_of(delim));
  if (delim_pos == std::string::npos)
    return (false);

  std::size_t first_pos;
  std::size_t last_pos;
  line.append("", 1);

  last_pos = line.find_last_not_of(whitespaces, delim_pos - 1);
  if (last_pos == std::string::npos)
    *key = NULL;
  else {
    first_pos = line.find_first_not_of(whitespaces);
    line[last_pos + 1] = '\0';
    *key = line.data() + first_pos;
  }

  first_pos = line.find_first_not_of(whitespaces, delim_pos + 1);
  if (first_pos == std::string::npos)
    *value = NULL;
  else {
    last_pos = line.find_last_not_of(whitespaces);
    line[last_pos + 1] = '\0';
    *value = line.data() + first_pos;
  }

  return (true);
}

/**
 *  Get key and value from line.
 *
 *  @param[in]  line  The line to extract data.
 *  @param[out] key   The key to fill.
 *  @param[out] value The value to fill.
 *  @param[in]  delim The delimiter.
 */
bool string::split(
       std::string const& line,
       std::string& key,
       std::string& value,
       char delim) {
  std::size_t delim_pos(line.find_first_of(delim));
  if (delim_pos == std::string::npos)
    return (false);

  std::size_t first_pos;
  std::size_t last_pos;

  last_pos = line.find_last_not_of(whitespaces, delim_pos - 1);
  if (last_pos == std::string::npos)
    key.clear();
  else {
    first_pos = line.find_first_not_of(whitespaces);
    key.assign(line, first_pos, last_pos + 1 - first_pos);
  }

  first_pos = line.find_first_not_of(whitespaces, delim_pos + 1);
  if (first_pos == std::string::npos)
    value.clear();
  else {
    last_pos = line.find_last_not_of(whitespaces);
    value.assign(line, first_pos, last_pos + 1 - first_pos);
  }

  return (true);
}

/**
 *  Split data into element.
 *
 *  @param[in]  data  The data to split.
 *  @param[out] out   The list to fill.
 *  @param[in]  delim The delimiter.
 */
void string::split(
               std::string const& data,
               std::list<std::string>& out,
               char delim) {
  if (data.empty())
    return ;

  std::size_t last(0);
  std::size_t current(0);
  while ((current = data.find(delim, current)) != std::string::npos) {
    std::string tmp(data.substr(last, current - last));
    out.push_back(trim(tmp));
    last = ++current;
  }
  std::string tmp(last ? data.substr(last) : data);
  out.push_back(trim(tmp));
}

/**
 *  Split data into sorted elements.
 *
 *  @param[in]  data   The data to split.
 *  @param[out] out    The set to fill.
 *  @param[in]  delim  The delimiter.
 */
void string::split(
               std::string const& data,
               std::set<std::string>& out,
               char delim) {
  std::list<std::string> elements;
  split(data, elements, delim);
  out.insert(elements.begin(), elements.end());
}

/**
 *  Split data into pair of sorted elements.
 *
 *  @param[in]  data   The data to split.
 *  @param[out] out    The set to fill.
 *  @param[in]  delim  The delimiter.
 */
void string::split(
               std::string const& data,
               std::set<std::pair<std::string, std::string> >& out,
               char delim) {
  std::list<std::string> elements;
  split(data, elements, delim);
  for (std::list<std::string>::const_iterator
         it(elements.begin()),
         end(elements.end());
       it != end;
       ++it) {
    std::list<std::string>::const_iterator first(it++);
    if (it == end)
      throw (engine_error()
             << "Not enough elements in the line to make pairs");
    out.insert(std::make_pair(*first, *it));
  }
}

/**
 *  Trim a string.
 *
 *  @param[in] str The string.
 *
 *  @return The trimming stream.
 */
std::string& string::trim(std::string& str) throw () {
  // First, search backward for the last non-space character.
  size_t pos(str.find_last_not_of(whitespaces));
  if (pos == std::string::npos)
    // Line is full of whitespaces, drop it.
    str.clear();
  else {
    // Search for comments.
    size_t comment(str.find_first_of(';'));
    if (comment != 0)
      while ((comment != std::string::npos)
             && (str[comment - 1] == '\\'))
        comment = str.find_first_of(';', comment + 1);

    if (comment != std::string::npos)
      // Comment was found, we can safely drop it as last non-whitespace
      // character will obviously comes after it.
      pos = comment;
    else
      // Otherwise drop from the last non-whitespace character.
      ++pos;
    str.erase(pos);

    // Drop initial whitespaces.
    if ((pos = str.find_first_not_of(whitespaces)) != std::string::npos)
      str.erase(0, pos);
    else
      str.clear();
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
std::string& string::trim_left(std::string& str) throw () {
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
std::string& string::trim_right(std::string& str) throw () {
  size_t pos(str.find_last_not_of(whitespaces));
  if (pos == std::string::npos)
    str.clear();
  else
    str.erase(pos + 1);
  return (str);
}
