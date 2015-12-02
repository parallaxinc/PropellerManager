#!/bin/bash
set -e

case "$TRAVIS_OS_NAME" in
"osx")
    wget -4 http://lamestation.net/downloads/travis/qt5.5.0-mac-clang.tgz
    tar xzf qt5.5.0-mac-clang.tgz
    mv local/ /Users/travis/local/
    ;;
"linux")
    ;;
*)
    echo "Invalid PLATFORM"
    exit 1
    ;;
esac

qmake -v
