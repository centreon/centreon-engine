=======================
Centreon Engine 20.04.2
=======================

************
Bug Fix
************

If a host is disabled, it should also be the case for its services
==================================================================

If a host with several services is disabled. Its services are removed from
the monitoring. But a query in centreon_storage shows that those services
are still there. With this new version, it is fixed.

=======================
Centreon Engine 20.04.01
=======================

************
Bug Fix
************

debug_lvl=-1
============

Engine was stuck when we put -1 as debug_lvl
in centengine.cfg.


=======================
Centreon Engine 20.04
=======================

************
New features
************

Support for POLLERNAME macro
=============================

You can now use $POLLERNAME$ macro to retrieve the name of your poller in
a check_command. It will use the poller_name field of your config.

Support for POLLERID macro
=============================

You can now use $POLLERID$ macro to retrieve the name of your poller in
a check_command. It will use the poller_id field of your config.


*********
Bug fixes
*********

Notifications between two fixed contiguous downtimes
====================================================

It was possible to have notifications sent between the two downtimes even if
the space duration is 0.

Macros replacements
===================

Host macros and several global macros containing numbers were badly replaced.
It is fixed now.
