/*
** Copyright 2011-2012 Merethis
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

#include <exception>
#include <QDebug>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/macros.hh"
#include "test/objects/create_object.hh"
#include "test/objects/release.hh"

using namespace test::objects;

int main() {
  try {
    release_null_pointer(static_cast<hostsmember const*>(NULL));
    release_objects(&create_hostsmember);
    release_objects(&create_hostsmember, 10);
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    free_memory(get_global_macros());
    return (1);
  }
  return (0);
}
