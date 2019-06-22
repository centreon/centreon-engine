/*
** Copyright 2019 Centreon
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

#ifndef CCE_NOTIFICATION_HH
#define CCE_NOTIFICATION_HH

#include <memory>
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/notifier.hh"

CCE_BEGIN()
class contact;

class notification {
 public:
  notification(notifier::reason_type type, std::string const& author, std::string const& message, uint32_t options, uint64_t notification_id);
  int execute(std::unordered_set<contact*> const& to_notify);

 private:
  notifier::reason_type _type;
  notifier::notification_category _category;
  std::string _author;
  std::string _message;
  uint32_t _options;
  uint64_t _id;
};
CCE_END()

#endif  // !CCE_NOTIFICATION_HH
