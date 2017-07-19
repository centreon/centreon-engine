=====================
Centreon Engine 1.7.2
=====================

************
Enhancements
************

Improve configuration loading time
==================================

In the configuration resolution phase, the object lookup mechanism was
mostly based on object iteration (ie. O(n)). This was replaced by
key-based lookup (either O(1) or O(ln n), based on searched object
type).

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
