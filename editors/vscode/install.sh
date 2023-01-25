#!/bin/bash

# A small script for installing this VSCode extension

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [[ -d "$HOME/.vscode/" ]]
then
	VSCODE_EXTENSION_DIR="$HOME/.vscode/extensions/cspydr"
elif [[ -d "$HOME/.vscode-oss/" ]]
then
	VSCODE_EXTENSION_DIR="$HOME/.vscode-oss/extensions/cspydr"
else
	echo "Error: Unable to find Visual Studio Code Path in $HOME!"
fi

pushd $SCRIPT_DIR

if [[ -d $VSCODE_EXTENSION_DIR ]]
then
    rm -vrf $VSCODE_EXTENSION_DIR
fi

mkdir -vp $VSCODE_EXTENSION_DIR
cp -vr ./* $VSCODE_EXTENSION_DIR

popd
