#!/bin/bash
#
# Try dump stack of all bash processes we can find 

set -euo pipefail

for x in $(pgrep -x bash); do
    procsplice -sd -p $x -f /tmp/bash_stack_$x
done

