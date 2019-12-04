=======================
Centreon Engine 19.10.9
=======================

*********
Bug fixes
*********

Notifications on volatile services
==================================

On a volatile service, if notifications are disabled, it should not send
notifications.

=======================
Centreon Engine 19.10.8
=======================

*********
Bug fixes
*********

Notifications and contiguous fixed downtimes
============================================

Between two contiguous downtimes, it was possible to receive notifications
of devices that were in critical state whereas they had to be in downtime.

Second notification with state change
=====================================

When a notification has been sent for a warning state. If the device changes to
a critical state, no notification was sent. This is fixed in this version.

First notification delay if no recovery notification configured
===============================================================

When a first notification delay but no recovery notification are configured,
the first notification delay is not applied. This is fixed in this release.

Double recovery notification sent on passive services/hosts
===========================================================

When a service come back to OK and is configured to send recovery notifications.
It sends two of them instead of one.

=======================
Centreon Engine 19.10.7
=======================

*********
Bug fixes
*********

Groups update with deleted object
=================================

On object deletion, the groups containing these objects were not updated.
So on group update (conf reload, not restart) the group keeps in its
members a deleted object and can use it.


=======================
Centreon Engine 19.10.6
=======================

*********
Bug fixes
*********

Escalations
===========

When a host or a service linked to an escalation was deleted, there was an
attempt to remove the host or the service twice. This behaviour led to a
cancellation of the new configuration during a reload.

=======================
Centreon Engine 19.10.5
=======================

*********
Bug fixes
*********

Notifications retention
=======================

In case of engine restart, notifications already sent were lost. It may have
impacts on normal notifications when it is time to send a recovery notification.
After a restart, the recovery notification was never sent.

=======================
Centreon Engine 19.10.4
=======================

*********
Bug fixes
*********

Custom variable update crash on reload
======================================

A bug was introduce in 19.10.2 with a reload on custom_variables
services/contacts/hosts update.

Recovery notification didn't work on escalation [1;0]
=====================================================

If an escalation was defined with a first notification at 1 and a
last notification at 0, we want the escalation to start from the
first notification, but only one time, the recovery notification
was not sent.

=======================
Centreon Engine 19.10.3
=======================

*********
Bug fixes
*********

Service escalation not well resolved
====================================

Service configurations look up failed when service escalations were defined.

=======================
Centreon Engine 19.10.2
=======================

*********
Bug fixes
*********

Custom variables not resolved
==============================

Some custom variables were not resolved.
This results in bad or pending checks with wrong commands.

Random host statuses
====================

Some hosts/services were changing status randomly.

Send custom variables to broker
===============================

Some broker notifications about customvariables were dropped.

Null string crash
=================

In some cases strings construction could lead to exceptions thrown.

=======================
Centreon Engine 19.10.1
=======================

*********
Bug fixes
*********

External commands on hosts could crash
======================================

In case of an external command about a host but specified by its ip address
instead of its name, the external command finished with a segfault.

=====================
Centreon Engine 19.10
=====================

*********
Bug fixes
*********

Notifications
=============

The notifications mechanism has been totally rewritten. First notification delay
and last recovery notification delay should work as expected.

Pipes in results
================

Pipes are allowed in the centreon-engine plugin return value.

***********
Improvements
***********

Optimizations
=============

Migration of the code to C++11. This implies many changes in data structures.
Some parts have also been rewritten and optimized.

Cmake cleanup
=============

The build directory is gone away. CMake is used as intended, this solves issues
with some ide (like kdevelop)...

Better test coverage
====================

We now have 123 new unit test (+63%). It allow us to have a better code coverage
of the code base.
