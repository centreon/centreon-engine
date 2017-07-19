=====================
Centreon Engine 1.8.0
=====================

*********
Bug fixes
*********

Make timezone macros work
=========================

The $HOSTTIMEZONE$, $SERVICETIMEZONE$ and $CONTACTTIMEZONE$ macros were
not included in the generated macro array. As the array size was
increased, these macros are now working.

Print initial states when creating new objects at reload
========================================================

Initial host and service states are not printed when new objects are
created during a configuration reload. This prevented Centreon Web's
reporting dashboard to work properly on such hosts and services until
a process restart occured.
