.. _user_installation:

############
Installation
############

Merethis recommends using its official packages from the Centreon
Entreprise Server (CES) repository. Most of Merethis' endorsed
software are available as RPM packages.

Alternatively, you can build and install your own version of this
software by following the :ref:`user_installation_using_sources`.

**************
Using packages
**************

Merethis provides RPM for its products through Centreon Entreprise
Server (CES). Open source products are freely available from our
repository.

These packages have been successfully tested with CentOS 5 and RedHat 5.

.. _user_installation_packages_prerequisites:

Prerequisites
=============

In order to use RPM from the CES repository, you have to install the
appropriate repo file. Run the following command as privileged user ::

  $ wget http://yum.centreon.com/standard/2.2/ces-standard.repo -O /etc/yum.repos.d/ces-standard.repo

The repo file is now installed. Don't forget to cleanup ::

  $ yum clean all

Install
=======

Run the following commands as privileged user ::

  $ yum install centreon-engine

All dependencies are automatically installed from Merethis repositories.

.. _user_installation_using_sources:

*************
Using sources
*************

To build Centreon Engine, you will need the following external
dependencies:

  * a C++ compilation environment.
  * CMake **(>= 2.8)**, a cross-platform build system.
  * Centreon Clib, The centreon Core library.

To build the optional web service module, you will need the following
external dependencies:

  * gSOAP **(>= 2.7)** toolkit for SOAP Web Services and XML-Based
    Applications.
  * zlib used for data compression.
  * SSL toolkit implementing SSL v2/v3 and TLS protocols with
    full-strength cryptography world-wide.
  * xerces-c toolkit to parse XML configuration files.


This program is compatible only with Unix-like platforms (Linux,
FreeBSD, Solaris, ...).

Prerequisites
=============

If you decide to build Centreon Engine from sources, we heavily
recommand that you create dedicated system user and group for
security purposes.

On all systems the commands to create a user and a group both named
**centreon-engine** are as follow (need to run these as root) ::

  $ groupadd centreon-engine
  $ useradd -g centreon-engine -m -r -d /var/lib/centreon-engine centreon-engine

Please note that these user and group will be used in the next steps. If
you decide to change user and/or group name here, please do so in
further steps too.

CentOS
------

In CentOS you need to add manually cmake. After that you can
install binary packages. Either use the Package Manager or the
yum tool to install them. You should check packages version when
necessary.

Package required to build:

=========================== =================== ================================
Software                    Package Name        Description
=========================== =================== ================================
C++ compilation environment gcc gcc-c++ make    Mandatory tools to compile.
CMake **(>= 2.8)**          cmake               Read the build script and
                                                prepare sources for compilation.
Centreon Clib               centreon-clib-devel Core library used by Centreon
=========================== =================== ================================

Optional packages was need for use web service module:

=========================== =================== ================================
Software                    Package Name        Description
=========================== =================== ================================
gSOAP **(>= 2.7)**          gsoap gsoap-devel   SOAP Web Services and XML-Based
                                                Applications.
Zlib                        zlib-devel          Used for data compression.
SSL                         openssl-devel       Implementing SSL and TLS
                                                protocols.
xerces-c                    xerces-c-devel      Used for parsing XML
                                                configuration files.
=========================== =================== ================================

#. Install basic compilation tools ::

   $ yum install gcc gcc-c++ make

#. Install Merethis repository

   You need to install Centreon Entreprise Server (CES) repos file as
   explained :ref:`user_installation_packages_prerequisites` to use some
   specific package version.

#. Install cmake ::

   $ yum install cmake

#. Install Centreon Clib

   See the Centreon Clib :ref:`documentation <centreon-clib:centreon_clib_install>`.

#. Install optional tools ::

   $ yum install zlib-devel openssl-devel xerces-c-devel

#. Install gSOAP ::

   $ yum install gsoap gsoap-devel

Debian/Ubuntu
-------------

In recent Debian/Ubuntu versions, necessary software is available as
binary packages from distribution repositories. Either use the Package
Manager or the apt-get tool to install them. You should check packages
version when necessary.

Package required to build:

=========================== ================= ================================
Software                    Package Name      Description
=========================== ================= ================================
C++ compilation environment build-essential   Mandatory tools to compile.
CMake **(>= 2.8)**          cmake             Read the build script and
                                              prepare sources for compilation.
Centreon Clib               centreon-clib-dev Core library used by Centreon
                                              Connector.
=========================== ================= ================================

Optional packages was need for use web service module:

=========================== =================== ================================
Software                    Package Name        Description
=========================== =================== ================================
gSOAP **(>= 2.7)**          gsoap               SOAP Web Services and XML-Based
                                                Applications.
Zlib                        zlib1g-dev          Used for data compression.
SSL                         libssl-dev          Implementing SSL and TLS
                                                protocols.
