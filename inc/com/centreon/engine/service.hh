/*
** Copyright 2017 Centreon
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

#ifndef CCE_SERVICE_HH
#  define CCE_SERVICE_HH

#  include "com/centreon/engine/notifications/notifier.hh"

CCE_BEGIN()

/**
 *  @class service service.hh "com/centreon/engine/service.hh"
 *  @brief Service as a host's service.
 *
 *  This class represents a service. It is checkable and also a notifier.
 */
class               service : public notifier {
 public:
                    service();
                    service(service const& other);
                    ~service();
  service&          operator=(service const& other);
};

CCE_END()

#endif // !CCE_SERVICE_HH
