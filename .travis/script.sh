#!/bin/bash
set -e

PROJECTNAME=propellermanager-$TRAVIS_TAG-$TRAVIS_OS_NAME

qmake PREFIX=$PROJECTNAME -r
make -j4
make install

ls bin/
ls lib/
ls include/

tar cvzf $PROJECTNAME.tgz $PROJECTNAME