xerces-c                    libxerces-c-dev     Used for parsing XML
                                                configuration files.
=========================== =================== ================================

#. Install compilation tools ::

     $ apt-get install build-essential cmake

#. Install Centreon Clib

   See the Centreon Clib :ref:`documentation <centreon-clib:centreon_clib_install>`.

#. Install optional tools ::

     $ apt-get install gsoap zlib1g-dev libssl-dev libxerces-c-dev

OpenSUSE
--------

In recent OpenSUSE versions, necessary software is available as binary
packages from OpenSUSE repositories. Either use the Package Manager or
the zypper tool to install them. You should check packages version
when necessary.

Package required to build:

=========================== =================== ================================
Software                    Package Name        Description
=========================== =================== ================================
C++ compilation environment gcc gcc-c++ make    Mandatory tools to compile.
CMake **(>= 2.8)**          cmake               Read the build script and
                                                prepare sources for compilation.
Centreon Clib               centreon-clib-devel Core library used by Centreon
                                                Connector.
=========================== =================== ================================

Optional packages was need for use web service module:

=========================== =================== ================================
Software                    Package Name        Description
=========================== =================== ================================
gSOAP **(>= 2.7)**          not available.      SOAP Web Services and XML-Based
                                                Applications.
Zlib                        zlib-devel          Used for data compression.
SSL                         libopenssl-devel    Implementing SSL and TLS
                                                protocols.
xerces-c                    libxerces-c-devel   Used for parsing XML
                                                configuration files.
=========================== =================== ================================

#. Install compilation tools ::

     $ zypper install gcc gcc-c++ make cmake

#. Install Centreon Clib

   See the Centreon Clib :ref:`documentation <centreon-clib:centreon_clib_install>`.

#. Install optional tools ::

     $ zypper install zlib1g-dev libssl-dev libxerces-c-dev

#. Install gSOAP from merethis server ::

     $ ARCH=`uname -m`
     $ wget http://download.centreon.com/RPMs/opensuse/${ARCH}/libgsoap-2.8.4-1.1.${ARCH}.rpm
     $ wget http://download.centreon.com/RPMs/opensuse/${ARCH}/libgsoap-devel-2.8.4-1.1.${ARCH}.rpm
     $ wget http://download.centreon.com/RPMs/opensuse/${ARCH}/gsoap-devel-2.8.4-1.1.${ARCH}.rpm
     $ rpm -Uvh libgsoap-2.8.4-1.1.${ARCH}.rpm libgsoap-devel-2.8.4-1.1.${ARCH}.rpm gsoap-devel-2.8.4-1.1.${ARCH}.rpm

.. _user_installation_using_sources_build:

Build
=====

Get sources
-----------

Centreon Engine can be checked out from Merethis's git server at
http://git.centreon.com/centreon-engine. On a Linux box with git
installed this is just a matter of ::

  $ git clone http://git.centreon.com/centreon-engine

Or You can get the latest Centreon Engine's sources from its
`download website <http://www.centreon.com/Content-Download/download-centreon-engine-centreon>`_
Once downloaded, extract it ::

  $ tar xzf centreon-engine.tar.gz

Configuration
-------------

At the root of the project directory you'll find a build directory
which holds build scripts. Generate the Makefile by running the
following command ::

  $ cd /path_to_centreon_engine/build

Your Centreon Engine can be tweaked to your particular needs
using CMake's variable system. Variables can be set like this ::

  $ cmake -D<variable1>=<value1> [-D<variable2>=<value2>] .

Here's the list of variables available and their description:

============================== ================================================ ============================================
Variable                       Description                                      Default value
============================== ================================================ ============================================
WITH_CENTREON_CLIB_INCLUDE_DIR Set the directory path of centreon-clib include. auto detection
WITH_CENTREON_CLIB_LIBRARIES   Set the centreon-clib library to use.            auto detection
WITH_CENTREON_CLIB_LIBRARY_DIR Set the centreon-clib library directory (don't   auto detection
                               use it if you use WITH_CENTREON_CLIB_LIBRARIES).
WITH_GROUP                     Set the group for Centreon Engine installation.  root
WITH_LOCK_FILE                 Used by the startup script.                      ``/var/lock/subsys/centengine.lock``
WITH_LOG_ARCHIVE_DIR           Use to archive log files that have been rotated. ``${WITH_VAR_DIR}/archives``
WITH_LOGROTATE_DIR             Use to install logrotate files.                  ``/etc/logrorate.d/``
WITH_LOGROTATE_SCRIPT          Enable or disable install logrotate files.       OFF
WITH_PID_FILE                  This file contains the process id (PID) number   ``/var/run/centengine.pid``
                               of the running Centreon Engine process.
