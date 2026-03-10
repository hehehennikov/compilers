#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: ./run.sh {file_name}"
    exit 1
fi

INPUT_FILE=$(realpath "$1" 2>/dev/null || readlink -f "$1" 2>/dev/null)

if [ -z "$INPUT_FILE" ]; then
    INPUT_FILE="$1"
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [ -f "$SCRIPT_DIR/CMakeLists.txt" ]; then
    PROJECT_ROOT="$SCRIPT_DIR"
else
    PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
    if [ ! -f "$PROJECT_ROOT/CMakeLists.txt" ]; then
        echo "Error: Could not find CMakeLists.txt in $SCRIPT_DIR or its parent directory."
        exit 1
    fi
fi

cd "$PROJECT_ROOT" || exit
mkdir -p build
cd build || exit

cmake ..
make

./main "$INPUT_FILE"