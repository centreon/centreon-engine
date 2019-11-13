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

#ifndef CCE_CONFIGURATION_APPLIER_HOST_HH
#define CCE_CONFIGURATION_APPLIER_HOST_HH

#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace configuration {
// Forward declarations.
class host;
class state;

namespace applier {
class host {
 public:
  host();
  host(host const& right) = delete;
  ~host() throw();
  host& operator=(host const& right) = delete;
  void add_object(configuration::host const& obj);
  void expand_objects(configuration::state& s);
  void modify_object(configuration::host const& obj);
  void remove_object(configuration::host const& obj);
  void resolve_object(configuration::host const& obj);
};
}  // namespace applier
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_APPLIER_HOST_HH
