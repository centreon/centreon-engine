/*
** Copyright 2014 Merethis
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

#ifndef CCE_CHECKS_VIABILITY_FAILURE_HH
#define CCE_CHECKS_VIABILITY_FAILURE_HH

#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace checks {
/**
 *  @class viability_failure viability_failure.hh
 * "com/centreon/engine/checks/viability_failure.hh"
 *  @brief Exception thrown on check viability failure.
 *
 *  If a check cannot be run because of a viability check (last
 *  execution time, dependencies, ...) this exception is thrown.
 */
class viability_failure : public exceptions::error {
 public:
  viability_failure();
  viability_failure(char const* file, char const* function, int line);
  viability_failure(viability_failure const& other);
  ~viability_failure() noexcept override;
  viability_failure& operator=(viability_failure const& other);
  template <typename T>
  viability_failure& operator<<(T const& t) {
    exceptions::error::operator<<(t);
    return *this;
  }
};
}  // namespace checks

CCE_END()

#ifdef NDEBUG
#define checks_viability_failure() \
  com::centreon::engine::checks::viability_failure()
#else
#define checks_viability_failure() \
  com::centreon::engine::checks::viability_failure(__FILE__, __func__, __LINE__)
#endif  // !NDEBUG

#endif  // !CCE_CHECKS_VIABILITY_FAILURE_HH
