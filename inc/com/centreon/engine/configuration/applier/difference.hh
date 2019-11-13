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

#ifndef CCE_CONFIGURATION_APPLIER_DIFFERENCE_HH
#define CCE_CONFIGURATION_APPLIER_DIFFERENCE_HH

#include <iterator>
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace configuration {
namespace applier {
template <typename T>
class difference {
 public:
  difference() {}
  difference(T const& old_data, T const& new_data) {
    parse(old_data, new_data);
  }
  ~difference() throw() {}
  T const& added() const throw() { return (_added); }
  T const& deleted() const throw() { return (_deleted); }
  T const& modified() const throw() { return (_modified); }
  void parse(T const& old_data, T const& new_data) {
    parse(old_data.begin(), old_data.end(), new_data.begin(), new_data.end());
  }
  void parse(typename T::const_iterator first1,
             typename T::const_iterator last1,
             typename T::const_iterator first2,
             typename T::const_iterator last2) {
    std::insert_iterator<T> add(std::inserter(_added, _added.begin()));

    if (first1 == last1) {
      std::copy(first2, last2, add);
      return;
    }

    std::insert_iterator<T> del(std::inserter(_deleted, _deleted.begin()));
    std::insert_iterator<T> modif(std::inserter(_modified, _modified.begin()));

    while (first1 != last1) {
      if (first2 == last2) {
        std::copy(first1, last1, del);
        break;
      }

      if (first1->key() < first2->key())
        *del++ = *first1++;
      else if (first1->key() != first2->key())
        *add++ = *first2++;
      else if (*first1 != *first2) {
        *modif++ = *first2++;
        ++first1;
      } else {
        ++first2;
        ++first1;
      }
    }

    if (first2 != last2)
      std::copy(first2, last2, add);
  }

 private:
  T _added;
  T _deleted;
  T _modified;
};
}  // namespace applier
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_APPLIER_DIFFERENCE_HH
