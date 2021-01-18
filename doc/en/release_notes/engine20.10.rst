=======================
Centreon Engine 20.10.3
=======================

*******
Bug Fix
*******

Flapping detection
==================
The code is updated to match the documentation. Following the documentation,
the hightest threshold on flapping detection is a non strict threshold whereas
the lowest threshold is a strict one. For the second one, we had to change the
code to be agress with the documentation.

process/process_manager api updated
===================================
Because of several changes in the clib api, we have to change several calls
here.

=======================
Centreon Engine 20.10.2
=======================

***********
Enhancement
***********

Instance updates
================
There is a minimal delay specified in seconds between two instance updates.
By default, its value is 30s. It can be set with the variable
instance_heartbeat_interval in the centengine.cfg file.

*******
Bug Fix
*******

Macros
======
fix some macro and test for some macro.
fix macro $TIMET$ now return time in a unix epoch format.

Notification
==============
fix Notifications are not sent when entering in notification timeperiod with notification_interval setted to 0.

=======================
Centreon Engine 20.10.1
=======================

*******
Bug Fix
*******

Macros filters
==============
Macros can be filtered. This was possible before and there was a regression
breaking this functionality. So now, we can activate the macros filtering and
then we can specify which macros to send to broker.

Stalking option
================
Stalking option works again.

last service notification
================
Last Service Notification set when state is HARD without notification. This new
version fixes this point.

Build
=====

Centos8 fixed.
