/*
** Copyright 2013 Merethis
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

#include <cstring>
#include <string>
#include <vector>
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

void applier::modify_if_different(char*& s1, char const* s2) {
  if (strcmp(s1, s2)) {
    delete [] s1;
    s1 = NULL;
    s1 = string::dup(s2);
  }
  return ;
}

void applier::modify_if_different(
                char** t1,
                std::vector<std::string> const& t2,
                unsigned int size) {
  unsigned int i(0);
  for (std::vector<std::string>::const_iterator
         it(t2.begin()),
         end(t2.end());
       (it != end) && (i < size);
       ++it, ++i)
    if (!t1[i] || strcmp(t1[i], it->c_str())) {
      delete [] t1[i];
      t1[i] = NULL;
      t1[i] = string::dup(it->c_str());
    }
  while (i < size) {
    delete [] t1[i];
    t1[i] = NULL;
    ++i;
  }
  return ;
}
