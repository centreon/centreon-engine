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

#ifndef CCE_CONFIGURATION_OBJECT_INFO_HH
#  define CCE_CONFIGURATION_OBJECT_INFO_HH

#  include <string>
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace              configuration {
  class                object_info {
  public:
                       object_info();
                       object_info(
                         object_ptr obj,
                         std::string const& path,
                         unsigned int line);
                       object_info(object_info const& right);
                       ~object_info() throw ();
    object_info&       operator=(object_info const& right);
    bool               operator==(
                         object_info const& right) const throw ();
    bool               operator!=(
                         object_info const& right) const throw ();
    unsigned int       line() const throw ();
    void               line(unsigned int line) throw ();
    object_ptr object() const throw ();
    void               object(object_ptr obj);
    std::string const& path() const throw ();
    void               path(std::string const& path);

  private:
    unsigned int       _line;
    object_ptr         _obj;
    std::string        _path;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_OBJECT_INFO_HH

