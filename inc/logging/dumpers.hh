/*
** Copyright 2011 Merethis
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
# define CCE_LOGGING_DUMPERS_HH

# include <QString>
# include "logging/logger.hh"
# include "objects.hh"

namespace        com {
  namespace      centreon {
    namespace    engine {
      namespace  logging {
        QTextStream& operator<<(QTextStream& stream, host const& hst);
        QTextStream& operator<<(QTextStream& stream, service const& svc);
      }
    }
  }
}

#endif // !CCE_LOGGING_DUMPERS_HH
