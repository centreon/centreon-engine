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
#define CCE_TEST_CONFIGURATION_APPLIER_CHKDIFF_HH

#include <iostream>

/**
 *  Check difference between to map of object.
 *
 *  @param[in] l1 The first map.
 *  @param[in] l2 The second map.
 *
 *  @return True if all map is equal, otherwise false.
 */
template <class Key, class T, class Hash, class Pred>
bool compare_with_true_contents(umap<Key, T, Hash, Pred> const& lhs,
                                umap<Key, T, Hash, Pred> const& rhs) {
  bool ret(true);
  for (typename umap<Key, T, Hash, Pred>::const_iterator it(lhs.begin()),
       end(lhs.end());
       it != end; ++it) {
    typename umap<Key, T, Hash, Pred>::const_iterator it_find(
        rhs.find(it->first));
    if (it_find == rhs.end()) {
      std::cerr << "missing object" << std::endl
                << "old " << *it->second << std::endl;
      ret = false;
    } else if (*it_find->second != *it->second) {
      std::cerr << "difference detected" << std::endl
                << "old " << *it->second << std::endl
                << "new " << *it_find->second << std::endl;
      ret = false;
    }
  }
  for (typename umap<Key, T, Hash, Pred>::const_iterator it(rhs.begin()),
       end(rhs.end());
       it != end; ++it) {
    typename umap<Key, T, Hash, Pred>::const_iterator it_find(
        lhs.find(it->first));
    if (it_find == lhs.end()) {
      std::cerr << "missing object" << std::endl
                << "new " << *it->second << std::endl;
      ret = false;
    }
  }
  return (ret);
}

/**
 *  Check difference between to multimap of object.
 *
 *  @param[in] l1 The first multimap.
 *  @param[in] l2 The second multimap.
 *
 *  @return True if all multimap is equal, otherwise false.
 */
template <class Key, class T, class Hash, class Pred>
bool compare_with_true_contents(umultimap<Key, T, Hash, Pred> const& lhs,
                                umultimap<Key, T, Hash, Pred> const& rhs) {
  if (lhs.size() != rhs.size())
    return (false);
  for (typename umap<Key, T, Hash, Pred>::const_iterator it(lhs.begin()),
       end(lhs.end());
       it != end; ++it) {
    bool find(false);
    for (typename umap<Key, T, Hash, Pred>::const_iterator
             it_find(rhs.find(it->first)),
         end(rhs.end());
         it_find != end && it_find->first == it->first; ++it_find) {
      if (*it_find->second == *it->second) {
        find = true;
        break;
      }
    }
    if (!find)
      return (false);
  }
  return (true);
}

/**
 *  Check difference between to list of object.
 *
 *  @param[in] l1 The first list.
 *  @param[in] l2 The second list.
 *
 *  @return True if all list is equal, otherwise false.
 */
template <typename T>
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

template <typename T>
void reset_next_check(T const& map) {
  for (typename T::const_iterator it(map.begin()), end(map.end()); it != end;
       ++it) {
    it->second->next_check = 0;
    it->second->should_be_scheduled = 1;
  }
}

#endif  // !CCE_TEST_CONFIGURATION_APPLIER_CHKDIFF_HH
