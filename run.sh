#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 <test-path>"
    exit 1
fi

set -e
set -x

./template-matching $1