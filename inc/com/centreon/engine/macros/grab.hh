/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2013 Merethis
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

#ifndef CCE_MACROS_GRAB_HH
#  define CCE_MACROS_GRAB_HH

#  include <iomanip>
#  include <sstream>
#  include <time.h>
#  include "com/centreon/engine/macros/process.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/string.hh"

CCE_BEGIN()

namespace  macros {
  /**
   *  Extract double.
   *
   *  @param[in] t   Host object.
   *  @param[in] mac Unused.
   *
   *  @return Newly allocated string with value as a fixed point string.
   */
  template <typename T, double (T::* member)() const, unsigned int precision>
  char*    get_double(T& t, nagios_macros* mac) {
    (void)mac;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision)
        << (t.*member)();
    return string::dup(oss.str());
  }

  template <typename T, typename V, double (V::* member)() const, unsigned int precision>
  char*    get_double(T& t, nagios_macros* mac) {
    (void)mac;

    V* v{&t};
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision)
        << (v->*member)();
    return string::dup(oss.str());
  }

  /**
   *  Extract double.
   *
   *  @param[in] t   Host object.
   *  @param[in] mac Unused.
   *
   *  @return Newly allocated string with value as a fixed point string.
   */
  //TODO SGA to remove after service rework
  template <typename T, double (T::* member), unsigned int precision>
  char*    get_double(T& t, nagios_macros* mac) {
    (void)mac;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision)
        << t.*member;
    return string::dup(oss.str());
  }

  /**
   *  Extract duration.
   *
   *  @param[in] t   Base object.
   *  @param[in] mac Unused.
   *
   *  @return Duration in a newly allocated string.
   */
  template <typename T>
  char*    get_duration(T& t, nagios_macros* mac) {
    (void)mac;

    // Get duration.
    time_t now(time(NULL));
    unsigned long duration(now - t.get_last_state_change());

    // Break down duration.
    unsigned int days(duration / (24 * 60 * 60));
    duration %= (24 * 60 * 60);
    unsigned int hours(duration / (60 * 60));
    duration %= (60 * 60);
    unsigned int minutes(duration / 60);
    duration %= 60;

    // Stringify duration.
    std::ostringstream oss;
    oss << days << "d "
        << hours << "h "
        << minutes << "m "
        << duration << "s";
    return string::dup(oss.str());
  }

  /**
   *  Extract duration.
   *
   *  @param[in] t   Base object.
   *  @param[in] mac Unused.
   *
   *  @return Duration in a newly allocated string.
   */
  //TODO SGA to remove with service rework
  template <typename T>
  char*    get_duration_old(T& t, nagios_macros* mac) {
    (void)mac;

    // Get duration.
    time_t now(time(NULL));
    unsigned long duration(now - t.get_last_state_change());

    // Break down duration.
    unsigned int days(duration / (24 * 60 * 60));
    duration %= (24 * 60 * 60);
    unsigned int hours(duration / (60 * 60));
    duration %= (60 * 60);
    unsigned int minutes(duration / 60);
    duration %= 60;

    // Stringify duration.
    std::ostringstream oss;
    oss << days << "d "
        << hours << "h "
        << minutes << "m "
        << duration << "s";
    return string::dup(oss.str());
  }

  /**
   *  Extract duration in seconds.
   *
   *  @param[in] t   Base object.
   *  @param[in] mac Unused.
   *
   *  @return Duration in second in a newly allocated string.
   */
  template <typename T>
  char*    get_duration_sec(T& t, nagios_macros* mac) {
    (void)mac;

    // Get duration.
    time_t now(time(NULL));
    unsigned long duration(now - t.get_last_state_change());
    return string::dup(duration);
  }

  /**
   *  Extract duration in seconds.
   *
   *  @param[in] t   Base object.
   *  @param[in] mac Unused.
   *
   *  @return Duration in second in a newly allocated string.
   */
  //TODO SGA to remove with service
  template <typename T>
  char*    get_duration_sec_old(T& t, nagios_macros* mac) {
    (void)mac;

    // Get duration.
    time_t now(time(NULL));
    unsigned long duration(now - t.get_last_state_change());
    return string::dup(duration);
  }

  /**
   *  Copy macro.
   *
   *  @param[in] t   Unused.
   *  @param[in] mac Macro array.
   *
   *  @return Copy of the requested macro.
   */
  template <typename T, unsigned int macro_id>
  char*    get_macro_copy(T& t, nagios_macros* mac) {
    (void)t;
    return string::dup(mac->x[macro_id] ? mac->x[macro_id] : "");
  }

  /**
   *  Get string copy of object member.
   *
   *  @param[in] t   Base object.
   *  @param[in] mac Unused.
   *
   *  @return String copy of object member.
   */
  template <typename T, typename U, U (T::* member)() const>
  char*    get_member_as_string(T& t, nagios_macros* mac) {
    (void)mac;
    return string::dup((t.*member)());
  }

  /**
   *  Get string copy of object member.
   *
   *  @param[in] t   Base object.
   *  @param[in] mac Unused.
   *
   *  @return String copy of object member.
   */
  template <typename T, typename U, typename V, U (V::* member)() const>
  char*    get_member_as_string(T& t, nagios_macros* mac) {
    (void)mac;
    V* v{&t};
    return string::dup((v->*member)());
  }

  /**
   *  Get string copy of object member.
   *
   *  @param[in] t   Base object.
   *  @param[in] mac Unused.
   *
   *  @return String copy of object member.
   */
