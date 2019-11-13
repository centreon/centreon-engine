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

#ifndef CCE_CONFIGURATION_APPLIER_CONTACT_HH
#define CCE_CONFIGURATION_APPLIER_CONTACT_HH

#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace configuration {
// Forward declarations.
class contact;
class state;

namespace applier {
class contact {
 public:
  contact();
  ~contact() throw();
  void add_object(configuration::contact const& obj);
  void expand_objects(configuration::state& s);
  void modify_object(configuration::contact const& obj);
  void remove_object(configuration::contact const& obj);
  void resolve_object(configuration::contact const& obj);

 private:
  contact(contact const& right);
  contact& operator=(contact const& right);
};
}  // namespace applier
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_APPLIER_CONTACT_HH
