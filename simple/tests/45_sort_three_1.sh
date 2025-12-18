#!/bin/bash
exec "$1" tests/SORT <<EOF
10 20 -1
EOF
