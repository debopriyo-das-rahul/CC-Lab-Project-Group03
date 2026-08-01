#!/bin/bash
# Runs the compiler against every test program in tests/.
# Usage: ./run_tests.sh

make --quiet

for f in tests/*.mini; do
    echo "============================================================"
    echo "TEST: $f"
    echo "============================================================"
    ./bin/compiler < "$f"
    echo
done
