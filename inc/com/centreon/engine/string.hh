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

#ifndef CCE_STRING_HH
#  define CCE_STRING_HH

#  include <cstring>
#  include <fstream>
#  include <list>
#  include <sstream>
#  include <string>
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                 string {
  bool                    get_next_line(
                            std::ifstream& stream,
                            std::string& line,
                            unsigned int& pos);

  inline char const*      chkstr(char const* str) throw () {
    return (str ? str : "\"NULL\"");
  }

  inline char*            dup(char const* value) {
    if (!value)
      return (NULL);
    char* buf(new char[strlen(value) + 1]);
    return (strcpy(buf, value));
  }

  inline char*            dup(std::string const& value) {
    char* buf(new char[value.size() + 1]);
    return (strcpy(buf, value.c_str()));
  }

  template<typename T>
  inline char*            dup(T value) {
    std::ostringstream oss;
    oss << value;
    std::string const& str(oss.str());
    char* buf(new char[str.size() + 1]);
    strcpy(buf, str.c_str());
    return (buf);
  }

  inline char const*      setstr(char*& buf, char const* value = NULL) {
    delete[] buf;
    return ((buf = string::dup(value)));
  }

  template<typename T>
  inline char const*      setstr(char*& buf, T value) {
    delete[] buf;
    return ((buf = string::dup(value)));
  }

  bool                    split(
                            std::string const& line,
                            std::string& key,
                            std::string& value,
                            char delim);
  std::list<std::string>& split(
                            std::string const& data,
                            std::list<std::string>& out,
                            char delim);

  template<typename T>
  bool                    to(std::string const& str, T& data) {
    std::istringstream iss(str);
    return ((iss >> data) && iss.eof());
  }

  template<typename T, typename U>
  bool                    to(std::string const& str, U& data) {
    T tmp;
    std::istringstream iss(str);
    if ((iss >> tmp) && iss.eof())
      return (false);
    data = static_cast<U>(tmp);
    return (true);

  }
  std::string&            trim(std::string& str) throw ();
  std::string&            trim_left(std::string& str) throw ();
  std::string&            trim_right(std::string& str) throw ();
}

CCE_END()

#endif // !CCE_MISC_STRING_HH
