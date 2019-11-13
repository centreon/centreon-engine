/*
** Copyright 2012-2013 Merethis
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

#include "embedded_perl.h"
#include "common.h"

extern "C" {
/**
 *  @brief Deinitialize Embedded Perl.
 *
 *  @warning This function is obsolete.
 *
 *  Centreon Engine does not support Embedded Perl anymore. It instead
 *  relies on Centreon Connector Perl.
 *
 *  @return ERROR.
 */
int deinit_embedded_perl() {
  return (ERROR);
}

/**
 *  @brief Embedded Perl related function.
 *
 *  @warning This function is obsolete.
 *
 *  Centreon Engine does not support Embedded Perl anymore. It instead
 *  relies on Centreon Connector Perl.
 *
 *  @param[in] param Unused.
 *
 *  @return ERROR.
 */
int file_uses_embedded_perl(void const* param) {
  (void)param;
  return (ERROR);
}

/**
 *  @brief Initialize Embedded Perl.
 *
 *  @warning This function is obsolete.
 *
 *  Centreon Engine does not support Embedded Perl anymore. It instead
 *  relies on Centreon Connector Perl.
 *
 *  @param[in] param Unused.
 *
 *  @return ERROR.
 */
int init_embedded_perl(void const* param) {
  (void)param;
  return (ERROR);
}
}
