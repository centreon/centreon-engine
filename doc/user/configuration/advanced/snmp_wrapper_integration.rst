.. _snmp_wrapper_integration:

SNMP Wrapper Integration
************************

Introduction
============

.. note::
   Centreon Engine is not designed to be a replacement for a full-blown
   SNMP management application like HP OpenView or `OpenNMS
   <http://www.opennms.org/>`_. However, you can set things up so that
   SNMP traps received by a host on your network can generate alerts in
   Centreon Engine.

As if designed to make the Gods of Hypocrisy die of laughter, SNMP is
anything but simple. Translating SNMP traps and getting them into
Centreon Engine (as passive check results) can be a bit tedious. To make
this task easier, I suggest you check out Alex Burger's SNMP Trap
Translator project located at http://www.snmptt.org. When combined with
Net-SNMP, SNMPTT provides an enhanced trap handling system that can be
integrated with Centreon Engine.

Yep, that's all.

