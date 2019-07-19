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
  std::string get_double(T& t, nagios_macros* mac) {
    (void)mac;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision)
        << (t.*member)();
    return oss.str();
  }

  template <typename T, typename V, double (V::* member)() const, unsigned int precision>
  std::string get_double(T& t, nagios_macros* mac) {
    (void)mac;

    V* v{&t};
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision)
        << (v->*member)();
    return oss.str();
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
  std::string get_double(T& t, nagios_macros* mac) {
    (void)mac;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision)
        << t.*member;
    return oss.str();
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
  std::string get_duration(T& t, nagios_macros* mac) {
    (void)mac;

    // Get duration.
    time_t now(time(nullptr));
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
    return oss.str();
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
  std::string get_duration_sec(T& t, nagios_macros* mac) {
    (void)mac;

    // Get duration.
    time_t now(time(nullptr));
    unsigned long duration(now - t.get_last_state_change());
    return std::to_string(duration);
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
  std::string get_macro_copy(T& t, nagios_macros* mac) {
    (void)t;
    return mac->x[macro_id];
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
  std::string get_member_as_string(T& t, nagios_macros* mac) {
    (void)mac;
    std::stringstream ss;
    ss << (t.*member)();
    return ss.str();
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
  std::string get_member_as_string(T& t, nagios_macros* mac) {
    (void)mac;
    V* v{&t};
    std::stringstream ss;
    ss << (v->*member)();
    return ss.str();
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
  std::string get_member_as_string(T& t, nagios_macros* mac) {
    (void)mac;
    std::stringstream ss;
    ss << t.*member;
    return ss.str();
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
  std::string get_recursive(T& t, nagios_macros* mac) {
    (void)mac;

    // Get copy of string with macros processed.
    std::string buffer;
    process_macros_r(mac, (t.*member)(), buffer, options);
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
  std::string get_recursive(T& t, nagios_macros* mac) {
    (void)mac;

    // Get copy of string with macros processed.
    std::string buffer;
    V* v{&t};
    process_macros_r(mac, (v->*member)(), buffer, options);
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
  std::string get_state_type(T& t, nagios_macros* mac) {
    (void)mac;
    return (t.get_state_type() == notifier::hard)
                      ? "HARD"
                      : "SOFT";
  }
}

CCE_END()

#endif // !CCE_MACROS_GRAB_HH
