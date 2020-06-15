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

#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/exceptions/error.hh"

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
bool string::get_next_line(std::ifstream& stream,
                           std::string& line,
                           unsigned int& pos) {
  while (std::getline(stream, line, '\n')) {
    ++pos;
    string::trim(line);
    if (!line.empty()) {
      char c(line[0]);
      if (c != '#' && c != ';' && c != '\x0')
        return true;
    }
  }
  return false;
}

/**
 *  Get key and value from line.
 *
 *  @param[in,out] line  The line to process.
 *  @param[out]    key   The key pointer.
 *  @param[out]    value The value pointer.
 *  @param[in]     delim The delimiter.
 */
bool string::split(std::string& line,
                   char const** key,
                   char const** value,
                   char delim) {
  std::size_t delim_pos(line.find_first_of(delim));
  if (delim_pos == std::string::npos)
    return false;

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

  return true;
}

/**
 *  Get key and value from line.
 *
 *  @param[in]  line  The line to extract data.
 *  @param[out] key   The key to fill.
 *  @param[out] value The value to fill.
 *  @param[in]  delim The delimiter.
 */
bool string::split(std::string const& line,
                   std::string& key,
                   std::string& value,
                   char delim) {
  std::size_t delim_pos(line.find_first_of(delim));
  if (delim_pos == std::string::npos)
    return false;

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

  return true;
}

/**
 *  Split data into element.
 *
 *  @param[in]  data  The data to split.
 *  @param[out] out   The list to fill.
 *  @param[in]  delim The delimiter.
 */
