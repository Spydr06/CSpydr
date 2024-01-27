#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

pushd $SCRIPT_DIR

gen-allimport-file "libc.csp" $SCRIPT_DIR "file for quickly importing all libc files"

popd
