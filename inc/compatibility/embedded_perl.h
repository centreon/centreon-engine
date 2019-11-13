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

#ifndef CCE_COMPATIBILITY_EMBEDDED_PERL_H
#define CCE_COMPATIBILITY_EMBEDDED_PERL_H

#ifdef __cplusplus
extern "C" {
#endif  // C++

int deinit_embedded_perl();
int init_embedded_perl(void const* param);
int file_uses_embedded_perl(void const* param);

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_COMPATIBILITY_EMBEDDED_PERL_H
