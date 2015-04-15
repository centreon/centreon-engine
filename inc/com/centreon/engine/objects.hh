/*
** Copyright 1999-2008      Ethan Galstad
** Copyright 2011-2013,2015 Merethis
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

#ifndef CCE_OBJECTS_HH
#  define CCE_OBJECTS_HH

#  include <time.h>
#  include "com/centreon/engine/common.hh"
#  include "com/centreon/engine/objects/command.hh"
#  include "com/centreon/engine/objects/commandsmember.hh"
#  include "com/centreon/engine/objects/customvariablesmember.hh"
#  include "com/centreon/engine/objects/daterange.hh"
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/hostdependency.hh"
#  include "com/centreon/engine/objects/hostsmember.hh"
#  include "com/centreon/engine/objects/objectlist.hh"
#  include "com/centreon/engine/objects/service.hh"
#  include "com/centreon/engine/objects/servicedependency.hh"
#  include "com/centreon/engine/objects/servicesmember.hh"
#  include "com/centreon/engine/objects/timeperiod.hh"
#  include "com/centreon/engine/objects/timeperiodexclusion.hh"
#  include "com/centreon/engine/objects/timerange.hh"
#  include "find.hh"

/*
** Current object revision, Increment when changes are made to data
** structures...
*/
#  define CURRENT_OBJECT_STRUCTURE_VERSION 2000

#endif // !CCE_OBJECTS_HH
