=====================
Centreon Engine 1.5.2
=====================

**********
Bugs fixed
**********

Crash when reloading escalation configuration
=============================================

In some cases, the configuration reload mechanism can lead Centreon
Engine to crash. This was caused by invalid links in the escalation
objects that were accessed before relinking occured.

=====================
Centreon Engine 1.5.1
=====================

**********
Bugs fixed
**********

Missing HOSTID and SERVICEID macros
===================================

When developing the 1.5.0 release of Centreon Engine, Centreon ID were
promoted to full-class configuration variables. This effectively removed
them from the list of available macros. This release restore them.

Wrong latency after first startup
=================================

The latency of newly created objects can be wrongly calculated in rare
situations where the smart scheduling mode is *not* used.

=====================
Centreon Engine 1.5.0
=====================

**********
What's new
**********

Timezone support
================

Hosts, services and contacts can now have a timezone. All time periods
applied to these objects will be computed regarding their timeperiod.

Webservice module removal
=========================

The webservice module has now been removed from Centreon Engine.

Direct Centreon ID support
==========================

Most important objects can now have their Centreon ID directly set
through a configuration directive. This change will reduce support code
needed in Centreon Broker and provide more performance in this piece of
software. However as a consequence of this change, Centreon Engine 1.5
requires at least Centreon Broker 2.11.

Failure prediction removal
==========================

The failure prediction configuration directives are now totally
deprecated. This system inherited from Nagios never worked as intended.
Only configuration fields were set in structures and this code was
removed in this release.
