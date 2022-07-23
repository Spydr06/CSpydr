#!/bin/bash

# A small script for installing this VSCode extension

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

VSCODE_EXTENSION_DIR="$HOME/.vscode-oss/extensions/cspydr"

pushd $SCRIPT_DIR

mkdir -vp $VSCODE_EXTENSION_DIR
cp -vr ./* $VSCODE_EXTENSION_DIR

popd