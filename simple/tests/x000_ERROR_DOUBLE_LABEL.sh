#!/bin/bash
if "$1" tests/ERROR_DOUBLE_LABEL < /dev/null
then
    # the program is malformed: we expect the interpreter to fail
    exit 1
fi
