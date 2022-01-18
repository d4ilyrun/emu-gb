#!/usr/bin/env bash

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

cd $SCRIPTPATH/..

MAKE=$(which make)
TARGET=all

$MAKE $TARGET > /dev/null

if [ $? -ne 0 ]; then
    echo "Project '$TARGET' cannot be built."
    exit 1
fi

exit 0
