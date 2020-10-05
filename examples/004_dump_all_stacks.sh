#!/bin/bash
#
# Try dump stack of all processes we can find 

set -euo pipefail

for x in $(pgrep ''); do
    procsplice -sd -p $x -f /tmp/stackof_$x
done

