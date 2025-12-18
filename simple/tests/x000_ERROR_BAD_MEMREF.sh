#!/bin/bash
if "$1" tests/ERROR_BAD_MEMREF < /dev/null
then
    # the program is malformed: we expect the interpreter to fail
    exit 1
fi
