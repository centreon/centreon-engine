=======================
Centreon Engine 19.04.3
=======================

*********
Bug fixes
*********

Centreon connectors timeout
===========================

The timeout applied on checks execute by connectors were shorter than the
timeout passed as parameter (near of 1s of difference). To fix this, timeouts
are checked as milliseconds values now (more accurate) and the good duration is
considered by connectors.

=======================
Centreon Engine 19.04.2
=======================

*********
Bug fixes
*********

Fixed downtimes and notifications
=================================

A device in downtime on contiguous fixed downtimes should not notify, even
between those two downtimes.

=======================
Centreon Engine 19.04.1
=======================

*********
Bug fixes
*********

Hosts and contacts custom macros are not inserted into centreon_storage db
==========================================================================

The custom macros added to the hosts and contacts objects are not inserted
into centreon_storage database. That's why it is not possible to filter by
criticity in "Monitoring > Status Details > Hosts" page.

=====================
Centreon Engine 19.04
=====================

Change version number according to new
`Centreon Lifecycle Products Policy <https://documentation.centreon.com/docs/centreon/en/latest/life_cycle.html>`_
