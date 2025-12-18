#!/bin/bash
exec "$1" tests/COLLATZ <<EOF
7
EOF
