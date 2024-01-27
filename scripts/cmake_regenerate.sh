#!/usr/bin/env bash

# A small script to delete the CMake cache and regenerate the project

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pushd $SCRIPT_DIR

rm CMakeCache.txt
rm *.cmake
rm -rf bin
rm -rf .cache

cmake . "$@"

popd
