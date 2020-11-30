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
