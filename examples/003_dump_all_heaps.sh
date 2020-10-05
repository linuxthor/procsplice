#!/bin/bash
#
# Try dump heap of all processes we can find 

set -euo pipefail

for x in $(pgrep ''); do
    procsplice -hd -p $x -f /tmp/heapof_$x
done

