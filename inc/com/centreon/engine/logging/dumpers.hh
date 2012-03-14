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

#  include <QTextStream>
#  include "com/centreon/engine/logging/logger.hh"
#  include "com/centreon/engine/objects.hh"

struct sched_info_struct;

namespace            com {
  namespace          centreon {
    namespace        engine {
      namespace      logging {
        void         dump_object_list();
        QTextStream& operator<<(QTextStream& stream, host const& hst);
        QTextStream& operator<<(QTextStream& stream, service const& svc);
        QTextStream& operator<<(QTextStream& stream, sched_info_struct const& sched);
        QTextStream& operator<<(QTextStream& stream, command const& cmd);
        QTextStream& operator<<(QTextStream& stream, contact const& cntct);
        QTextStream& operator<<(QTextStream& stream, hostgroup const& hstgrp);
        QTextStream& operator<<(QTextStream& stream, servicegroup const& svcgrp);
        QTextStream& operator<<(QTextStream& stream, contactgroup const& cntctgrp);
        QTextStream& operator<<(QTextStream& stream, serviceescalation const& escalation);
        QTextStream& operator<<(QTextStream& stream, servicedependency const& dependency);
        QTextStream& operator<<(QTextStream& stream, hostescalation const& escalation);
        QTextStream& operator<<(QTextStream& stream, hostdependency const& dependency);
      }
    }
  }
}

#endif // !CCE_LOGGING_DUMPERS_HH
