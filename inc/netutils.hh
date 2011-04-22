/*
** Copyright          1999,2008 Ethan Galstad
** Portions Copyright 1999-2008 Nagios Plugin Development Team
** Copyright          2011      Merethis
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

#ifndef CCE_NETUTILS_HH
# define CCE_NETUTILS_HH

# ifdef __cplusplus
extern "C" {
# endif

int my_tcp_connect(char* host_name, int port, int* sd, int timeout);
int my_sendall(int s, char* buf, int* len, int timeout);
int my_recvall(int s, char* buf, int* len, int timeout);

# ifdef __cplusplus
}
# endif

#endif // !CCE_NETUTILS_HH
