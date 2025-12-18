#!/bin/bash
exec "$1" tests/PRINT_REVERSE <<EOF
2 4 6 8 10
EOF
