# Centreon Engine #

Centreon Engine is a fast and powerful open-source monitoring scheduler.
It is a low-level component of the [Centreon software suite](https://www.centreon.com).

Centreon Engine is released under the General Public License version 2
and is endorsed by the [Centreon company](https://www.centreon.com).

## Documentation ##

The full Centreon Engine documentation is available online
[here](http://documentation.centreon.com/docs/centreon-engine/en/). It
is generated from ReST files located in the ./doc/ directory of Centreon
Engine sources.

The documentation extensively covers all aspects of Centreon Engine such
as installation, compilation, configuration, use and more. It is the
reference guide the software. This README is only provided as a quick
introduction.

## Installing from binaries ##

**Warning**: Centreon Engine is a low-level component of the Centreon
software suite. If this is your first installation you would probably
want to [install it entirely](https://documentation.centreon.com/docs/centreon/en/2.6.x/installation/index.html).

Centreon ([the company behind the Centreon software suite](http://www.centreon.com])
provides binary packages for RedHat / CentOS. They are available either
as part of the [Centreon Entreprise Server distribution](https://www.centreon.com/en/products/centreon-enterprise-server/)
or as individual packages on [our RPM repository](https://documentation.centreon.com/docs/centreon/en/2.6.x/installation/from_packages.html).

Once the repository installed a simple command will be needed to install
Centreon Engine.

  $# yum install centreon-engine

## Fetching the sources ##

The reference repository is hosted at [GitHub](https://github.com/centreon/centreon-engine).
Beware that the repository hosts in-developement sources and that it
might not work at all.

Stable releases are available as gziped tarballs on [Centreon's download site](https://download.centreon.com/).

## Compilation (quickstart) ##

**Warning**: Centreon Engine is a low-level component of the Centreon
software suite. If this is your first installation you would probably
want to [install it entirely](https://documentation.centreon.com/docs/centreon/en/2.6.x/installation/index.html).

This paragraph is only a quickstart guide for the compilation of
Centreon Engine. For a more in-depth guide with build options you should
refer to the [online documentation](https://documentation.centreon.com/docs/centreon-engine/en/latest/installation/index.html#using-sources).

Centreon Engine needs Centreon Clib to be build. You should
[install it first](https://github.com/centreon/centreon-clib).

Once the sources of Centreon Engine extracted go to the *./build/*
directory and launch the CMake command. This will look for required
dependencies and print a summary of the compilation parameters if
everything went fine.

  $> cd centreon-broker/build
  $> cmake .
  ...

Now launch the compilation using the *make* command and then install it
by running *make install* as priviledged user.

  $> make -j 4
  ...
  $# make install

You're done !