//TODO SGA to remove after service
  template <typename T, typename U, U (T::* member)>
  char*    get_member_as_string(T& t, nagios_macros* mac) {
    (void)mac;
    return string::dup(t.*member);
  }

  /**
   *  Recursively process macros.
   *
   *  @param[in] hst Host object.
   *  @param[in] mac Unused.
   *
   *  @return Newly allocated string with macros processed.
   */
  template <typename T, std::string const& (T::* member)() const, unsigned int options>
  char* get_recursive(T& t, nagios_macros* mac) {
    (void)mac;

    // Get copy of string with macros processed.
    char* buffer(NULL);
    process_macros_r(mac, (t.*member)().c_str(), &buffer, options);
    return buffer;
  }
  /**
   *  Recursively process macros.
   *
   *  @param[in] hst Host object.
   *  @param[in] mac Unused.
   *
   *  @return Newly allocated string with macros processed.
   */
  template <typename T, typename V, std::string const& (V::* member)() const, unsigned int options>
  char* get_recursive(T& t, nagios_macros* mac) {
    (void)mac;

    // Get copy of string with macros processed.
    char* buffer(NULL);
    V* v{&t};
    process_macros_r(mac, (v->*member)().c_str(), &buffer, options);
    return buffer;
  }

  /**
   *  Recursively process macros.
   *
   *  @param[in] hst Host object.
   *  @param[in] mac Unused.
   *
   *  @return Newly allocated string with macros processed.
   */
  //TODO SGA to remove after service rework
  template <typename T, char* (T::* member), unsigned int options>
  char* get_recursive(T& t, nagios_macros* mac) {
    (void)mac;

    // Get copy of string with macros processed.
    char* buffer(NULL);
    process_macros_r(mac, t.*member, &buffer, options);
    return buffer;
  }
  /**
   *  Extract state type.
   *
   *  @param[in] t   Base object.
   *  @param[in] mac Unused.
   *
   *  @return Newly allocated state type as a string.
   */
  template <typename T>
  char* get_state_type(T& t, nagios_macros* mac) {
    (void)mac;
    return (string::dup((t.get_state_type() == HARD_STATE)
                      ? "HARD"
                      : "SOFT"));
  }
  /**
   *  Extract state type.
   *
   *  @param[in] t   Base object.
   *  @param[in] mac Unused.
   *
   *  @return Newly allocated state type as a string.
   */
  //TODO SGA to remove after service rework
  template <typename T>
  char* get_state_type_old(T& t, nagios_macros* mac) {
    (void)mac;
    return (string::dup((t.state_type == HARD_STATE)
                      ? "HARD"
                      : "SOFT"));
  }
}

CCE_END()

#endif // !CCE_MACROS_GRAB_HH