WITH_PKGCONFIG_DIR             Use to install pkg-config files.                 ``${WITH_PREFIX_LIB}/pkgconfig``
WITH_PKGCONFIG_SCRIPT          Enable or disable install pkg-config files.      ON
WITH_PREFIX                    Base directory for Centreon Engine installation. ``/usr/local``
                               If other prefixes are expressed as relative
                               paths, they are relative to this path.
WITH_PREFIX_BIN                Define specific directory for Centreon Engine    ``${WITH_PREFIX}/bin``
                               binary.
WITH_PREFIX_CONF               Define specific directory for Centreon Engine    ``${WITH_PREFIX}/etc``
                               configuration.
WITH_PREFIX_INC                Define specific directory for Centreon Engine    ``${WITH_PREFIX}/include/centreon-engine``
                               headers.
WITH_PREFIX_LIB                Define specific directory for Centreon Engine    ``${WITH_PREFIX}/lib/centreon-engine``
                               modules.
WITH_RW_DIR                    Use for files to need read/write access.         ``${WITH_VAR_DIR}/rw``
WITH_SAMPLE_CONFIG             Install sample configuration files.              ON
WITH_SSL                       Enable or disable SSL support in web service.    OFF
WITH_STARTUP_DIR               Define the startup directory.                    Generaly in ``/etc/init.d`` or ``/etc/init``
WITH_STARTUP_SCRIPT            Generate and install startup script.             auto
WITH_TESTING                   Build unit test.                                 OFF
WITH_USER                      Set the user for Centreon Engine installation.   root
WITH_VAR_DIR                   Define specific directory for temporary Centreon ``${WITH_PREFIX}/var``
                               Engine files.
WITH_WEBSERVICE                Enable or disable web service option.            OFF
WITH_XERCESC_INCLUDE_DIR       Set the directory path of xerces-c include.      auto detection
WITH_XERCESC_LIBRARIES         Set the xerces-c library to use.                 auto detection
WITH_XERCESC_LIBRARY_DIR       Set the xercess-c library directory (don't use   auto detection
                               it if you use WITH_XERCESC_LIBRARIES).
WITH_ZLIB                      Enable or disable compression in web service.    ON
============================== ================================================ ============================================

Example ::

  $ cmake \
     -DWITH_PREFIX=/usr \
     -DWITH_PREFIX_BIN=/usr/sbin \
     -DWITH_PREFIX_CONF=/etc/centreon-engine \
     -DWITH_PREFIX_LIB=/usr/lib/centreon-engine \
     -DWITH_USER=centreon-engine \
     -DWITH_GROUP=centreon-engine \
     -DWITH_LOGROTATE_SCRIPT=1 \
     -DWITH_VAR_DIR=/var/log/centreon-engine \
     -DWITH_RW_DIR=/var/lib/centreon-engine/rw \
     -DWITH_STARTUP_DIR=/etc/init.d \
     -DWITH_PKGCONFIG_SCRIPT=1 \
     -DWITH_PKGCONFIG_DIR=/usr/lib/pkgconfig \
     -DWITH_TESTING=0 \
     -DWITH_WEBSERVICE=0 .

At this step, the software will check for existence and usability of the
rerequisites. If one cannot be found, an appropriate error message will
be printed. Otherwise an installation summary will be printed.

.. note::
  If you need to change the options you used to compile your software,
  you might want to remove the *CMakeCache.txt* file that is in the
  *build* directory. This will remove cache entries that might have been
  computed during the last configuration step.

Compilation
-----------

Once properly configured, the compilation process is really simple ::

  $ make

And wait until compilation completes.

Install
=======

Once compiled, the following command must be run as privileged user to
finish installation ::

  $ make install

And wait for its completion.

Check-Up
========

After a successful installation, you should check for the existence of
some of the following files.

================================================ =========================================
File                                             Description
================================================ =========================================
``${WITH_PREFIX_BIN}/centengine``                Centreon Engine daemon.
``${WITH_PREFIX_BIN}/centenginestats``           Centreon Engine statistic.
``${WITH_PREFIX_BIN}/centenginews``              Centreon Engine Web Service command line.
``${WITH_PREFIX_CONF}/``                         Centreon Engine sample configuration.
``${WITH_PREFIX_LIB}/externalcmd.so``            External commands module.
``${WITH_PREFIX_LIB}/webservice.so``             Webservice module.
``${WITH_STARTUP_DIR}/centengine.conf``          Startup script for ubuntu.
``${WITH_STARTUP_DIR}/centengine``               Startup script for other os.
``${WITH_PREFIX_INC}/include/centreon-engine/``  All devel Centreon Engine's include.
``${WITH_PKGCONFIG_DIR}/centengine.pc``          Centreon Engine pkg-config file.
================================================ =========================================
