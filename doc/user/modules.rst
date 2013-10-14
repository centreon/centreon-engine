#######
Modules
#######

This is a way of extending functionality in Centreon Engine without
having to alter main code. Modules are based on shared code libraries.
Modules are hooked into the Centreon Engine process. Modules uses
callback routines which have to be served by the modules. These routines
are executed when special events occur in the Centreon Engine server
process.

External Command
================

The external commands system was moved on a module. To use it, you need
to add line on Centreon Engine configuration::

    broker_module=/usr/lib/centreon-engine/externalcmd.so

Web Service
===========

.. warning::
   This module is in testing and has absolutely no warranty. And the
   WSDL can change in the future.

Centreon Engine has a web service module to receive order form other
application. To use it, you need to add line on Centreon Engine
configuration::

    broker_module=/usr/lib/centreon-engine/webservice.so

Configuration
-------------

To configure this module you need to create an XML configuration file.

The list of available options for use within a webservice block are
defined in the table below.

============== ==============================================
Option         Description
============== ==============================================
host           Host to connect to Centreon Engine Webservice.
port           Port to connect to Centreon Engine Webservice.
recv_timeout   Timeout in second for receive data.
send_timeout   Timeout in second for send data.
accept_timeout Timeout in microsecond to accept connection.
ssl            Block to enable or disable SSL.
============== ==============================================

The list of available options for use within a ssl block are defined in
the table below.

========  ==============================================================
Option    Description
========  ==============================================================
enable    true to enable SSL or false to disable SSL.
keyfile   Required when server must authenticate to clients (see SSL
          docs on how to obtain this file).
certif    Optional cacert file to store trusted certificates.
dh        DH file name or DH key len bits (minimum is 512) to generate
          DH param, if NULL use RSA.
password  Password to read the key file (not used with GNUTLS).
========  ==============================================================

Example
-------

::

    <?xml version="1.0" encoding="UTF-8"?>
    <webservice>
      <host>127.0.0.1</host>
      <port>4042</port>
      <recv_timeout>5</recv_timeout>
      <send_timeout>5</send_timeout>
      <accept_timeout>500</accept_timeout>

      <ssl>
        <enable>false</enable>
        <keyfile>keyfile</keyfile>
        <cacert>certif</cacert>
        <dh>dhfile</dh>
        <password>password</password>
      </ssl>
    </webservice>

