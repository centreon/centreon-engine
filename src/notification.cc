/*
 * Copyright 2019 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */
#include "com/centreon/engine/notification.hh"

#include <algorithm>

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/notifier.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

notification::notification(notifier* parent,
                           notifier::reason_type type,
                           std::string const& author,
                           std::string const& message,
                           uint32_t options,
                           uint64_t notification_id,
                           uint32_t notification_number,
                           uint32_t notification_interval,
                           bool escalated,
                           const std::set<std::string>& notified_contacts)
    : _parent{parent},
      _type{type},
      _author{author},
      _message{message},
      _options{options},
      _id{notification_id},
      _number{notification_number},
      _escalated{escalated},
      _interval{notification_interval},
      _notified_contact{notified_contacts} {}

/**
 *  @brief Execute the notification on each contact in to_notify.
 *
 *  @return OK if at least one notification is sent, and ERROR otherwise.
 */
int notification::execute(std::unordered_set<contact*> const& to_notify) {

  uint32_t contacts_notified{0};

  struct timeval start_time;
  gettimeofday(&start_time, nullptr);

  struct timeval end_time {
    0L, 0L
  };

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
    for (contact_map::const_iterator cit{contact::contacts.begin()},
         cend{contact::contacts.end()};
         cit != cend; ++cit) {
      if (cit->second->get_alias() == _author) {
        author = cit->second.get();
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
    case notifier::reason_acknowledgement:
      mac.x[MACRO_NOTIFICATIONTYPE] = "ACKNOWLEDGEMENT";
      break;
    case notifier::reason_flappingstart:
      mac.x[MACRO_NOTIFICATIONTYPE] = "FLAPPINGSTART";
      break;
    case notifier::reason_flappingstop:
      mac.x[MACRO_NOTIFICATIONTYPE] = "FLAPPINGSTOP";
      break;
    case notifier::reason_flappingdisabled:
      mac.x[MACRO_NOTIFICATIONTYPE] = "FLAPPINGDISABLED";
      break;
    case notifier::reason_downtimestart:
      mac.x[MACRO_NOTIFICATIONTYPE] = "DOWNTIMESTART";
      break;
    case notifier::reason_downtimeend:
      mac.x[MACRO_NOTIFICATIONTYPE] = "DOWNTIMEEND";
      break;
    case notifier::reason_downtimecancelled:
      mac.x[MACRO_NOTIFICATIONTYPE] = "DOWNTIMECANCELLED";
      break;
    case notifier::reason_custom:
      mac.x[MACRO_NOTIFICATIONTYPE] = "CUSTOM";
      break;
    case notifier::reason_recovery:
      mac.x[MACRO_NOTIFICATIONTYPE] = "RECOVERY";
      break;
    default:
      mac.x[MACRO_NOTIFICATIONTYPE] = "PROBLEM";
      break;
  }

  if (_parent->get_notifier_type() == notifier::host_notification) {
    /* set the notification number macro */
    mac.x[MACRO_HOSTNOTIFICATIONNUMBER] = std::to_string(_number);

    /* The $NOTIFICATIONNUMBER$ macro is maintained for backward compatibility
     */
    mac.x[MACRO_NOTIFICATIONNUMBER] = mac.x[MACRO_HOSTNOTIFICATIONNUMBER];

    /* Set the notification id macro */
    mac.x[MACRO_HOSTNOTIFICATIONID] = std::to_string(_id);
  } else {
    /* set the notification number macro */
    mac.x[MACRO_SERVICENOTIFICATIONNUMBER] = std::to_string(_number);

    /* The $NOTIFICATIONNUMBER$ macro is maintained for backward compatibility
     */
    mac.x[MACRO_NOTIFICATIONNUMBER] = mac.x[MACRO_SERVICENOTIFICATIONNUMBER];

    /* Set the notification id macro */
    mac.x[MACRO_SERVICENOTIFICATIONID] = std::to_string(_id);
  }

  for (contact* ctc : to_notify) {
    /* grab the macro variables for this contact */
    grab_contact_macros_r(&mac, ctc);

    /* clear summary macros (they are customized for each contact) */
    clear_summary_macros_r(&mac);

    /* check viability of notifying the user */
    notifier::notification_category cat{notifier::get_category(_type)};
    if (ctc->should_be_notified(cat, _type, *_parent)) {
      /* notify this contact */
      if (_parent->notify_contact(&mac, ctc, _type, _author.c_str(),
                                  _message.c_str(), _options,
                                  _escalated) == OK) {
        /* keep track of how many contacts were notified */
        contacts_notified++;
        _notified_contact.insert(ctc->get_name());
      }
    }
  }

  /* get the time we finished */
  gettimeofday(&end_time, nullptr);

  /* send data to event broker */
  broker_notification_data(NEBTYPE_NOTIFICATION_END, NEBFLAG_NONE, NEBATTR_NONE,
                           _parent->get_notifier_type(), _type, start_time,
                           end_time, (void*)_parent, _author.c_str(),
                           _message.c_str(), _escalated, contacts_notified,
                           nullptr);

  logger(dbg_notifications, basic)
      << contacts_notified << " contacts were notified.";
  return OK;
}

notifier::reason_type notification::get_reason() const {
  return _type;
}

uint32_t notification::get_notification_interval() const {
  return _interval;
}

/**
 * @brief Return a boolean telling if this notification has been sent to the
 * given user.
 *
 * @param user The name of the user.
 *
 * @return a boolean.
 */
bool notification::sent_to(const std::string& user) const {
  return std::find(_notified_contact.begin(), _notified_contact.end(), user) !=
         _notified_contact.end();
}

namespace com {
namespace centreon {
namespace engine {
/**
 *  operator<< to dump a notification in a stream
 *
 * @param os The output stream
 * @param obj The notification to dump.
 *
 * @return The output stream
 */
std::ostream& operator<<(std::ostream& os, notification const& obj) {
  os << "type: " << obj._type << ", author: " << obj._author
     << ", options: " << obj._options << ", escalated: " << obj._escalated
     << ", id: " << obj._id << ", number: " << obj._number
     << ", interval: " << obj._interval << ", contacts: ";

  for (auto& c : obj._notified_contact)
    os << c << ",";
  os << "\n";
  return os;
}
}  // namespace engine
}  // namespace centreon
}  // namespace com
