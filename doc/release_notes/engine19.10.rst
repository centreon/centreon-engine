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
