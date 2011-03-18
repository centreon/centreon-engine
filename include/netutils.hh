/*
** Copyright 2008 Ethan Galstad
** Copyright 2011 Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef SCHEDULER_NETUTILS_HH
# define SCHEDULER_NETUTILS_HH

int my_tcp_connect(char *host_name, int port, int *sd, int timeout);
int my_sendall(int s, char *buf, int *len, int timeout);
int my_recvall(int s, char *buf, int *len, int timeout);

#endif // !SCHEDULER_NETUTILS_HH
