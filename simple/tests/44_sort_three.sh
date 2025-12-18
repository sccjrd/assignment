#!/bin/bash
exec "$1" tests/SORT <<EOF
20 10 -1
EOF
