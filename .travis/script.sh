#!/bin/bash
set -e

PROJECTNAME=propellermanager-$TRAVIS_TAG-$TRAVIS_OS_NAME

qmake PREFIX=`pwd`/$PROJECTNAME -r
make -j4
make install

tar cvzf $PROJECTNAME.tgz $PROJECTNAME
