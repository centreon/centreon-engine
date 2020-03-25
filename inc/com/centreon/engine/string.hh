/*
** Copyright 2011-2013,2017 Centreon
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

#ifndef CCE_STRING_HH
#define CCE_STRING_HH

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <list>
#include <set>
#include <sstream>
#include <string>
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace string {
bool get_next_line(std::ifstream& stream, std::string& line, unsigned int& pos);

inline char const* chkstr(char const* str) noexcept {
  return (str ? str : "\"NULL\"");
}

inline std::string ctime(time_t const& time) {
  char buf[64];
  buf[0] = 0;
  if (ctime_r(&time, buf))
    buf[strlen(buf) - 1] = 0;
  return (buf);
}

inline char* dup(char const* value) {
  if (!value)
    return (NULL);
  char* buf(new char[strlen(value) + 1]);
  return (strcpy(buf, value));
}

inline char* dup(std::string const& value) {
  char* buf(new char[value.size() + 1]);
  return (strcpy(buf, value.c_str()));
}

template <typename T>
inline char* dup(T value) {
  std::ostringstream oss;
  oss << value;
  std::string const& str(oss.str());
  char* buf(new char[str.size() + 1]);
  strcpy(buf, str.c_str());
  return (buf);
}

template <typename T>
std::string from(T value) {
  std::ostringstream oss;
  oss << value;
  return (oss.str());
}

inline char const* setstr(char*& buf, char const* value = NULL) {
  delete[] buf;
  return ((buf = string::dup(value)));
}

template <typename T>
inline char const* setstr(char*& buf, T value) {
  delete[] buf;
  return ((buf = string::dup(value)));
}

bool split(std::string& line, char const** key, char const** value, char delim);
bool split(std::string const& line,
           std::string& key,
           std::string& value,
           char delim);
void split(std::string const& data, std::list<std::string>& out, char delim);
void split(std::string const& data, std::set<std::string>& out, char delim);
void split(std::string const& data,
           std::set<std::pair<std::string, std::string> >& out,
           char delim);

template <typename T>
inline bool to(char const* str, T& data) {
  std::istringstream iss(str);
  return ((iss >> data) && iss.eof());
}

template <>
inline bool to(char const* str, long& data) {
  char* end(NULL);
  errno = 0;
  data = strtol(str, &end, 10);
  return (!*end && !errno);
}

template <>
inline bool to(char const* str, unsigned long& data) {
  char* end(NULL);
  errno = 0;
  data = strtoul(str, &end, 10);
  return (!*end && !errno);
}

template <>
inline bool to(char const* str, bool& data) {
  unsigned long tmp;
  if (!to(str, tmp))
    return (false);
  data = static_cast<bool>(tmp);
  return (true);
}

template <>
inline bool to(char const* str, double& data) {
  char* end(NULL);
  errno = 0;
  data = strtod(str, &end);
  return (!*end && !errno);
}

template <>
inline bool to(char const* str, float& data) {
  char* end(NULL);
  errno = 0;
  data = strtof(str, &end);
  return (!*end && !errno);
}

template <>
inline bool to(char const* str, int& data) {
  long tmp;
  if (!to(str, tmp) || tmp > std::numeric_limits<int>::max() ||
      tmp < std::numeric_limits<int>::min())
    return (false);
  data = static_cast<int>(tmp);
  return (true);
}

template <>
inline bool to(char const* str, long double& data) {
  char* end(NULL);
  errno = 0;
  data = strtold(str, &end);
  return (!*end && !errno);
}

template <>
inline bool to(char const* str, long long& data) {
  char* end(NULL);
  errno = 0;
  data = strtoll(str, &end, 10);
  return (!*end && !errno);
}

template <>
inline bool to(char const* str, unsigned int& data) {
  unsigned long tmp;
  if (!to(str, tmp) || tmp > std::numeric_limits<unsigned int>::max())
    return (false);
  data = static_cast<unsigned int>(tmp);
  return (true);
}

template <>
inline bool to(char const* str, unsigned long long& data) {
  char* end(NULL);
  errno = 0;
  data = strtoull(str, &end, 10);
  return (!*end && !errno);
}

template <typename T, typename U>
inline bool to(char const* str, U& data) {
  T tmp;
  if (!to(str, tmp))
    return (false);
  data = static_cast<U>(tmp);
  return (true);
}
std::string& trim(std::string& str) noexcept;
std::string& trim_left(std::string& str) noexcept;
std::string& trim_right(std::string& str) noexcept;
std::string extract_perfdata(std::string const& perfdata, std::string const& metric) noexcept;
std::string& remove_thresholds(std::string& perfdata) noexcept;
}  // namespace string

CCE_END()

#endif  // !CCE_MISC_STRING_HH
