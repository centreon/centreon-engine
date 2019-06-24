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
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/notification.hh"
#include "com/centreon/engine/notifier.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

notification::notification(notifier* parent,
                           notifier::reason_type type,
                           std::string const& author,
                           std::string const& message,
                           uint32_t options,
                           uint64_t notification_id,
                           uint32_t notification_number)
    : _parent{parent},
      _type{type},
      _author{author},
      _message{message},
      _options{options},
      _id{notification_id},
      _number{notification_number} {
  switch (_type) {
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
  uint32_t contacts_notified{0};

  struct timeval start_time;
  gettimeofday(&start_time, nullptr);

  struct timeval end_time{0L, 0L};

  /* send data to event broker */
  int neb_result{broker_notification_data(
      NEBTYPE_NOTIFICATION_START, NEBFLAG_NONE, NEBATTR_NONE,
      _parent->get_notifier_type(), _type, start_time, end_time, (void*)_parent,
      _author.c_str(), _message.c_str(), _escalated, 0, nullptr)};

  if (neb_result == NEBERROR_CALLBACKCANCEL)
    return ERROR;
  else if (neb_result == NEBERROR_CALLBACKOVERRIDE) {
    return OK;
  }

  nagios_macros mac;

  /* Grab the macro variables */
  _parent->grab_macros_r(&mac);

  contact* author{nullptr};
  contact_map::const_iterator it{contact::contacts.find(_author)};
  if (it != contact::contacts.end())
    author = it->second.get();
  else {
    for (contact_map::const_iterator
        cit{contact::contacts.begin()},
        cend{contact::contacts.end()};
        cit != cend; ++cit) {
      if (cit->second->get_alias() == _author) {
        author = it->second.get();
        break;
      }
    }
  }

  /* Get author and comment macros */
  mac.x[MACRO_NOTIFICATIONAUTHOR] = _author;
  mac.x[MACRO_NOTIFICATIONCOMMENT] = _message;
  if (author) {
    mac.x[MACRO_NOTIFICATIONAUTHORNAME] = author->get_name();
    mac.x[MACRO_NOTIFICATIONAUTHORALIAS] = author->get_alias();
  } else {
    mac.x[MACRO_NOTIFICATIONAUTHORNAME] = "";
    mac.x[MACRO_NOTIFICATIONAUTHORALIAS] = "";
  }

  /* set the notification type macro */
  switch (_type) {
    case notifier::notification_acknowledgement:
      mac.x[MACRO_NOTIFICATIONTYPE] = "ACKNOWLEDGEMENT";
      break;
    case notifier::notification_flappingstart:
      mac.x[MACRO_NOTIFICATIONTYPE] = "FLAPPINGSTART";
      break;
    case notifier::notification_flappingstop:
      mac.x[MACRO_NOTIFICATIONTYPE] = "FLAPPINGSTOP";
      break;
    case notifier::notification_flappingdisabled:
      mac.x[MACRO_NOTIFICATIONTYPE] = "FLAPPINGDISABLED";
      break;
    case notifier::notification_downtimestart:
      mac.x[MACRO_NOTIFICATIONTYPE] = "DOWNTIMESTART";
      break;
    case notifier::notification_downtimeend:
      mac.x[MACRO_NOTIFICATIONTYPE] = "DOWNTIMEEND";
      break;
    case notifier::notification_downtimecancelled:
      mac.x[MACRO_NOTIFICATIONTYPE] = "DOWNTIMECANCELLED";
      break;
    case notifier::notification_custom:
      mac.x[MACRO_NOTIFICATIONTYPE] = "CUSTOM";
      break;
    case notifier::notification_recovery:
      mac.x[MACRO_NOTIFICATIONTYPE] = "RECOVERY";
      break;
    default:
      mac.x[MACRO_NOTIFICATIONTYPE] = "PROBLEM";
      break;
  }

  if (_parent->get_notifier_type() == notifier::host_notification) {
    /* set the notification number macro */
    mac.x[MACRO_HOSTNOTIFICATIONNUMBER] = std::to_string(_number);

    /* The $NOTIFICATIONNUMBER$ macro is maintained for backward compatibility */
    mac.x[MACRO_NOTIFICATIONNUMBER] = mac.x[MACRO_HOSTNOTIFICATIONNUMBER];

    /* Set the notification id macro */
    mac.x[MACRO_HOSTNOTIFICATIONID] = std::to_string(_id);
  }
  else {
    /* set the notification number macro */
    mac.x[MACRO_SERVICENOTIFICATIONNUMBER] = std::to_string(_number);

    /* The $NOTIFICATIONNUMBER$ macro is maintained for backward compatibility */
    mac.x[MACRO_NOTIFICATIONNUMBER] = mac.x[MACRO_SERVICENOTIFICATIONNUMBER];

    /* Set the notification id macro */
    mac.x[MACRO_SERVICENOTIFICATIONID] = std::to_string(_id);
  }

  for (contact* ctc : to_notify) {
    /* grab the macro variables for this contact */
    grab_contact_macros_r(&mac, ctc);

    /* clear summary macros (they are customized for each contact) */
    clear_summary_macros_r(&mac);

    /* notify this contact */
    int result =
        _parent->notify_contact(&mac, ctc, _type, _author.c_str(),
                                _message.c_str(), _options, _escalated);

    /* keep track of how many contacts were notified */
    if (result == OK)
      contacts_notified++;
  }

  logger(dbg_notifications, basic)
      << contacts_notified << " contacts were notified.";
  return OK;
}
