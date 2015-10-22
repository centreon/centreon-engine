===================
Centreon Engine 1.5
===================

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