void string::split(std::string const& data,
                   std::list<std::string>& out,
                   char delim) {
  if (data.empty())
    return;

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
void string::split(std::string const& data,
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
void string::split(std::string const& data,
                   std::set<std::pair<std::string, std::string> >& out,
                   char delim) {
  std::list<std::string> elements;
  split(data, elements, delim);
  for (std::list<std::string>::const_iterator it(elements.begin()),
       end(elements.end());
       it != end; ++it) {
    std::list<std::string>::const_iterator first(it++);
    if (it == end)
      throw(engine_error() << "Not enough elements in the line to make pairs");
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
std::string& string::trim(std::string& str) noexcept {
  // First, search backward for the last non-space character.
  size_t pos(str.find_last_not_of(whitespaces));
  if (pos == std::string::npos)
    // Line is full of whitespaces, drop it.
    str.clear();
  else {
    // Search for comments.
    size_t comment = str.find_first_of(';');
    if (comment != 0)
      while ((comment != std::string::npos) && (str[comment - 1] == '\\'))
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
  return str;
}

/**
 *  Trim at the left of the string.
 *
 *  @param[in] str The string.
 *
 *  @return The trimming stream.
 */
std::string& string::trim_left(std::string& str) noexcept {
  size_t pos(str.find_first_not_of(whitespaces));
  if (pos != std::string::npos)
    str.erase(0, pos);
  return str;
}

/**
 *  Trim at the right of the string.
 *
 *  @param[in] str The string.
 *
 *  @return The trimming stream.
 */
std::string& string::trim_right(std::string& str) noexcept {
  size_t pos(str.find_last_not_of(whitespaces));
  if (pos == std::string::npos)
    str.clear();
  else
    str.erase(pos + 1);
  return str;
}

std::string string::extract_perfdata(std::string const& perfdata, std::string const& metric) noexcept {
  size_t pos, pos_start = 0;

  do {
    pos_start = perfdata.find(metric, pos_start);
    pos = pos_start;

    // Metric name not found
    if (pos == std::string::npos)
      return "";

    while (pos > 0 && perfdata[pos - 1] != ' ')
      pos--;

    size_t end = pos + metric.size();
    while (end < perfdata.size() && perfdata[end] != '=')
      end++;

    // Metric name should be from pos to end. We have to verify this

    // Are there quotes?
    size_t p = pos;
    size_t e = end - 1;
    if (perfdata[p] == '\'' && perfdata[e] == '\'') {
      p++;
      e--;
    }

    // Is the metric type specified?
    char c1 = perfdata[p], c2 = perfdata[p + 1], e1 = perfdata[e];
    if (c2 == '[' && e1 == ']' && (c1 == 'a' || c1 == 'd' || c1 == 'g')) {
      p += 2;
      e--;
    }
    if (e - p + 1 == metric.size()) {
      size_t ee = perfdata.find_first_of(" \n\r", end);
      return perfdata.substr(pos, ee - pos);
    }
    pos_start++;
  } while (pos < perfdata.size());
  return "";
}

std::string& string::remove_thresholds(std::string& perfdata) noexcept {
  size_t pos1 = perfdata.find(";");

  if (pos1 == std::string::npos)
    // No ';' so no thresholds in this perfdata
    return perfdata;

  size_t pos2 = perfdata.find(";", pos1 + 1);
  if (pos2 == std::string::npos) {
    // No second threshold. We just have to remove the first one.
    perfdata.resize(pos1);
    return perfdata;
  }

  size_t pos3 = perfdata.find(";", pos2 + 1);
  if (pos3 == std::string::npos) {
    // No min/max. We just have to remove thresholds.
    perfdata.resize(pos1);
    return perfdata;
  }

  perfdata.replace(pos1, pos3 - pos1, ";;");
  return perfdata;
}

void string::check_string_utf8(std::string&& str) {
  for (auto it = str.begin(); it != str.end(); ) {
    if ((*it & ~127) == 0)
      ++it;
    else if ((*it & 192) == 192 && (*(it + 1) & 128) == 128)
      it += 2;
    else if ((*it & 224) == 224 && (*(it + 1) & 128) == 128 && (*(it + 2) & 128) == 128)
      it += 3;
    else if ((*it & 240) == 240 && (*(it + 1) & 128) == 128 && (*(it + 2) & 128) == 128 && (*(it + 3) & 128) == 128)
      it += 4;
    else {
      /* Not a UTF-8 string */
      bool is_cp1252 = true, is_iso8859 = true;
      auto itt = it;

      do {
        unsigned char c = *itt;
        /* not ISO-8859-15 */
        if (c < 32 || (c > 126 && c < 160))
          is_iso8859 = false;
        /* not cp1252 */
        if (c & 128)
          if (c == 129 || c == 141 || c == 143 || c == 144 || c == 155)
            is_cp1252 = false;
        if (!is_cp1252) {
          std::string out;
          std::size_t d = str.distance(str.begin(), it);
          out.reserve(d + 2 * (str.size() - d));
          out = str.substr(0, d);
          while (it != str.end())
            if (*it < 128)
              out.push_back(*it++);
            else {
              out.push_back(0xc2 + (*it > 0xbf));
              out.push_back((*it++ & 0x3f) + 0x80);
            }
          str.swap(out);
          break;
        } else if (!is_iso8859) {
          std::string out;
          std::size_t d = str.distance(str.begin(), it);
          out.reserve(d + 3 * (str.size() - d));
          out = str.substr(0, d);
          while (it != str.end()) {
            c = *it;
            if (c < 128)
              out.push_back(c++);
            else {
              switch (c) {
                case 128:
                  out.append("€");
                  break;
                case 130:
                  out.append("‚");
                  break;
                case 131:
                  out.append("ƒ");
                  break;
                case 132:
                  out.append("„");
                  break;
                case 133:
                  out.append("…");
                  break;
                case 134:
                  out.append("†");
                  break;
                case 135:
                  out.append("‡");
                  break;
                case 136:
                  out.append("ˆ");
                  break;
                case 137:
                  out.append("‰");
                  break;
                case 138:
                  out.append("Š");
                  break;
                case 139:
                  out.append("‹");
                  break;
                case 140:
                  out.append("Œ");
                  break;
                case 142:
                  out.append("Ž");
                  break;
                case 145:
                  out.append("‘");
                  break;
                case 146:
                  out.append("’");
                  break;
                case 147:
                  out.append("“");
                  break;
                case 148:
                  out.append("”");
                  break;
                case 149:
                  out.append("•");
                  break;
                case 150:
                  out.append("–");
                  break;
                case 151:
                  out.append("—");
                  break;
                case 152:
                  out.append("˜");
                  break;
                case 153:
                  out.append("™");
                  break;
                case 154:
                  out.append("š");
                  break;
                case 155:
                  out.append("›");
                  break;
                case 156:
                  out.append("œ");
                  break;
                case 158:
                  out.append("ž");
                  break;
                case 159:
                  out.append("Ÿ");
                  break;
                default:
              out.push_back(0xc2 + (*it > 0xbf));
              out.push_back((*it++ & 0x3f) + 0x80);
              break;
              }
            }
          }
          str.swap(out);
          break;
        }
        ++itt;
      } while (itt != str.end());
    }
  }
}

void string::check_string_utf8(std::string&& str) {
  bool is_cp1252 = true, is_iso8859 = true, is_utf8 = true;
  for (auto it = str.begin(); it != str.end(); ++it) {
    unsigned char c = *it;
    /* not ISO-8859-15 */
    if (c < 32 || (c > 126 && c < 160))
      is_iso8859 = false;
    /* not cp1252 */
    if (c & 128)
      if (c == 129 || c == 141 || c == 143 || c == 144 || c == 155)
        is_cp1252 = false;
    /* UTF-8 */
    if ((c & ~127) == 0 || // One byte
        (c & 192) == 192 && 
  }
}
