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
