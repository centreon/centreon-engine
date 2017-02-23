=====================
Centreon Engine 1.7.1
=====================

*********
Bug fixes
*********

Reduce module reloading failure severity
========================================

In previous versions, Centreon Engine complained when not being able to
reload one of its module with an Error message. The severity was reduce
to what it really is, a Warning.

Set default auto-rescheduling interval to one hour
==================================================

When activated, auto-rescheduling places service checks in an even
manner. However by default the auto-rescheduling routine was run every
30 seconds, which could lead in the most extreme cases in the same
service being executed every 30 seconds. The default interval is now set
to one hour.

Remove notification warnings if notification is not enabled
===========================================================

Warnings about notification misconfiguration are now removed if
notification is not enabled on the target host or service.

=====================
Centreon Engine 1.7.0
=====================

**********
What's new
**********

Rewritten timeperiods
=====================

The timeperiod code was heavily modified to be more robust and handle
more edge cases such as DST changes or timeperiods that span between
two years. Unit tests ensure that these edge cases work properly.
