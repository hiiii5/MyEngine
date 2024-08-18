#!/bin/bash

HELP() {
    echo "This script builds the Engine"
    echo
    echo "Syntax: $0 -c <config>"
    echo
    echo "Options:"
    echo "-c Debug (Debug build)"
    echo "-c RelWithDebInfo"
    echo "-c Release"
    echo "-m (use multi-configuration generator)"
    echo
}

config="Release"
multiconfig=false

while getopts c:m:h: flag
do
    case "${flag}" in
        h)  HELP
            exit 0
            ;;
        c) config=${OPTARG};;
        m) multiconfig=true;;
        \?) # Incorrect option
            echo "Error: Invalid option"
            HELP
            exit 1
            ;;
    esac
done

echo "Building in ${config} mode"
echo

echo "Building in multiconfig mode: ${multiconfig}"
echo

if ${multiconfig}; then
    cmake -S . -B ./build
    cmake --build ./build --config ${config}
else
    cmake -S . -B ./build -D CMAKE_BUILD_TYPE=${config}
    cmake --build ./build # --config ${config}
fi
