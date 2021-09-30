#!/bin/bash
set -e 
set -x

source common.sh

# Check arguments
if [ -z "$VERSION" -o -z "$RELEASE" ] ; then
  echo "You need to specify VERSION / RELEASE variables"
  exit 1
fi

MAJOR=`echo $VERSION | cut -d . -f 1,2`
ENGINEEL7RPMS=`echo output/*engine*.el7.*.rpm`
ENGINEEL8RPMS=`echo output/*engine*.el8.*.rpm`

# Publish RPMs
if [ "$BUILD" '=' 'QA' ]
then
  put_rpms "standard" "$MAJOR" "el7" "unstable" "x86_64" "engine" "centreon-engine-$VERSION-$RELEASE" $ENGINEEL7RPMS

  put_rpms "standard" "$MAJOR" "el8" "unstable" "x86_64" "engine" "centreon-engine-$VERSION-$RELEASE" $ENGINEEL8RPMS
elif [ "$BUILD" '=' 'RELEASE' ]
then
  copy_internal_source_to_testing "standard" "engine" "centreon-engine-$VERSION-$RELEASE"
  put_rpms "standard" "$MAJOR" "el7" "testing" "x86_64" "engine" "centreon-engine-$VERSION-$RELEASE" $ENGINEEL7RPMS

  put_rpms "standard" "$MAJOR" "el8" "testing" "x86_64" "engine" "centreon-engine-$VERSION-$RELEASE" $ENGINEEL8RPMS
fi