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

#ifndef CCE_MISC_OBJECT_HH
#  define CCE_MISC_OBJECT_HH

#  include <cstring>
#  include <fstream>
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace misc {
  template<typename T>
  inline std::ostream& chkobj(std::ostream& os, T const* obj) throw () {
    if (obj)
      os << *obj;
    else
      os << "\"NULL\"";
    return (os);
  }

  inline bool             is_equal(
                            char const* str1,
                            char const* str2) throw () {
    return (str1 == str2 || (str1 && str2 && !strcmp(str1, str2)));
  }

  inline bool             is_equal(
                            char* const* tab1,
                            char* const* tab2,
                            unsigned int size) throw () {
    for (unsigned int i(0); i < size; ++i)
      if (!is_equal(tab1[i], tab2[i]))
        return (false);
    return (true);
  }

  template<typename T>
  inline bool             is_equal(
                            T const* tab1,
                            T const* tab2,
                            unsigned int size) throw () {
    for (unsigned int i(0); i < size; ++i)
      if (tab1[i] != tab2[i])
        return (false);
    return (true);
  }

  template<typename T>
  inline bool is_equal(T const* obj1, T const* obj2) throw () {
    return (obj1 == obj2 || (obj1 && obj2 && *obj1 == *obj2));
  }
}

CCE_END()

#endif // !CCE_MISC_OBJECT_HH
