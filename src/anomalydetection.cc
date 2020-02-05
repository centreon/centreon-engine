/*
** Copyright 2020 Centreon
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

#include "com/centreon/engine/anomalydetection.hh"

using namespace com::centreon::engine;

anomalydetection::anomalydetection(std::string const& hostname,
                                   std::string const& description,
                                   std::string const& display_name,
                                   std::string const& check_command,
                                   bool checks_enabled,
                                   bool accept_passive_checks,
                                   enum service::service_state initial_state,
                                   uint32_t check_interval,
                                   uint32_t retry_interval,
                                   uint32_t notification_interval,
                                   int max_attempts,
                                   uint32_t first_notification_delay,
                                   uint32_t recovery_notification_delay,
                                   std::string const& notification_period,
                                   bool notifications_enabled,
                                   bool is_volatile,
                                   std::string const& check_period,
                                   std::string const& event_handler,
                                   bool event_handler_enabled,
                                   std::string const& notes,
                                   std::string const& notes_url,
                                   std::string const& action_url,
                                   std::string const& icon_image,
                                   std::string const& icon_image_alt,
                                   bool flap_detection_enabled,
                                   double low_flap_threshold,
                                   double high_flap_threshold,
                                   bool check_freshness,
                                   int freshness_threshold,
                                   bool obsess_over,
                                   std::string const& timezone)
    : service{hostname,                    description,
              display_name,                check_command,
              checks_enabled,              accept_passive_checks,
              initial_state,               check_interval,
              retry_interval,              notification_interval,
              max_attempts,                first_notification_delay,
              recovery_notification_delay, notification_period,
              notifications_enabled,       is_volatile,
              check_period,                event_handler,
              event_handler_enabled,       notes,
              notes_url,                   action_url,
              icon_image,                  icon_image_alt,
              flap_detection_enabled,      low_flap_threshold,
              high_flap_threshold,         check_freshness,
              freshness_threshold,         obsess_over,
              timezone} {}
