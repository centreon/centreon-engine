/*
** Copyright 2011      Merethis
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

#ifndef CCE_COMPATIBILITY_CONFIG_H
# define CCE_COMPATIBILITY_CONFIG_H

/***** MISC DEFINITIONS *****/

# ifndef USE_EVENT_BROKER
#  define USE_EVENT_BROKER
# endif /* !USE_EVENT_BROKER */

# ifndef USE_XSDDEFAULT
#  define USE_XSDDEFAULT
# endif /* !USE_XSDDEFAULT */

# ifndef USE_XCDDEFAULT
#  define USE_XCDDEFAULT
# endif /* !USE_XCDDEFAULT */

# ifndef USE_XRDDEFAULT
#  define USE_XRDDEFAULT
# endif /* !USE_XRDDEFAULT */

# ifndef USE_XODTEMPLATE
#  define USE_XODTEMPLATE
# endif /* !USE_XODTEMPLATE */

# ifndef USE_XPDDEFAULT
#  define USE_XPDDEFAULT
# endif /* !USE_XPDDEFAULT */

# ifndef USE_XDDDEFAULT
#  define USE_XDDDEFAULT
# endif /* !USE_XDDDEFAULT */

# ifndef USE_NANOSLEEP
#  define USE_NANOSLEEP
# endif /* !USE_NANOSLEEP */

/***** HEADER FILES *****/

# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <time.h>
# include <sys/resource.h>
# include <limits.h>
# include <pwd.h>
# include <grp.h>
# include <strings.h>
# include <string.h>
# include <unistd.h>
# include <signal.h>
# include <sys/stat.h>
# include <sys/mman.h>
# include <fcntl.h>
# include <stdarg.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <errno.h>
# include <sys/timeb.h>
# include <sys/ipc.h>
# include <sys/msg.h>
# include <math.h>
# include <ctype.h>
# include <dirent.h>
# include <pthread.h>
# include <regex.h>
# include <sys/socket.h>
# include <syslog.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <libgen.h>
# include <sys/un.h>
# include <poll.h>
# include <dlfcn.h>

#endif /* !CCE_COMPATIBILITY_CONFIG_H */
