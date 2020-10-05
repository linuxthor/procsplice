#!/bin/bash
#
# Try dump heap of all bash processes we can find 

set -euo pipefail

for x in $(pgrep -x bash); do
    procsplice -hd -p $x -f /tmp/bash_heap_$x
done

