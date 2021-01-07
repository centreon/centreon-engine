=======================
Centreon Engine 21.04.0
=======================

************
New features
************

External commands
=================

Engine hosts a gRPC server now. All the external commands can be executed
through this new server now.

Flapping
========

The internal flapping object does not reference notifiers by host name and
service description but by host id and service id.
