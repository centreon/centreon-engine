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

# include <arpa/inet.h>
# include <ctype.h>
# include <dirent.h>
# include <dlfcn.h>
# include <errno.h>
# include <fcntl.h>
# include <grp.h>
# include <libgen.h>
# include <limits.h>
# include <math.h>
# include <netdb.h>
# include <netinet/in.h>
# include <poll.h>
# include <pthread.h>
# include <pwd.h>
# include <regex.h>
# include <signal.h>
# include <stdarg.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <syslog.h>
# include <sys/mman.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <time.h>
# include <unistd.h>

#endif /* !CCE_COMPATIBILITY_CONFIG_H */
