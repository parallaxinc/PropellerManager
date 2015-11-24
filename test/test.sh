#!/bin/bash

PROPMAN=./bin/propman

TOTAL_FAILS=0
TOTAL_TESTS=0

test_header()
{
    echo TEST: $1 $2
}

test_download()
{
    test_header "$2" "$1"
    echo
    test_pass $PROPMAN $3 $1
    echo

    sleep 6
}

test_commandline()
{
    echo TEST: command line 

    if [[ -z `$PROPMAN 2>&1 | grep binary` ]] ; then
        echo "didn't complain about missing binary"
    fi

    if [[ -z `$PROPMAN -h | grep Parallax` ]] ; then
        echo "Help not printing"
    fi
}


test_listdevices()
{
    echo TEST: list devices
    if [[ -z `$PROPMAN --list | grep 'ttyUSB\|cu.usbserial'` ]] ; then
        echo "No devices found"
    fi
}

test_identifydevices()
{
    echo TEST: identify devices
    if [[ -z `$PROPMAN --identify | grep 'Propeller P8X32A'` ]] ; then
        echo "No Propeller devices found"
    fi
}

test_fail()
{
    "$@"
    local status=$?
    if [ $status == 0 ] ; then
        echo "test passed when should have failed: $@" >&2
        exit 1
    fi
    return $status
}

test_pass()
{
    TOTAL_TESTS=$((TOTAL_TESTS+1))
    "$@"
    local status=$?
    if [ $status -ne 0 ] ; then
        echo "test failed: $@" >&2
        TOTAL_FAILS=$((TOTAL_FAILS+1))
    fi
    return $status
}

test_commandline
test_listdevices
test_identifydevices

test_download './test/images/Blank.binary' 'small file download'
test_download './test/images/ls/Brettris.binary' 'file download'
test_download './test/images/ls/FrappyBard.eeprom' 'EEPROM download'

test_download './test/images/Blank.binary' 'small file write' '-w'
test_download './test/images/ls/Brettris.binary' 'file write' '-w'
test_download './test/images/ls/FrappyBard.eeprom' 'EEPROM write' '-w'

echo
echo "Download Failures: $TOTAL_FAILS / $TOTAL_TESTS"
