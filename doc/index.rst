Welcome to Centreon Engine's documentation!
===========================================

Centreon Engine is an Open Source system and network monitoring
application. It watches hosts and services that you specify, alerting
you when things go bad and when they get better.

Centreon Engine was originally designed to run under Linux, although it
should work under most other unices as well.

Some of the many features of Centreon Engine include:

  * monitoring of network services (SMTP, IMAP, HTTP, NNTP, PING ...).
  * monitoring of host resources (processor load, disk usage ...).
  * simple plugin design that allows users to easily develop their own
    service checks.
  * parallelized service checks.
  * ability to define network host hierarchy using "parent" hosts,
    allowing detection of and distinction between hosts that are down and
    those that are unreachable.
  * contact notifications when service or host problems occur and get
    resolved (via email, page, or user-defined method).
  * ability to define event handlers to be run during service or host
    events for proactive problem resolution.
  * automatic log file rotation.
  * support for implementing redundant monitoring hosts.

System Requirements
===================

The only requirement of running Centreon Engine is a machine running
Linux (or Unix variant) that has network access and a C++ compiler
installed (if installing from source code).

Licensing
=========

Centreon Engine is licensed under the terms of the `GNU General Public
License <http://www.gnu.org/licenses/old-licenses/gpl-2.0>`_ Version 2
as published by the `Free Software Foundation <http://www.fsf.org/>`_.
This gives you legal permission to copy, distribute and/or modify
Centreon Engine under certain conditions. Read the ``license.txt`` file in
the Centreon Engine distribution or read the `online version of the
license <http://www.gnu.org/licenses/old-licenses/gpl-2.0>`_ for more
details.

Centreon Engine has started as a fork of the Nagios Core project. Nagios
and the Nagios logo are trademarks, servicemarks, registered trademarks
or registered servicemarks owned by Nagios Enterprises, LLC. Centreon,
the Centreon Logo, Merethis and the Merethis logo are trademarks,
servicemarks, registered trademarks or registered servicemarks owned by
Merethis. All other trademarks, servicemarks, registered trademarks, and
registered servicemarks are the property of their respective owner(s).

All source code, binaries, documentation, information, and other files
contained in this distribution are provided AS IS with NO WARRANTY OF
ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY, AND FITNESS
FOR A PARTICULAR PURPOSE.

Several people have contributed to Centreon Engine or Nagios Core by
either reporting bugs, suggesting improvements, writing plugins, etc. A
list of some of the many contributors to the development of Centreon
Engine or Nagios Core can be found in the thanks.txt file in the root of
the Centreon Engine distribution.

Contents:

.. toctree::

   installation/index
   user/index
   exploit/index
