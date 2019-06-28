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

#ifndef CCE_NOTIFICATION_HH
#define CCE_NOTIFICATION_HH

#include <memory>
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/notifier.hh"

CCE_BEGIN()
class contact;

class notification {
 public:
  notification(notifier* parent,
               notifier::reason_type type,
               std::string const& author,
               std::string const& message,
               uint32_t options,
               uint64_t notification_id,
               uint32_t notification_number);
  int execute(std::unordered_set<contact*>&& to_notify);
  notifier::reason_type get_reason() const;

 private:
  notifier* _parent;
  notifier::reason_type _type;
  notifier::notification_category _category;
  std::string _author;
  std::string _message;
  uint32_t _options;
  bool _escalated;
  uint64_t _id;
  uint32_t _number;
  std::unordered_set<contact*> _notified_contacts;
};
CCE_END()

#endif  // !CCE_NOTIFICATION_HH
