#!/bin/bash
exec "$1" tests/ADD_INPUT <<EOF
1
EOF
