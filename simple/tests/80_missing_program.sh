#!/bin/bash
if "$1" < /dev/null
then
    # we expect the program to fail, since no program is specified
    exit 1
fi

