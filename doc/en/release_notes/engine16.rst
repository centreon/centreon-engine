=====================
Centreon Engine 1.6.2
=====================

*********
Bug fixes
*********

Crash when reloading escalation configuration
=============================================

In some cases, the configuration reload mechanism can lead Centreon
Engine to crash. This was caused by invalid links in the escalation
objects that were accessed before relinking occured.

=====================
Centreon Engine 1.6.1
=====================

**********
What's new
**********

Recovery notification delay
===========================

A new configuration variable is introduced on hosts and services to
delay recovery notification. The recovery notification delay property
defines a duration below which no recovery notification will be sent for
the affected host or service. This helps to reduce the number of
transient issues.

Timezone macros
===============

Objects' timezones are now available through the HOSTTIMEZONE,
SERVICETIMEZONE and CONTACTTIMEZONE macros for hosts, services and
contacts respectively.

Code cleanup
============

As Centreon Engine became performant enough, all caching files inherited
from the original software became unnecessary. Support code for these
cache files was removed in this release.

*********
Bug fixes
*********

Additive inheritance
====================

Additive inheritance was not always working according to documentation.
This release make the documentation and the real software behavior
match.
