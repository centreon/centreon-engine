=======================
Centreon Engine 20.04.5
=======================

************
Bug Fix
************

Unicode check was buggy
=======================

The code that validates the UTF-8 strings was buggy and could keep as is some
characters that were not UTF-8. It is fixed and moved to the cbmod module.

=======================
Centreon Engine 20.04.4
=======================

************
Bug Fix
************

Macros on services do not work
==============================

A macro working on a service did not work, and most of the time crashed.
This is fixed with this new version.

=======================
Centreon Engine 20.04.3
=======================

************
Bug Fix
************

Windows checks can be CP1252 encoded
====================================

To write into the database such strings is impossible unless we convert the
string to utf-8. This is what is done in this new Engine version. Each time
a check is done, we verify its output is in UTF-8 format, if it is not the
case, it is converted. Supported input encodings are ISO-8859-15, CP-1252 and
UTF-8.

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
