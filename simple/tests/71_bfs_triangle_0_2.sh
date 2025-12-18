#!/bin/bash
exec "$1" tests/BFS <<EOF
3 3
0 1
1 2
2 0

0 2
EOF
