#!/bin/bash
set -e

PROJECTNAME=propellermanager-$TRAVIS_TAG

qmake PREFIX=$PROJECTNAME
make -j4
make install

tar cvzf $PROJECTNAME.tgz $PROJECTNAME
