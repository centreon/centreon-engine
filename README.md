# Centreon Engine #

Centreon Engine is a fast and powerful open-source monitoring scheduler.
It is a low-level component of the [Centreon software suite](https://www.centreon.com).

Centreon Engine is released under the General Public License version 2
and is endorsed by the [Centreon company](https://www.centreon.com).

This project was started as a fork of Nagios, the well known open-source
monitoring application. While keeping its configuration file format and
its stability we improved it in several ways :

  - reduced startup time
  - faster standard check execution engine
  - new light check execution system (connectors)
  - on-the-fly configuration reload
  - less obscure configuration options
  - frequent bugfix releases

Just give it a try !


## Documentation ##

The full Centreon Engine documentation is available online
[here](http://documentation.centreon.com/docs/centreon-engine/en/). It
is generated from [reStructuredText](http://docutils.sourceforge.net/rst.html) files located in the *./doc/* directory of
Centreon Engine sources.

The documentation extensively covers all aspects of Centreon Engine such
as installation, compilation, configuration, use and more. It is the
reference guide of the software. This *README* is only provided as a
quick introduction.


## Installing from binaries ##

**Warning**: Centreon Engine is a low-level component of the Centreon
software suite. If this is your first installation you would probably
want to [install it entirely](https://documentation.centreon.com/docs/centreon/en/2.6.x/installation/index.html).

Centreon ([the company behind the Centreon software suite](http://www.centreon.com))
provides binary packages for RedHat / CentOS. They are available either
as part of the [Centreon Entreprise Server distribution](https://www.centreon.com/en/products/centreon-enterprise-server/)
or as individual packages on [our RPM repository](https://documentation.centreon.com/docs/centreon/en/2.6.x/installation/from_packages.html).

Once the repository installed a simple command will be needed to install
Centreon Engine.

    $> yum install centreon-engine


## Fetching sources ##

The reference repository is hosted at [GitHub](https://github.com/centreon/centreon-engine).
Beware that the repository hosts in-developement sources and that it
might not work at all.

Stable releases are available as gziped tarballs on [Centreon's download site](https://download.centreon.com).


## Compilation (quickstart) ##

**Warning**: Centreon Engine is a low-level component of the Centreon
software suite. If this is your first installation you would probably
want to [install it entirely](https://documentation.centreon.com/docs/centreon/en/2.6.x/installation/index.html).

This paragraph is only a quickstart guide for the compilation of
Centreon Engine. For a more in-depth guide with build options you should
refer to the [online documentation](https://documentation.centreon.com/docs/centreon-engine/en/latest/installation/index.html#using-sources).

Centreon-Clib should be installed before installing Centreon-Engine.
If you do not have Centreon-Clib you can follow the README for a quick introduction at : [GitHub](https://github.com/centreon/centreon-clib)

**For Centos 7 :**
First of all, check if you have these packages installed :

    git, make, gcc-c++, python3, epel-release, cmake3, centreon-clib-devel

If they are not installed, please install them. Make sure to install epel-release before cmake3, if you install them both together it will not find cmake3.
    $> yum install git make cmake gcc-c++ python3 epel-release centreon-clib-devel
    $> yum install cmake3

Conan is needed as well but make sure to install gcc before conan.
When you are ready do :  
    $> pip3 install conan  

Then you will be able to resume the following steps :
    $> git clone https://github.com/centreon/centreon-engine
    $> cd centreon-engine/build
    $> conan install .. --build missing
    $> cd .. && ./cmake.sh

To speed up the compilation you can use make -j4 if you have 4 cpu or make -j8 if you have 8 cpu...
    $> make && make install
   Or:
    $> make -j4 && make install

**For other distributions :**
If you are on another distribution, then follow the steps below.

Check if you have these packages installed (Note that packages names come from Centos 7 distribution, so if some packages names don't match on your distribution try to find their equivalent names) :

    git, make, cmake, gcc-c++, python3, epel-release, centreon-clib-devel

For the projet compilation you need to have conan installed. Try to use the package manager given by your OS to install conan. ('apt' for Debian, 'rpm' for Red Hat, 'pacman' for Arch Linux, ...) It is prefered to install gcc before conan.

Exemple :
    $> _apt_ install conan

If it does not work, conan can be installed with pip3 :
    $> pip3 install conan

You can now compile centreon-engine :
    $> git clone https://github.com/centreon/centreon-engine
    $> mkdir -p centreon-engine/build
    $> cd centreon-engine/build
    $> conan install .. --build missing

Once the sources of Centreon Engine extracted go to the *./build/* directory and launch the CMake command.
This will look for required dependencies and print a summary of the compilation parameters if everything went fine.

   $> cmake -DWITH_PREFIX=/usr -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=Release -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine ..

Now launch the compilation using the *make* command and then install the software by running *make install* as priviledged user.

    $> make && make install


You're done !

Verify /etc/centreon-engine directory.

Usually, its content is filled by the Centreon web interface, owned by the apache user.

A command like the following may be necessary:

    $> chown -R apache:apache /etc/centreon-engine/*


## Bug reports / Feature requests ##

The best way to report a bug or to request a feature is to open an issue
in GitHub's [issue tracker](https://github.com/centreon/centreon-engine/issues/).

Please note that Centreon Engine follows the
[same workflow as Centreon](https://github.com/centreon/centreon/blob/master/project/issues.md)
to process issues.

For a quick resolution of a bug your message should contain :

* the problem description
* precise steps on how to reproduce the issue (if you're using Centreon
  web UI tell us where you click)
* the expected behavior
* the Centreon product**s** version**s**
* the operating system you're using (name and version)
* if possible configuration, log and debug files


## Contributing ##

Contributions are much welcome ! If possible provide them as
pull-requests on GitHub. If not, patches will do but describe against
which vesion/commit they apply.

For any question or remark feel free to send a mail to the project
maintainer : Matthieu Kermagoret (mkermagoret@centreon.com).
