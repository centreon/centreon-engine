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

#ifndef CCE_CONFIGURATION_FILE_INFO_HH
#define CCE_CONFIGURATION_FILE_INFO_HH

#include <string>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace configuration {
class file_info {
 public:
  file_info(std::string const& path = "", unsigned int line = 0)
      : _line(line), _path(path) {}
  file_info(file_info const& right) { operator=(right); }
  ~file_info() throw() {}
  file_info& operator=(file_info const& right) {
    if (this != &right) {
      _line = right._line;
      _path = right._path;
    }
    return (*this);
  }
  bool operator==(file_info const& right) const throw() {
    return (_line == right._line && _path == right._path);
  }
  bool operator!=(file_info const& right) const throw() {
    return (!operator==(right));
  }
  friend error& operator<<(error& err, file_info const& info) {
    err << "in file '" << info.path() << "' on line " << info.line();
    return (err);
  }
  unsigned int line() const throw() { return (_line); }
  void line(unsigned int line) throw() { _line = line; }
  std::string const& path() const throw() { return (_path); }
  void path(std::string const& path) { _path = path; }

 private:
  unsigned int _line;
  std::string _path;
};
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_FILE_INFO_HH
