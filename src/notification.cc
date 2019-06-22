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
#include "com/centreon/engine/notification.hh"

using namespace com::centreon::engine;

notification::notification(notifier::reason_type type, std::string const& author, std::string const& message, uint32_t options, uint64_t notification_id)
    : _type{type},
      _author{author},
      _message{message},
      _options{options},
      _id{notification_id} {
  switch (type) {
    case notifier::notification_normal:
      _category = notifier::cat_normal;
      break;
      case notifier::notification_recovery:
      _category = notifier::cat_recovery;
      break;
    case notifier::notification_acknowledgement:
      _category = notifier::cat_acknowledgement;
      break;
    case notifier::notification_flappingstop:
    case notifier::notification_flappingstart:
    case notifier::notification_flappingdisabled:
      _category = notifier::cat_flapping;
      break;
    case notifier::notification_downtimestart:
    case notifier::notification_downtimeend:
    case notifier::notification_downtimecancelled:
      _category = notifier::cat_downtime;
      break;
    case notifier::notification_custom:
      _category = notifier::cat_custom;
      break;
  }
}

int notification::execute(std::unordered_set<contact*> const& to_notify) {
  return OK;
}
