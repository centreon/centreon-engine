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

#ifndef CCE_TEST_CONFIGURATION_APPLIER_CHKDIFF_HH
#  define CCE_TEST_CONFIGURATION_APPLIER_CHKDIFF_HH

#  include <iostream>

/**
 *  Check difference between to list of object.
 *
 *  @param[in] l1 The first list.
 *  @param[in] l2 The second list.
 *
 *  @return True if all list is equal, otherwise false.
 */
template<typename T>
static bool chkdiff(T const* l1, T const* l2) {
  T const* obj1(l1);
  T const* obj2(l2);
  while (obj1 && obj2) {
    if (*obj1 != *obj2) {
      std::cerr << "difference detected" << std::endl;
      std::cerr << "old " << *obj1 << std::endl;
      std::cerr << "new " << *obj2 << std::endl;
      return (false);
    }
    obj1 = obj1->next;
    obj2 = obj2->next;
  }
  if (obj1) {
    std::cerr << "missing object" << std::endl;
    std::cerr << "old " << *obj1 << std::endl;
    return (false);
  }
  if (obj2) {
    std::cerr << "missing object" << std::endl;
    std::cerr << "new " << *obj2 << std::endl;
    return (false);
  }
  return (true);
}

#endif // !CCE_TEST_CONFIGURATION_APPLIER_CHKDIFF_HH
