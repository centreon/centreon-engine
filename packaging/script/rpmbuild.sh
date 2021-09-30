#!/bin/bash

show_help() {
cat << EOF
Usage: ${0##*/} -n=[yes|no] -v

This program build Centreon-engine

    -v  : major version
    -r  : release number
    -h  : help
EOF
exit 2
}

VERSION=$1
RELEASE=$2

if [ -z $VERSION ] || [ -z $RELEASE ] ; then
   echo "Some or all of the parameters are empty";
   echo $VERSION;
   echo $RELEASE;
   show_help
fi

ln -s /usr/bin/cmake3 /usr/bin/cmake

# dossier racine du nouveau centreon collect
if [ ! -d /root/rpmbuild/SOURCES ] ; then
    mkdir -p /root/rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
fi

cd "../../../"

mkdir centreon-engine-$VERSION
cp -r centreon-engine/* centreon-engine-$VERSION
tar -czf centreon-engine-$VERSION.tar.gz centreon-engine-$VERSION cmake.sh
mv centreon-engine-$VERSION.tar.gz /root/rpmbuild/SOURCES/
rm -rf centreon-engine-$VERSION

cp centreon-engine/packaging/rpm/centreonengine_integrate_centreon_engine2centreon.sh /root/rpmbuild/SOURCES/
rpmbuild -ba centreon-engine/packaging/rpm/centreon-engine.spectemplate -D "VERSION $VERSION" -D "RELEASE $RELEASE"

