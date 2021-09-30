#!/bin/bash
set -e

if [ -z "$VERSION" -o -z "$RELEASE" -o -z "$DISTRIB" ] ; then
  echo "You need to specify VERSION / RELEASE variables"
  exit 1
fi

# echo "################################################## install CLIB ##################################################"
if [ $DISTRIB = "el7" ] ; then
  curl http://yum-1.centreon.com/standard/21.10/el7/unstable/x86_64/clib/centreon-clib-21.10.0-1632908959.e89357c/centreon-clib-21.10.0-1632908959.e89357c.el7.centos.x86_64.rpm --output centreon-clib-21.10.0-1632908959.e89357c.el7.centos.x86_64.rpm
  curl http://yum-1.centreon.com/standard/21.10/el7/unstable/x86_64/clib/centreon-clib-21.10.0-1632908959.e89357c/centreon-clib-devel-21.10.0-1632908959.e89357c.el7.centos.x86_64.rpm --output centreon-clib-devel-21.10.0-1632908959.e89357c.el7.centos.x86_64.rpm
  yum -y install centreon-clib*.rpm
else
  curl http://yum-1.centreon.com/standard/21.10/el8/unstable/x86_64/clib/clib-21.10.0-1632908959.e89357c/centreon-clib-21.10.0-1632908959.e89357c.el8.x86_64.rpm --output centreon-clib-21.10.0-1632908959.e89357c.el8.x86_64.rpm
  curl http://yum-1.centreon.com/standard/21.10/el8/unstable/x86_64/clib/clib-21.10.0-1632908959.e89357c/centreon-clib-devel-21.10.0-1632908959.e89357c.el8.x86_64.rpm --output centreon-clib-devel-21.10.0-1632908959.e89357c.el8.x86_64.rpm
  yum -y install centreon-clib*.rpm
fi

# echo "################################################## BUILDING ENGINE ##################################################"

# generate rpm engine
if [ ! -d /root/rpmbuild/SOURCES ] ; then
    mkdir -p /root/rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
fi

mkdir centreon-engine-$VERSION
cp -r centreon-engine/* centreon-engine-$VERSION
tar -czf centreon-engine-$VERSION.tar.gz centreon-engine-$VERSION centreon-engine/cmake.sh
mv centreon-engine-$VERSION.tar.gz /root/rpmbuild/SOURCES/
rm -rf centreon-engine-$VERSION

cp centreon-engine/packaging/rpm/centreonengine_integrate_centreon_engine2centreon.sh /root/rpmbuild/SOURCES/
rpmbuild -ba centreon-engine/packaging/rpm/centreon-engine.spectemplate -D "VERSION $VERSION" -D "RELEASE $RELEASE"

# cleaning and according permissions to slave to delivery rpms
rm -rf *.rpm
cp -r /root/rpmbuild/RPMS/x86_64/*.rpm centreon-engine/
chmod 777 centreon-engine/*.rpm