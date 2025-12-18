#!/bin/bash
exec "$1" tests/ADD_INPUT <<EOF
1 2 3 4 5 6 7 8 9 10
EOF
