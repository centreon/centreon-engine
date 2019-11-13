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

#ifndef CCE_CONFIGURATION_GROUP_HH
#define CCE_CONFIGURATION_GROUP_HH

#include <list>
#include <set>
#include <string>
#include <utility>
#include "com/centreon/engine/namespace.hh"

typedef std::list<std::string> list_string;
typedef std::set<std::string> set_string;
typedef std::set<std::pair<std::string, std::string> > set_pair_string;

CCE_BEGIN()

namespace configuration {
template <typename T>
class group {
 public:
  group(bool inherit = false);
  group(group const& other);
  ~group() throw();
  group& operator=(group const& other);
  group& operator=(std::string const& other);
  group& operator+=(group const& other);
  bool operator==(group const& other) const throw();
  bool operator!=(group const& other) const throw();
  bool operator<(group const& other) const throw();
  T& operator*() throw() { return (_data); }
  T const& operator*() const throw() { return (_data); }
  T* operator->() throw() { return (&_data); }
  T const* operator->() const throw() { return (&_data); }
  T& get() throw() { return (_data); }
  T const& get() const throw() { return (_data); }
  bool is_inherit() const throw() { return (_is_inherit); }
  void is_inherit(bool enable) throw() { _is_inherit = enable; }
  bool is_set() const throw() { return (_is_set); }
  void reset();

 private:
  T _data;
  bool _is_inherit;
  bool _is_null;
  bool _is_set;
};
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_GROUP_HH
