#!/bin/bash
exec "$1" tests/COLLATZ <<EOF
27
EOF
