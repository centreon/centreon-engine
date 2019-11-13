/*
** Copyright 2012-2013 Merethis
**
** This file is part of Centreon Clib.
**
** Centreon Clib is free software: you can redistribute it
** and/or modify it under the terms of the GNU Affero General Public
** License as published by the Free Software Foundation, either version
** 3 of the License, or (at your option) any later version.
**
** Centreon Clib is distributed in the hope that it will be
** useful, but WITHOUT ANY WARRANTY; without even the implied warranty
** of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public
** License along with Centreon Clib. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef CCE_COMMANDS_ENVIRONMENT_HH
#define CCE_COMMANDS_ENVIRONMENT_HH

#include <string>
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace commands {
/**
 *  @class process environment.hh "com/centreon/environment.hh"
 *  @brief Allow to get and manage environment.
 *
 *  This class allow to get and set environment.
 */
class environment {
 public:
  environment(char** env = NULL);
  environment(environment const& right);
  ~environment() throw();
  environment& operator=(environment const& right);
  bool operator==(environment const& right) const throw();
  bool operator!=(environment const& right) const throw();
  void add(char const* line);
  void add(char const* name, char const* value);
  void add(std::string const& line);
  void add(std::string const& name, std::string const& value);
  char** data() throw();

 private:
  void _internal_copy(environment const& right);
  void _realoc_buffer(uint32_t size);
  void _realoc_env(uint32_t size);
  void _rebuild_env();

  char* _buffer;
  char** _env;
  uint32_t _pos_buffer;
  uint32_t _pos_env;
  uint32_t _size_buffer;
  uint32_t _size_env;
};
}  // namespace commands

CCE_END()

#endif  // !CC_COMMANDS_ENVIRONMENT_HH
