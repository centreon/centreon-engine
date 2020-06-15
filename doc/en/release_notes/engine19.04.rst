=====================
Centreon Engine 19.04
=====================

Change version number according to new
`Centreon Lifecycle Products Policy <https://documentation.centreon.com/docs/centreon/en/latest/life_cycle.html>`_

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
