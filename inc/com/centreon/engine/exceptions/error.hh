/*
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

#ifndef CCE_EXCEPTIONS_ERROR_HH
#define CCE_EXCEPTIONS_ERROR_HH

#include <exception>
#include <string>
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace exceptions {
/**
 *  @class error error.hh
 *  @brief Base exception class.
 *
 *  Simple exception class containing an error message and a flag to
 *  determine if the error that generated the exception was either fatal
 *  or not.
 */
class error : public std::exception {
  template <typename T>
  void _insert_with_snprintf(T& t, char const* format);

  mutable char _buffer[4096];
  uint32_t _current;

 public:
  error() noexcept;
  error(char const* file, char const* function, int line) noexcept;
  error(error const& e) noexcept;
  ~error() throw() override;
  error& operator=(error const& e) noexcept;
  error& operator<<(char c) noexcept;
  error& operator<<(char const* str) noexcept;
  error& operator<<(int i) noexcept;
  error& operator<<(unsigned long u) noexcept;
  error& operator<<(unsigned int u) noexcept;
  error& operator<<(long l) noexcept;
  error& operator<<(long long ll) noexcept;
  error& operator<<(unsigned long long ull) noexcept;
  error& operator<<(double d) noexcept;
  error& operator<<(std::string const& str) noexcept;
  char const* what() const throw() override;
};

}

CCE_END()

#ifdef NDEBUG
#define engine_error() com::centreon::engine::exceptions::error()
#else
#define engine_error() \
  com::centreon::engine::exceptions::error(__FILE__, __func__, __LINE__)
#endif  // !NDEBUG

#endif  // !CCE_EXCEPTIONS_ERROR_HH
