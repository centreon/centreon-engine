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

#ifndef CCE_HOST_HH
#  define CCE_HOST_HH

#  include "com/centreon/engine/notifications/notifier.hh"

CCE_BEGIN()

/**
 *  @class host host.hh "com/centreon/engine/host.hh"
 *  @brief This class represents a host.
 *
 *  A host is checkable and also a notifier.
 */
class               host : public notifier {
 public:
                    host();
                    host(host const& other);
                    ~host();
  host&             operator=(host const& other);
};

CCE_END()

#endif // !CCE_HOST_HH
