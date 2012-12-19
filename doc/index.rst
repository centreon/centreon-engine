.. _top:

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

Contents:

.. toctree::
   :maxdepth: 2

   release_notes/index
   installation/index
   user/index
   exploit/index
   license
