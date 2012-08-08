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

#ifndef CCE_LOGGING_DUMPERS_HH
#  define CCE_LOGGING_DUMPERS_HH

#  include <sstream>
#  include "com/centreon/engine/logging/logger.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/objects.hh"

struct sched_info_struct;

CCE_BEGIN()

namespace             logging {
  void                dump_object_list();
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        host const& hst);
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        service const& svc);
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        sched_info_struct const& sched);
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        command const& cmd);
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        contact const& cntct);
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        hostgroup const& hstgrp);
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        servicegroup const& svcgrp);
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        contactgroup const& cntctgrp);
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        serviceescalation const& escalation);
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        servicedependency const& dependency);
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        hostescalation const& escalation);
  std::ostringstream& operator<<(
                        std::ostringstream& stream,
                        hostdependency const& dependency);
}

CCE_END()

#endif // !CCE_LOGGING_DUMPERS_HH
