##
## Copyright 2011-2013,2015 Merethis
##
## This file is part of Centreon Scheduler.
##
## Centreon Scheduler is free software: you can redistribute it and/or
## modify it under the terms of the GNU General Public License version 2
## as published by the Free Software Foundation.
##
## Centreon Scheduler is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Centreon Scheduler. If not, see
## <http://www.gnu.org/licenses/>.
##

# Set directories.
set(SRC_DIR "${SRC_DIR}/deleter")
set(INC_DIR "${INC_DIR}/com/centreon/engine/deleter")

# Set files.
set(
  FILES
  ${FILES}

  # Sources.
  "${SRC_DIR}/command.cc"
  "${SRC_DIR}/commandsmember.cc"
  "${SRC_DIR}/contact.cc"
  "${SRC_DIR}/contactgroup.cc"
  "${SRC_DIR}/contactgroupsmember.cc"
  "${SRC_DIR}/contactsmember.cc"
  "${SRC_DIR}/customvariablesmember.cc"
  "${SRC_DIR}/daterange.cc"
  "${SRC_DIR}/downtime.cc"
  "${SRC_DIR}/host.cc"
  "${SRC_DIR}/hostdependency.cc"
  "${SRC_DIR}/hostgroup.cc"
  "${SRC_DIR}/hostsmember.cc"
  "${SRC_DIR}/objectlist.cc"
  "${SRC_DIR}/service.cc"
  "${SRC_DIR}/servicedependency.cc"
  "${SRC_DIR}/servicegroup.cc"
  "${SRC_DIR}/servicesmember.cc"
  "${SRC_DIR}/timeperiod.cc"
  "${SRC_DIR}/timeperiodexclusion.cc"
  "${SRC_DIR}/timerange.cc"

  # Headers.
  "${INC_DIR}/command.hh"
  "${INC_DIR}/commandsmember.hh"
  "${INC_DIR}/contact.hh"
  "${INC_DIR}/contactgroup.hh"
  "${INC_DIR}/contactgroupsmember.hh"
  "${INC_DIR}/contactsmember.hh"
  "${INC_DIR}/customvariablesmember.hh"
  "${INC_DIR}/daterange.hh"
  "${INC_DIR}/downtime.hh"
  "${INC_DIR}/host.hh"
  "${INC_DIR}/hostdependency.hh"
  "${INC_DIR}/hostgroup.hh"
  "${INC_DIR}/hostsmember.hh"
  "${INC_DIR}/listmember.hh"
  "${INC_DIR}/objectlist.hh"
  "${INC_DIR}/service.hh"
  "${INC_DIR}/servicedependency.hh"
  "${INC_DIR}/servicegroup.hh"
  "${INC_DIR}/servicesmember.hh"
  "${INC_DIR}/timeperiod.hh"
  "${INC_DIR}/timeperiodexclusion.hh"
  "${INC_DIR}/timerange.hh"

  PARENT_SCOPE
)
